#ifndef CROSSPLATFORM_H
#define CROSSPLATFORM_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "Ws2_32.lib")

    typedef SOCKET socket_t;
    #define socket_close closesocket
    #define popen _popen
    #define pclose _pclose

    static inline int network_init() {
        WSADATA wsa;
        return WSAStartup(MAKEWORD(2, 2), &wsa);
    }

    static inline void network_cleanup() {
        WSACleanup();
    }
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>

    typedef int socket_t;
    #define socket_close close
    #define network_init() 0
    #define network_cleanup()
#endif

#endif //CROSSPLATFORM_H
