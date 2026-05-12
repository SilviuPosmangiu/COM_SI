#include "communications.h"
#include "aes_128.h"

int peer_socket = -1;
uchar shared_key[16];

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

long long rsa_gcd(long long a, long long b) {
    while (b) {
        a %= b;
		long long temp = a;
        a = b;
		b = temp;
    }
    return a;
}

long long rsa_modInverse(long long a, long long m) {
    long long m0 = m, y = 0, x = 1;
    if (m == 1) return 0;
    while (a > 1) {
        long long q = a / m;
        long long t = m;
        m = a % m, a = t;
        t = y;
        y = x - q * y;
        x = t;
    }
    if (x < 0) x += m0;
    return x;
}

long long rsa_modExp(long long base, long long exp, long long mod) {
    long long res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * base) % mod;
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return res;
}

int rsa_isPrime(long long n) {
    if (n < 2)
        return 0;

    for (long long i = 2; i * i <= n; i++) {
        if (n % i == 0)
            return 0;
    }

    return 1;
}

long long rsa_generatePrime() {
    while (1) {
        long long x = rand() % 100 + 100;

        if (rsa_isPrime(x))
            return x;
    }
}

RSAKeyPair rsa_generateKeys() {
    RSAKeyPair key;

    long long p = rsa_generatePrime();
    long long q = rsa_generatePrime();

    while (q == p)
        q = rsa_generatePrime();

    long long n = p * q;
    long long phi = (p - 1) * (q - 1);

    long long e = 3;

    while (rsa_gcd(e, phi) != 1)
        e += 2;

    long long d = rsa_modInverse(e, phi);

    key.n = n;
    key.e = e;
    key.d = d;

    return key;
}


void rsa_server_handshake(int peer_socket) {
    // Placeholder for RSA handshake implementation
	srand(time(NULL));

	RSAKeyPair keys = rsa_generateKeys();

	printf("[*] Public Key Generated: (e=%lld, n=%lld)\n", keys.e, keys.n);
	printf("[*] Private Key Generated: (d=%lld, n=%lld)\n", keys.d, keys.n);

	send(peer_socket, (const char*)&keys.e, sizeof(keys.e), 0);
	send(peer_socket, (const char*)&keys.n, sizeof(keys.n), 0);

    printf("[*] Waiting for client key");

    long long encryptedAESKey[16];
	recv(peer_socket, (char*)encryptedAESKey, sizeof(encryptedAESKey), 0);
    for (int i = 0; i < 16; i++) {
        long long decryptedByte = rsa_modExp(encryptedAESKey[i], keys.d, keys.n);
        shared_key[i] = (uchar)decryptedByte;
    }
	printf("\n[*] AES key received and decrypted: ");
    for(int i = 0; i < 16; i++) {
        printf("%02x ", shared_key[i]);
	}
	printf("\n");
}

void rsa_client_handshake(int peer_socket) {
    // Placeholder for RSA handshake implementation
	srand(time(NULL));

    long long e, n;
        recv(peer_socket, (char*)&e, sizeof(e), 0);
    recv(peer_socket, (char*)&n, sizeof(n), 0);
    printf("[*] Received Public Key: (e=%lld, n=%lld)\n", e, n);
    for (int i = 0; i < 16; i++) {
        shared_key[i] = rand() % 256;
    }
    printf("[*] Generated AES Key: ");
    for(int i = 0; i < 16; i++) {
        printf("%02x ", shared_key[i]);
	}
    printf("\n");
    long long encryptedAESKey[16];
    for (int i = 0; i < 16; i++) {
        encryptedAESKey[i] = rsa_modExp(shared_key[i], e, n);
    }
    send(peer_socket, (const char*)encryptedAESKey, sizeof(encryptedAESKey), 0);
	printf("[*] AES key encrypted and sent to server.\n");
}

int start_communication(const char* ip) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    bool is_server = false;

    peer_socket = try_as_client(ip);

    if (peer_socket == INVALID_SOCKET) {
        printf("[*] Could not connect, waiting as server...\n");
        peer_socket = try_as_server();
        is_server = true;
    }

    if (peer_socket == INVALID_SOCKET) {
        printf("[!] Connection failed\n");
        WSACleanup();
        return 1;
    }

    printf("[OK] Connected! Start chatting.\n\n");

    if (is_server) {
		rsa_server_handshake(peer_socket);
    }
    else {
		rsa_client_handshake(peer_socket);
    }

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
