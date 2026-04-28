#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#include <stdio.h>

#define PORT 9999
#define BUF_SIZE 1024
#define CHUNK_SIZE 16 

DWORD WINAPI receive_loop(LPVOID arg);
SOCKET try_as_server();
SOCKET try_as_client(const char* peer_ip);
int start_communication(const char* ip);