#pragma once

#include <netinet/in.h>

enum {
    LISTEN_BACKLOG = 10
};

typedef struct {
    int listen_file_descriptor;
    int port;
} ProcessingServer;

void ProcessingServer_init(ProcessingServer *server, int port, int *error_flag);
int ProcessingServer_create_listening_socket(int port, int *error_flag);
int ProcessingServer_accept_client(int listen_fd, struct sockaddr_in *client_address, int *error_flag);
void ProcessingServer_run(ProcessingServer *server);
void ProcessingServer_destroy(ProcessingServer *server);
void ProcessingServer_handle_client(int client_file_descriptor, const struct sockaddr_in *client_address);
