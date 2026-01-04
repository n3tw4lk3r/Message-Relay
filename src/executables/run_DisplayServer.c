#include "executables/run_DisplayServer.h"

#include <stdio.h>
#include <stdlib.h>

#include "core/DisplayServer.h"
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

    DisplayServer server = { .listen_file_descriptor = -1 };

    int init_error = 0;
    display_server_init(&server, port, &init_error);
    
    if (init_error != 0) {
        perror("server init");
        return EXIT_FAILURE;
    }

    printf("Server listening on port %d\n", port);

    display_server_run(&server);
    display_server_destroy(&server);

    return EXIT_SUCCESS;
}
