#pragma once

#include <netinet/in.h>
#include <pthread.h>
#include <sys/select.h>

typedef struct ProcessingServer ProcessingServer;

ProcessingServer *ProcessingServer_create(int port, int *error_flag);
void ProcessingServer_destroy(ProcessingServer *server);

void ProcessingServer_run(ProcessingServer *server);

