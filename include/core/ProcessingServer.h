#pragma once

#include <netinet/in.h>

typedef struct {
    int listen_file_descriptor;
    int port;
} ProcessingServer;

void ProcessingServer_init(ProcessingServer *server, int port, int *error_flag);
void ProcessingServer_run(ProcessingServer *server);
void ProcessingServer_destroy(ProcessingServer *server);
void ProcessingServer_handle_client(int client_file_descriptor, const struct sockaddr_in *client_address);
