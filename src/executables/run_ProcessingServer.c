#include "executables/run_ProcessingServer.h"

#include <stdio.h>
#include <stdlib.h>

#include "core/ProcessingServer.h"
#include "utils/parse.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int parse_error = 0;
    int port = parse_port(argv[1], &parse_error);
    
    if (parse_error != 0) {
        fprintf(stderr, "Invalid port\n");
        return EXIT_FAILURE;
    }
    
    int create_error = 0;
    ProcessingServer *server = ProcessingServer_create(port, &create_error);
    
    if (create_error != 0) {
        perror("ProcessingServer_create error");
        return EXIT_FAILURE;
    }

    printf("Server listening on port %d\n", port);
    
    int error_flag = 0;
    ProcessingServer_run(server, &error_flag);
    if (error_flag) {
        return EXIT_FAILURE;
    }

    ProcessingServer_destroy(server);

    return EXIT_SUCCESS;
}
