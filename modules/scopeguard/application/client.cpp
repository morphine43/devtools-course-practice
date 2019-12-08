#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#ifdef _WIN32
/* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501  /* Windows XP. */
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
/* Assume that any non-Windows platform uses POSIX-style sockets instead. */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#include <unistd.h> /* Needed for close() */
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
using SOCKET = int;
#endif

#define SERVER_PORT "8080"
#define MAX_MSG_SIZE 1024
#define MAX_USERNAME 32

pthread_mutex_t sockfd_mutex;
char* username;

int socketInit(void)
{
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(1,1), &wsa_data);
#else
    return 0;
#endif
}

int socketQuit(void)
{
#ifdef _WIN32
    return WSACleanup();
#else
    return 0;
#endif
}

int socketClose(SOCKET sock)
{

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

void* getInAddr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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

    if ((rv = getaddrinfo(hostname, SERVER_PORT, &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            socketClose(*sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        std::cerr << "client: failed to connect" << std::endl;
        exit(1);
    }

    inet_ntop(p->ai_family, getInAddr((struct sockaddr *)p->ai_addr), s, sizeof(s));
    std::cout << "client: connecting to " << s << std::endl;

    freeaddrinfo(servinfo);
}

void* outputHandler(void* sock) {
    SOCKET* sockfd = (SOCKET*) sock;

    if (send(*sockfd, username, MAX_USERNAME -1, 0) == -1) {
        std::cerr << "username send failed" << std::endl;
    }

    while (true) {
        char send_msg[MAX_MSG_SIZE];

        std::cin.getline(send_msg, MAX_MSG_SIZE);

        if (strcmp(send_msg, ":q") == 0) {
            break;
        }

        if (send(*sockfd, send_msg, MAX_MSG_SIZE -1, 0) == -1) {
            std::cerr << "message send failed" << std::endl;
        }
    }

    return 0;
}

void* inputHandler(void* sock) {
    SOCKET* sockfd = (SOCKET*) sock;

    while (true) {
        char recv_msg[MAX_MSG_SIZE + MAX_USERNAME + 3];
        int numbytes;

        numbytes = recv(*sockfd, recv_msg, MAX_MSG_SIZE + MAX_USERNAME + 3, 0);

        if (numbytes == -1) {
            std::cerr << "message receive failed" << std::endl;
            break;
        } else if (numbytes == 0) {
            break;
        }

        std::cout << recv_msg << std::endl;
    }

    return 0;
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
    SOCKET* sockfd = new int;
    createAndConnectSocket(sockfd, hostname);

    pthread_t input, output;
    pthread_create(&input, NULL, outputHandler, sockfd);
    pthread_create(&output, NULL, inputHandler, sockfd);

    pthread_join(input, NULL);
    pthread_cancel(output);

    delete username;
    socketClose(*sockfd);
    delete sockfd;

    return 0;
}
