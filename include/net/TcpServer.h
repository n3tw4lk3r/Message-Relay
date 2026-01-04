#pragma once

#include <netinet/in.h>

int tcpServer_create_listening_socket(int port, int *error_flag);
int tcpServer_accept_client(int listen_fd, struct sockaddr_in *client_address, int *error_flag);
