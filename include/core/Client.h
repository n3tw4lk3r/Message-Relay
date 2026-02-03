#pragma once

#include <netinet/in.h>

typedef struct Client Client;

Client *Client_create(const char *server_ip, int port, int *error_flag);
void Client_destroy(Client *client);

void Client_connect(Client *client, int *error_flag);
int Client_is_connected(const Client *client);

ssize_t Client_receive(Client *client, char *buffer, size_t buffer_size, int *error_flag);

void Client_run(Client *client, int *error_flag);
