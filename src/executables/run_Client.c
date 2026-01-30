#include "executables/run_Client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "core/Client.h"
#include "utils/parse.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];

    int parse_error = 0;
    int port = parse_port(argv[2], &parse_error);
    if (parse_error != 0) {
        fprintf(stderr, "Invalid port: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    int create_error = 0;
    Client *client = Client_create(server_ip, port, &create_error);
    if (create_error != 0) {
        fprintf(stderr, "Failed to create client\n");
        return EXIT_FAILURE;
    }

    int connect_error = 0;
    Client_connect(client, &connect_error);
    if (connect_error != 0) {
        fprintf(stderr, "Failed to connect to server\n");
        Client_destroy(client);
        return EXIT_FAILURE;
    }

    printf("Connected to %s:%d\n", server_ip, port);
    Client_run(client);

    Client_destroy(client);

    return EXIT_SUCCESS;
}
