#include "aes_128_tests.h"
#include "aes_128.h"
#include "communications.h"

#include <iostream>
#include <vector>
#include <string>
#include <mutex>
#include <ctime>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 9998
#define BUFFER_SIZE 1024
#define MAX_PEERS 100

struct Peer {
    std::string name;
    std::string ip;
    time_t last_seen;
};

std::vector<Peer> peers;

std::mutex peers_mutex;

char my_name[64];

DWORD WINAPI broadcast_thread(LPVOID lpParam);
DWORD WINAPI listen_thread(LPVOID lpParam);

void add_or_update_peer(const char* name, const char* ip) {

    std::lock_guard<std::mutex> lock(peers_mutex);

    for (auto& peer : peers) {

        if (peer.ip == ip) {

            peer.name = name;
            peer.last_seen = time(NULL);

            return;
        }
    }

    if (peers.size() < MAX_PEERS) {

        Peer peer;

        peer.name = name;
        peer.ip = ip;
        peer.last_seen = time(NULL);

        peers.push_back(peer);
    }
}

void cleanup_peers() {

    std::lock_guard<std::mutex> lock(peers_mutex);

    time_t now = time(NULL);

    for (auto it = peers.begin(); it != peers.end();) {

        if (now - it->last_seen > 5) {
            it = peers.erase(it);
        }
        else {
            ++it;
        }
    }
}

DWORD WINAPI broadcast_thread(LPVOID lpParam) {

    SOCKET sock;

    sockaddr_in broadcast_addr;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    BOOL broadcastEnable = TRUE;

    setsockopt(sock,
        SOL_SOCKET,
        SO_BROADCAST,
        (char*)&broadcastEnable,
        sizeof(broadcastEnable));

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));

    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(PORT);

    inet_pton(AF_INET,
        "255.255.255.255",
        &broadcast_addr.sin_addr);

    char message[BUFFER_SIZE];

    while (true) {

        snprintf(message,
            sizeof(message),
            "I_AM_HERE:%s",
            my_name);

        sendto(sock,
            message,
            strlen(message),
            0,
            (sockaddr*)&broadcast_addr,
            sizeof(broadcast_addr));

        Sleep(1000);
    }

    closesocket(sock);

    return 0;
}

DWORD WINAPI listen_thread(LPVOID lpParam) {

    SOCKET sock;

    sockaddr_in addr;
    sockaddr_in sender;

    int sender_len = sizeof(sender);

    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    BOOL reuse = TRUE;

    setsockopt(sock,
        SOL_SOCKET,
        SO_REUSEADDR,
        (char*)&reuse,
        sizeof(reuse));

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock,
        (sockaddr*)&addr,
        sizeof(addr)) == SOCKET_ERROR) {

        printf("Bind failed.\n");

        return 1;
    }

    while (true) {

        int len = recvfrom(sock,
            buffer,
            BUFFER_SIZE - 1,
            0,
            (sockaddr*)&sender,
            &sender_len);

        if (len <= 0)
            continue;

        buffer[len] = '\0';

        if (strncmp(buffer, "I_AM_HERE:", 10) == 0) {

            char* received_name = buffer + 10;

            char ip[INET_ADDRSTRLEN];

            inet_ntop(AF_INET,
                &sender.sin_addr,
                ip,
                sizeof(ip));

            if (strcmp(received_name, my_name) == 0)
                continue;

            add_or_update_peer(received_name, ip);
        }
    }

    closesocket(sock);

    return 0;
}

void print_peers() {

    cleanup_peers();

    std::lock_guard<std::mutex> lock(peers_mutex);

    printf("\n=== ONLINE PEERS ===\n");

    if (peers.empty()) {
        printf("No peers found.\n");
    }

    for (int i = 0; i < peers.size(); i++) {

        printf("%d. %s (%s)\n",
            i + 3,
            peers[i].name.c_str(),
            peers[i].ip.c_str());
    }
}

int main(int argc, char* argv[]) {

    WSADATA wsa;

    WSAStartup(MAKEWORD(2, 2), &wsa);

    test_aes_128();

    printf("Enter your name: ");

    fgets(my_name, sizeof(my_name), stdin);

    my_name[strcspn(my_name, "\n")] = 0;

    CreateThread(NULL,
        0,
        broadcast_thread,
        NULL,
        0,
        NULL);

    CreateThread(NULL,
        0,
        listen_thread,
        NULL,
        0,
        NULL);

    while (true) {

        char command[32];

        printf("\nCommands:\n");
        printf("1. Refresh\n");
        printf("2. Exit\n");

        {
            std::lock_guard<std::mutex> lock(peers_mutex);

            for (int i = 0; i < peers.size(); i++) {

                printf("%d. Connect to %s (%s)\n",
                    i + 3,
                    peers[i].name.c_str(),
                    peers[i].ip.c_str());
            }
        }

        printf("Enter command: ");

        fgets(command, sizeof(command), stdin);

        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "1") == 0) {

            print_peers();
        }
        else if (strcmp(command, "2") == 0) {

            printf("Exiting...\n");

            break;
        }
        else {

            int choice = atoi(command);

            std::lock_guard<std::mutex> lock(peers_mutex);

            if (choice >= 3 &&
                choice < 3 + peers.size()) {

                const char* ip =
                    peers[choice - 3].ip.c_str();

                start_communication(ip);
            }
            else {

                printf("Invalid command.\n");
            }
        }
    }

    WSACleanup();

    return 0;
}