#ifndef LAN_H
#define LAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define TCP_PORT 12345

void client(char * username);
void serveur();


#endif
