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
using SOCKET = int;
#endif

#include <iostream>
#include <thread>
#include <mutex>

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

#define PORT "4343"
#define BACKLOG 10

#define MAX_MSG_SIZE 1024
#define MAX_CLIENTS 256
#define MAX_USERNAME 32

struct Client {
    SOCKET* sockfd;
    char username[MAX_USERNAME];
};

Client** clients;
std::mutex clients_mutex;

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
        struct sockaddr_in *sockaddr_in = (struct sockaddr_in*)sa;
        struct in_addr *sin_addr = &sockaddr_in->sin_addr;
        return sin_addr;
    } else {
        struct sockaddr_in6 *sockaddr_in6 = (struct sockaddr_in6*)sa;
        struct in6_addr *sin6_addr = &sockaddr_in6->sin6_addr;
        return sin6_addr;
    }
}

void addClient(Client* client) {
    clients_mutex.lock();

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == NULL) {
            clients[i] = client;
            break;
        }
    }

    clients_mutex.unlock();
}

void removeClient(SOCKET* sockfd) {
    clients_mutex.lock();

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL) {
            if (clients[i]->sockfd == sockfd) {
                delete clients[i]->sockfd;
                clients[i]->sockfd = NULL;

                delete clients[i];
                clients[i] = NULL;

                break;
            }
        }
    }

    clients_mutex.unlock();
}

void sendMessageToClients(Client* client, char msg[]) {
    char send_msg[MAX_MSG_SIZE + MAX_USERNAME + 3];
    snprintf(send_msg, MAX_MSG_SIZE + MAX_USERNAME + 3,
             "[%s]: %s", client->username, msg);

    clients_mutex.lock();

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->sockfd != client->sockfd) {
            SOCKET* sockfd = clients[i]->sockfd;
            int status;

            status = send(*sockfd, send_msg,
                          MAX_MSG_SIZE + MAX_USERNAME + 3 - 1, 0);
            if (status == -1) {
                std::cerr << "send msg" << std::endl;
            }
        }
    }

    clients_mutex.unlock();
}

void sendJoinedMessageToClients(Client* client) {
    char joined_msg[MAX_MSG_SIZE];
    snprintf(joined_msg, MAX_MSG_SIZE, "%s joined", client->username);
    clients_mutex.lock();

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->sockfd != client->sockfd) {
            int status;

            status = send(*(clients[i]->sockfd),
                          joined_msg, MAX_MSG_SIZE -1, 0);
            if (status == -1) {
                std::cerr << "send joined" << std::endl;
            }
        }
    }

    clients_mutex.unlock();
}

void sendLeftMessageToClients(Client* client) {
    clients_mutex.lock();

    char left_msg[MAX_MSG_SIZE];
    snprintf(left_msg, MAX_MSG_SIZE, "%s left", client->username);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->sockfd != client->sockfd) {
            int status;

            status = send(*(clients[i]->sockfd), left_msg, MAX_MSG_SIZE -1, 0);
            if (status == -1) {
                std::cerr << "send left" << std::endl;
            }
        }
    }

    clients_mutex.unlock();
}

void setUsername(Client* client) {
    int numbytes;
    char username[MAX_USERNAME];

    numbytes = recv(*(client->sockfd), username, MAX_USERNAME, 0);

    if (numbytes == -1) {
        std::cerr << "recv user" << std::endl;
        return;
    } else if (numbytes == 0) {
        return;
    }

    std::cout << "server: user " << username << " join" << std::endl;
    snprintf(client->username, MAX_USERNAME, "%s", username);
}

void clientHandler(SOCKET* sock) {
    char msg[MAX_MSG_SIZE];

    Client* client = new Client();
    client->sockfd = sock;
    setUsername(client);
    addClient(client);
    sendJoinedMessageToClients(client);

    while (true) {
        int numbytes = recv(*sock, msg, MAX_MSG_SIZE, 0);

        if (numbytes == -1) {
            std::cerr << "recv msg" << std::endl;
            break;
        } else if (numbytes == 0) {
            break;
        }

        std::cout << "server: " << client->username
                  << " writing: " << msg << std::endl;

        sendMessageToClients(client, msg);
    }

    std::cout << "server: user " << client->username
              << " has left" << std::endl;
    socketClose(*sock);
    sendLeftMessageToClients(client);
    removeClient(sock);
}

void createAndBindSocket(SOCKET* sockfd) {
    struct addrinfo hints, *servinfo, *p;
    int yes = 0;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        *sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (*sockfd == -1) {
            std::cerr << "server: socket" << std::endl;
            continue;
        }

        int status;

        status = setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR,
                            reinterpret_cast<char*>(&yes), sizeof(yes));
        if (status == -1) {
            std::cerr << "setsockopt" << std::endl;
            exit(1);
        }

        if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            socketClose(*sockfd);
            std::cerr << "server: bind" << std::endl;
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        std::cerr << "server: failed to bind" << std::endl;
        exit(1);
    }
}

void initClients() {
    clients_mutex.lock();
    clients = new Client*[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }
    clients_mutex.unlock();
}

void deinitClients() {
    clients_mutex.lock();
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL) {
            delete clients[i]->sockfd;
            delete clients[i];
        }
    }

    delete[] clients;
    clients_mutex.unlock();
}

int main() {
    SOCKET sockfd;
    socketInit();
    createAndBindSocket(&sockfd);

    if (listen(sockfd, BACKLOG) == -1) {
        std::cerr << "listen" << std::endl;
        exit(1);
    }

    std::cout << "server: waiting for new connections..." << std::endl;

    initClients();

    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    while (true) {
        SOCKET* new_fd = new SOCKET;

        sin_size = sizeof(their_addr);
        *new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (*new_fd == -1) {
            std::cerr << "accept" << std::endl;
            continue;
        }

        inet_ntop_compat(their_addr.ss_family,
                  getInAddr((struct sockaddr *) &their_addr), s, sizeof(s));
        std::cout << "server: got connection from " << s << std::endl;

        auto thread = std::thread(clientHandler, new_fd);
        thread.detach();
    }

    deinitClients();
    socketQuit();

    return 0;
}
