#include "communications.h"

int peer_socket = -1;

using namespace std;

DWORD WINAPI receive_loop(LPVOID arg) {
    char buf[BUF_SIZE];
    while (1) {
        memset(buf, 0, BUF_SIZE);
        int bytes_recv = recv(peer_socket, buf, BUF_SIZE, 0);
        if (bytes_recv <= 0) {
            printf("Peer disconnected.\n");
            exit(0);
        }
        printf("Received: %s\n> ", buf);
    }
    return NULL;
}


SOCKET try_as_server() {
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) return INVALID_SOCKET;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(server_fd);
        return INVALID_SOCKET;
    }

    listen(server_fd, 1);
    printf("[*] Listening for peer on port %d...\n", PORT);

    peer_socket = accept(server_fd, NULL, NULL);
    closesocket(server_fd);
    return peer_socket;
}

SOCKET try_as_client(const char* peer_ip) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return INVALID_SOCKET;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, peer_ip, &addr.sin_addr);

    printf("[*] Trying to connect to %s:%d...\n", peer_ip, PORT);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

int start_communication(const char* ip) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    peer_socket = try_as_client(ip);

    if (peer_socket == INVALID_SOCKET) {
        printf("[*] Could not connect, waiting as server...\n");
        peer_socket = try_as_server();
    }

    if (peer_socket == INVALID_SOCKET) {
        printf("[!] Connection failed\n");
        WSACleanup();
        return 1;
    }

    printf("[OK] Connected! Start chatting.\n\n");

    CreateThread(NULL, 0, receive_loop, NULL, 0, NULL);

    char buf[BUF_SIZE];
    while (1) {
        printf("> ");
        fflush(stdout);
        fgets(buf, BUF_SIZE, stdin);
        buf[strcspn(buf, "\n")] = 0;
        send(peer_socket, buf, strlen(buf), 0);
    }

    closesocket(peer_socket);
    WSACleanup();
    return 0;
}
