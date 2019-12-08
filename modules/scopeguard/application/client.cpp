/**
 * Copyright 2019 Nikita Danilov
 */
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501  // Windows XP.
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
// Assume that any non-Windows platform uses POSIX-style sockets instead.
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>   // Needed for getaddrinfo() and freeaddrinfo()
#include <unistd.h>  // Needed for close()
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
using SOCKET = int;
#endif


#include <iostream>
#include <thread>
#include <chrono>
#include <future>

const char *inet_ntop_compat(int af, const void *src,
                             char *dst, socklen_t cnt) {
    if (af == AF_INET) {
        struct sockaddr_in in;
        memset(&in, 0, sizeof(in));
        in.sin_family = AF_INET;
        memcpy(&in.sin_addr, src, sizeof(struct in_addr));
        getnameinfo((struct sockaddr *)&in, sizeof(struct
            sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);
        return dst;
    }
    return NULL;
}

#define SERVER_PORT "4343"
#define MAX_MSG_SIZE 1024
#define MAX_USERNAME 32

char* username;

int socketInit(void) {
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(1, 1), &wsa_data);
#else
    return 0;
#endif
}

int socketQuit(void) {
#ifdef _WIN32
    return WSACleanup();
#else
    return 0;
#endif
}

int socketClose(SOCKET sock) {
    int status = 0;

#ifdef _WIN32
    status = shutdown(sock, SD_BOTH);
    if (status == 0) {
        status = closesocket(sock);
    }
#else
    status = shutdown(sock, SHUT_RDWR);
    if (status == 0) {
        status = close(sock);
    }
#endif

    return status;
}

void *getInAddr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        struct sockaddr_in *sockaddr_in;
        sockaddr_in = reinterpret_cast<struct sockaddr_in*>(sa);
        struct in_addr *sin_addr = &sockaddr_in->sin_addr;

        return sin_addr;
    } else {
        struct sockaddr_in6 *sockaddr_in6;
        sockaddr_in6 = reinterpret_cast<struct sockaddr_in6*>(sa);
        struct in6_addr *sin6_addr = &sockaddr_in6->sin6_addr;

        return sin6_addr;
    }
}

void createAndConnectSocket(SOCKET* sockfd, char* hostname) {
    struct addrinfo hints;
    struct addrinfo *servinfo,
                    *p;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    rv = getaddrinfo(hostname, SERVER_PORT, &hints, &servinfo);
    if (rv != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        *sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (*sockfd == -1) {
            continue;
        }

        size_t con = connect(*sockfd, p->ai_addr, p->ai_addrlen);
        if (con == size_t(-1)) {
            socketClose(*sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        std::cerr << "client: failed to connect" << std::endl;
        exit(1);
    }

    inet_ntop_compat(p->ai_family, getInAddr((struct sockaddr *)p->ai_addr),
              s, sizeof(s));
    std::cout << "client: connecting to " << s << std::endl;

    freeaddrinfo(servinfo);
}

void outputHandler(SOCKET* sock) {
    if (send(*sock, username, MAX_USERNAME -1, 0) == -1) {
        std::cerr << "username send failed" << std::endl;
    }

    while (true) {
        char send_msg[MAX_MSG_SIZE];

        std::cin.getline(send_msg, MAX_MSG_SIZE);

        if (strcmp(send_msg, ":q") == 0) {
            break;
        }

        if (send(*sock, send_msg, MAX_MSG_SIZE -1, 0) == -1) {
            std::cerr << "message send failed" << std::endl;
        }
    }
}

void inputHandler(SOCKET* sock, std::future<void> futureObj) {
#ifdef _WIN32
    u_long iMode = 1;
    ioctlsocket(*sock, FIONBIO, &iMode);
#endif
    while (futureObj.wait_for(std::chrono::milliseconds(1)) ==
            std::future_status::timeout) {
        char recv_msg[MAX_MSG_SIZE + MAX_USERNAME + 3];
        int numbytes;

#ifdef _WIN32
        numbytes = recv(*sock, recv_msg, MAX_MSG_SIZE + MAX_USERNAME + 3, 0);
#else
        numbytes = recv(*sock, recv_msg, MAX_MSG_SIZE + MAX_USERNAME + 3,
                        MSG_DONTWAIT);
#endif
        if (numbytes > 0) {
            std::cout << recv_msg << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 3) {
        std::cerr << "usage: client <username> <hostname>" << std::endl;
        exit(1);
    }

    username = new char[MAX_USERNAME];
    memcpy(username, argv[1], MAX_USERNAME);

    char* hostname = NULL;
    if (argc == 3) {
        hostname = new char[strlen(argv[2])];
        memcpy(hostname, argv[2], strlen(argv[2]));
    }

    socketInit();
    SOCKET* sockfd = new SOCKET;
    createAndConnectSocket(sockfd, hostname);

    // Create a std::promise object
    std::promise<void> exitSignal;
    // Fetch std::future object associated with promise
    std::future<void> futureObj = exitSignal.get_future();

    std::thread outputThread(outputHandler, sockfd);
    std::thread inputThread(inputHandler, sockfd, std::move(futureObj));

    outputThread.join();
    exitSignal.set_value();
    inputThread.join();

    delete[] username;
    if (hostname != NULL)
        delete[] hostname;

    socketClose(*sockfd);
    delete sockfd;

    return 0;
}
