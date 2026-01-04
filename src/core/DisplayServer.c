#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "core/DisplayServer.h"
#include "net/TcpServer.h"
#include "utils/safe_io.h"

void display_server_init(DisplayServer *server, int port, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }
    
    if (!server || port <= 0 || port > 65535) {
        if (error_flag) {
            *error_flag = 1;
        }
        return;
    }
    
    server->port = port;
    
    int socket_error = 0;
    server->listen_file_descriptor = tcpServer_create_listening_socket(port, &socket_error);
    
    if (socket_error != 0 && error_flag) {
        *error_flag = socket_error;
    }
}

void display_server_run(DisplayServer *server) {
    if (!server) {
        return;
    }
    
    while (1) {
        struct sockaddr_in client_address;
        int accept_error = 0;
        int client_file_descriptor = tcpServer_accept_client(server->listen_file_descriptor, &client_address, &accept_error);
        
        if (accept_error != 0) {
            perror("accept");
            continue;
        }
        
        if (client_file_descriptor < 0) {
            continue;
        }

        handle_client(client_file_descriptor, &client_address);
        close(client_file_descriptor);
    }
}

void display_server_destroy(DisplayServer *server) {
    if (server->listen_file_descriptor >= 0) {
        close(server->listen_file_descriptor);
    }
}

void handle_client(int client_file_descriptor, const struct sockaddr_in *client_addr) {
    char ip[INET_ADDRSTRLEN];

    if (!inet_ntop(AF_INET, &client_addr->sin_addr, ip, sizeof(ip))) {
        strcpy(ip, "unknown");
    }
    printf("Client connected: %s:%d\n", ip, ntohs(client_addr->sin_port));

    char buffer[BUFSIZ];
    size_t buffer_used_size = 0;
    while (1) {
        int read_error = 0;
        ssize_t bytes_read = safe_read(client_file_descriptor, buffer + buffer_used_size, sizeof(buffer) - 1 - buffer_used_size, &read_error);
        
        if (read_error != 0) {
            perror("read");
            break;
        }
        
        if (bytes_read < 0) {
            break;
        }
        
        if (bytes_read == 0) {
            printf("Client %s disconnected\n", ip);
            break;
        }

        buffer_used_size += (size_t) bytes_read;
        buffer[buffer_used_size] = '\0';

        char *start = buffer;
        char *newline;
        while ((newline = memchr(start, '\n', buffer_used_size - (start - buffer))) != NULL) {
            *newline = '\0';
            printf("[%s]: %s\n", ip, start);
            start = newline + 1;
        }

        size_t remaining = buffer + buffer_used_size - start;
        if (start != buffer) {
            memmove(buffer, start, remaining);
        }
        buffer_used_size = remaining;
        
        if (buffer_used_size >= sizeof(buffer) - 1) {
            fprintf(stderr, "Buffer overflow, discarding data\n");
            buffer_used_size = 0;
        }
    }
    
    close(client_file_descriptor);
}
