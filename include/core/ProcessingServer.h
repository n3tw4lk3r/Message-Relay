#pragma once

#include <netinet/in.h>
#include <pthread.h>
#include <sys/select.h>

enum {
    LISTEN_BACKLOG = 10
};

typedef struct ClientNode ClientNode; // for internal use

typedef struct {
    int listen_file_descriptor;
    ClientNode *clients;
    int client_count;
    pthread_mutex_t mutex;
    fd_set master_file_descriptor_set;
    int max_file_descriptor;
    int port;
} ProcessingServer;

void ProcessingServer_init(ProcessingServer *server, int port, int *error_flag);
int ProcessingServer_create_listening_socket(int port, int *error_flag);
int ProcessingServer_accept_connection(int listen_fd, struct sockaddr_in *client_address, int *error_flag);
void ProcessingServer_run(ProcessingServer *server);
void ProcessingServer_destroy(ProcessingServer *server);
void ProcessingServer_attach_client(ProcessingServer *server, int file_descriptor, struct sockaddr_in *address);
void Processing_server_detach_client(ProcessingServer *server, int file_descriptor);
