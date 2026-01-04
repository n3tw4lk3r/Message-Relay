#pragma once

#include <netinet/in.h>

typedef struct {
    int socket_file_descriptor;
    struct sockaddr_in server_address;
    int is_connected;
} Client;

void client_init(Client *client, const char *server_ip, int port, int *error_flag);
void client_connect(Client *client, int *error_flag);
ssize_t client_send(Client *client, const char *message, size_t len, int *error_flag);
ssize_t client_receive(Client *client, char *buffer, size_t buffer_size, int *error_flag);
void client_destroy(Client *client);
int client_is_connected(const Client *client);
