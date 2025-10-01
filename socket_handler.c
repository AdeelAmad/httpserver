//
// Created by Adeel Ahmad on 1/21/25.
//

#include "socket_handler.h"

typedef struct listener_sock {
    int sockfd;
    struct sockaddr_in *addr;
} l_sock;

l_sock *listen_port(int port) {
    l_sock *sock = calloc(1, sizeof(l_sock));
    sock->sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock->sockfd < 0) {
        free(sock);
        return NULL;
    }

    struct timeval *timeout = calloc(1, sizeof(struct timeval));
    timeout->tv_sec = 5;
    timeout->tv_usec = 0;

    if (setsockopt(sock->sockfd, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(struct timeval)) < 0) {
        free(timeout);
        free(sock);
        return NULL;
    }

    if (setsockopt(sock->sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) { 1 }, sizeof(int)) < 0) {
        free(timeout);
        free(sock);
        return NULL;
    }

    free(timeout);

    sock->addr = calloc(1, sizeof(struct sockaddr_in));
    socklen_t addrlen = sizeof(*sock->addr);

    sock->addr->sin_family = AF_INET;
    sock->addr->sin_port = htons(port);
    sock->addr->sin_addr.s_addr = INADDR_ANY;

    if (bind(sock->sockfd, (const struct sockaddr *) sock->addr, addrlen) == 0) {
        if (listen(sock->sockfd, BACKLOG) == 0) {
            return sock;
        }
    }

    free(sock->addr);
    free(sock);
    return NULL;
}

int accept_connection(l_sock *sock) {
    int connection = accept(sock->sockfd, NULL, NULL);
    if (connection >= 0) {
        return connection;
    }
    return -1;
}

void close_connection(int sock) {
    close(sock);
}

void close_port(l_sock *sock) {
    close(sock->sockfd);
    free(sock->addr);
    free(sock);
}
