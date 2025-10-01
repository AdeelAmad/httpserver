//
// Created by Adeel Ahmad on 1/21/25.
//

#ifndef AAHMAD9_ADEEL_SOCKET_HANDLER_H
#define AAHMAD9_ADEEL_SOCKET_HANDLER_H

#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>

#define BACKLOG 1000

typedef struct listener_sock l_sock;

l_sock *listen_port(int port);

int accept_connection(l_sock *sock);

void close_connection(int sock);

void close_port(l_sock *sock);

#endif //AAHMAD9_ADEEL_SOCKET_HANDLER_H
