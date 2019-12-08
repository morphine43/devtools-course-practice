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

#define PORT "8080"
#define BACKLOG 10

#define MAX_MSG_SIZE 1024
#define MAX_CLIENTS 256
#define MAX_USERNAME 32

struct Client {
    SOCKET* sockfd;
    char username[MAX_USERNAME];
};

Client** clients;
pthread_mutex_t clients_mutex;

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

void *getInAddr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void addClient(Client* client) {
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == NULL) {
            clients[i] = client;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void removeClient(int* sockfd) {
    pthread_mutex_lock(&clients_mutex);

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

    pthread_mutex_unlock(&clients_mutex);
}

void sendMessageToClients(Client* client, char msg[]) {
    char send_msg[MAX_MSG_SIZE + MAX_USERNAME + 3];
    strcpy(send_msg, "[");
    strcat(send_msg, client->username);
    strcat(send_msg, "]");
    strcat(send_msg, ": ");
    strcat(send_msg, msg);

    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->sockfd != client->sockfd) {
            int* sockfd = clients[i]->sockfd;
            if (send(*sockfd, send_msg, MAX_MSG_SIZE + MAX_USERNAME + 3 - 1, 0) == -1) {
                std::cerr << "send msg" << std::endl;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void sendJoinedMessageToClients(Client* client) {
    char joined_msg[MAX_MSG_SIZE];
    strcpy(joined_msg, client->username);
    strcat(joined_msg, " joined");

    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->sockfd != client->sockfd) {
            if (send(*(clients[i]->sockfd), joined_msg, MAX_MSG_SIZE -1, 0) == -1) {
                std::cerr << "send joined" << std::endl;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void sendLeftMessageToClients(Client* client) {
    pthread_mutex_lock(&clients_mutex);

    char left_msg[MAX_MSG_SIZE];
    strcpy(left_msg, client->username);
    strcat(left_msg, " left");

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->sockfd != client->sockfd) {
            if (send(*(clients[i]->sockfd), left_msg, MAX_MSG_SIZE -1, 0) == -1) {
                std::cerr << "send left" << std::endl;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
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
    strcpy(client->username, username);
}

void *clientHandler(void* sock) {
    SOCKET* sockfd = (SOCKET*) sock;
    int numbytes;
    char msg[MAX_MSG_SIZE];

    Client* client = new Client();
    client->sockfd = sockfd;
    setUsername(client);
    addClient(client);
    sendJoinedMessageToClients(client);

    while (true){
        numbytes = recv(*sockfd, msg, MAX_MSG_SIZE, 0);

        if (numbytes == -1) {
            std::cerr << "recv msg" << std::endl;
            break;
        } else if (numbytes == 0) {
            break;
        }

        std::cout << "server: " << client->username << " writing: " << msg << std::endl;

        sendMessageToClients(client, msg);
    }

    std::cout << "server: user " << client->username << " has left" << std::endl;
    socketClose(*sockfd);
    sendLeftMessageToClients(client);
    removeClient(sockfd);
    pthread_exit(NULL);
}

void createAndBindSocket(SOCKET* sockfd) {
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;
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
        if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cerr << "server: socket" << std::endl;
            continue;
        }

        if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
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
    pthread_mutex_lock(&clients_mutex);
    clients = new Client*[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }
    pthread_mutex_unlock(&clients_mutex);
}

void deinitClients() {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL) {
            delete clients[i]->sockfd;
            delete clients[i];
        }
    }

    delete[] clients;
    pthread_mutex_unlock(&clients_mutex);
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
    while (true){
        SOCKET* new_fd = new int;
        sin_size = sizeof(their_addr);
        if ((*new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
            std::cerr << "accept" << std::endl;
            continue;
        }

        inet_ntop(their_addr.ss_family, getInAddr((struct sockaddr *) &their_addr), s, sizeof(s));
        std::cout << "server: got connection from " << s << std::endl;

        pthread_t client;
        pthread_create(&client, NULL, clientHandler, new_fd);
    }

    deinitClients();
    socketQuit();

    return 0;
}
