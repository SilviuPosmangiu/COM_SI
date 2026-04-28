#include "communications.h"
#include "aes_128.h"

int peer_socket = -1;

const uchar shared_key[16] = {
    0x2b, 0x7e, 0x15, 0x16,
    0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88,
    0x09, 0xcf, 0x4f, 0x3c
};

using namespace std;

DWORD WINAPI receive_loop(LPVOID arg) {
    while (1) {
        int total_len = 0;
        int bytes_recv = recv(peer_socket, (char*)&total_len, sizeof(total_len), 0);
        if (bytes_recv <= 0) {
            printf("\nPeer disconnected.\n");
            exit(0);
        }
        
        total_len = ntohl(total_len);
        
        int received = 0;
        uchar ciphertext[BUF_SIZE + 16];
        memset(ciphertext, 0, sizeof(ciphertext));
        uchar* dest = ciphertext;
        
        while (received < total_len) {
            int to_receive = total_len - received;
            if (to_receive > CHUNK_SIZE) {
                to_receive = CHUNK_SIZE;
            }
            
            char chunk[CHUNK_SIZE];
            memset(chunk, 0, CHUNK_SIZE);
            
            int b = recv(peer_socket, chunk, to_receive, 0);
            if (b <= 0) {
                printf("\nPeer disconnected during transfer.\n");
                exit(0);
            }
            
            memcpy(dest, chunk, b);
            dest += b;
            received += b;
        }

        uchar plaintext[BUF_SIZE + 16];
        memset(plaintext, 0, sizeof(plaintext));
        int plain_len = 0;

        aes_128_decrypt(ciphertext, total_len, shared_key, plaintext, &plain_len);
        plaintext[plain_len] = '\0';

        printf("Received: %s\n> ", plaintext);
        fflush(stdout);
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
        if (!fgets(buf, BUF_SIZE, stdin)) break;
        buf[strcspn(buf, "\n")] = 0;
        
        int total_len = strlen(buf);
        if (total_len == 0) continue;

        uchar ciphertext[BUF_SIZE + 16];
        memset(ciphertext, 0, sizeof(ciphertext));
        int cipher_len = 0;

        aes_128_encrypt((const uchar*)buf, total_len, shared_key, ciphertext, &cipher_len);

        int net_len = htonl(cipher_len);
        send(peer_socket, (const char*)&net_len, sizeof(net_len), 0);
        
        int sent = 0;
        uchar* src = ciphertext;
        
        while (sent < cipher_len) {
            int to_send = cipher_len - sent;
            if (to_send > CHUNK_SIZE) {
                to_send = CHUNK_SIZE;
            }
            
            char chunk[CHUNK_SIZE];
            memset(chunk, 0, CHUNK_SIZE);
            memcpy(chunk, src, to_send);
            
            send(peer_socket, chunk, to_send, 0);
            
            src += to_send;
            sent += to_send;
        }
    }

    closesocket(peer_socket);
    WSACleanup();
    return 0;
}
