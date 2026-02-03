#pragma once

#include <netinet/in.h>
#include <pthread.h>
#include <sys/select.h>

typedef struct ProcessingServer ProcessingServer;

ProcessingServer *ProcessingServer_create(int port, int *error_flag);
void ProcessingServer_destroy(ProcessingServer *server);

void ProcessingServer_run(ProcessingServer *server);
void ProcessingServer_attach_client(ProcessingServer *server, int file_descriptor, struct sockaddr_in *address);
void Processing_server_detach_client(ProcessingServer *server, int file_descriptor);

void Processing_server_broadcast(ProcessingServer *server, const char *message, size_t message_length);
