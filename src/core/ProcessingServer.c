#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "core/ProcessingServer.h"
#include "utils/safe_io.h"

void ProcessingServer_init(ProcessingServer *server, int port, int *error_flag) {
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
    server->listen_file_descriptor = ProcessingServer_create_listening_socket(port, &socket_error);
    
    if (socket_error != 0 && error_flag) {
        *error_flag = socket_error;
    }
}

int ProcessingServer_create_listening_socket(int port, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }
    
    int file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (file_descriptor < 0) {
        if (error_flag) {
            *error_flag = 1;
        }
        return -1;
    }

    int opt = 1;
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(file_descriptor);
        if (error_flag) {
            *error_flag = 1;
        }
        return -1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons((uint16_t)port);

    if (bind(file_descriptor, (struct sockaddr *) &address, sizeof(address)) < 0) {
        close(file_descriptor);
        if (error_flag) {
            *error_flag = 1;
        }
        return -1;
    }

    if (listen(file_descriptor, LISTEN_BACKLOG) < 0) {
        close(file_descriptor);
        if (error_flag) {
            *error_flag = 1;
        }
        return -1;
    }

    return file_descriptor;
}

int ProcessingServer_accept_client(int listen_file_descriptor, struct sockaddr_in *client_address, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }
    
    socklen_t length = sizeof(*client_address);
    while (1) {
        int file_descriptor = accept(listen_file_descriptor, (struct sockaddr *) client_address, &length);
        if (file_descriptor < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (error_flag) {
                *error_flag = 1;
            }
        }
        return file_descriptor;
    }
}
void ProcessingServer_run(ProcessingServer *server) {
    if (!server) {
        return;
    }
    
    while (1) {
        struct sockaddr_in client_address;
        int accept_error = 0;
        int client_file_descriptor = ProcessingServer_accept_client(server->listen_file_descriptor, &client_address, &accept_error);
        
        if (accept_error != 0) {
            perror("accept");
            continue;
        }
        
        if (client_file_descriptor < 0) {
            continue;
        }

        ProcessingServer_handle_client(client_file_descriptor, &client_address);
        close(client_file_descriptor);
    }
}

void ProcessingServer_destroy(ProcessingServer *server) {
    if (server->listen_file_descriptor >= 0) {
        close(server->listen_file_descriptor);
    }
}

void ProcessingServer_handle_client(int client_file_descriptor, const struct sockaddr_in *client_addr) {
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

