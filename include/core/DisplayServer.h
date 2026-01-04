#pragma once

#include <netinet/in.h>

typedef struct {
    int listen_file_descriptor;
    int port;
} DisplayServer;

void display_server_init(DisplayServer *server, int port, int *error_flag);
void display_server_run(DisplayServer *server);
void display_server_destroy(DisplayServer *server);
void handle_client(int client_file_descriptor, const struct sockaddr_in *client_address);
