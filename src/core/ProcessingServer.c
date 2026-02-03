#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "core/Console.h"
#include "core/ProcessingServer.h"
#include "utils/safe_io.h"

enum {
    LISTEN_BACKLOG = 10,
};

typedef struct ClientNode ClientNode;

struct ClientNode {
    int file_descriptor;
    struct sockaddr_in address;
    struct ClientNode *next;
};

struct ProcessingServer {
    int listen_file_descriptor;
    ClientNode *clients;
    int client_count;
    pthread_mutex_t mutex;
    fd_set master_file_descriptor_set;
    int max_file_descriptor;
    int port;
};

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

ProcessingServer *ProcessingServer_create(int port, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }

    if (port <= 0 || port > 65535) {
        if (error_flag) {
            *error_flag = 1;
        }
        return NULL;
    }
    
    ProcessingServer *server = calloc(1, sizeof(ProcessingServer));
    if (!server) {
        *error_flag = 1;
        return NULL;
    }

    server->port = port;
    server->listen_file_descriptor = ProcessingServer_create_listening_socket(port, error_flag);

    if (server->listen_file_descriptor < 0) {
        if (error_flag) {
            *error_flag = 1;
        }
        return NULL;
    }

    FD_ZERO(&server->master_file_descriptor_set);
    FD_SET(server->listen_file_descriptor, &server->master_file_descriptor_set);
    server->max_file_descriptor = server->listen_file_descriptor;

    pthread_mutex_init(&server->mutex, NULL);
    return server;
}

void ProcessingServer_attach_client(ProcessingServer *server, int file_descriptor, struct sockaddr_in *address) {
    ClientNode *node = malloc(sizeof(ClientNode));
    node->file_descriptor = file_descriptor;
    node->address = *address;
    node->next = server->clients;
    server->clients = node;
    ++server->client_count;

    FD_SET(file_descriptor, &server->master_file_descriptor_set);
    if (file_descriptor > server->max_file_descriptor) {
        server->max_file_descriptor = file_descriptor;
    }
}

void Processing_server_detach_client(ProcessingServer *server, int file_descriptor) {
    ClientNode **current = &server->clients;
    while (*current) {
        if ((*current)->file_descriptor == file_descriptor) {
            ClientNode *tmp = *current;
            *current = tmp->next;
            close(tmp->file_descriptor);
            free(tmp);
            --server->client_count;
            break;
        }
        current = &(*current)->next;
    }

    FD_CLR(file_descriptor, &server->master_file_descriptor_set);

    if (file_descriptor == server->max_file_descriptor) {
        int new_max = server->listen_file_descriptor;
        for (ClientNode *current_ = server->clients; current_; current_ = current_->next) {
            if (current_->file_descriptor > new_max) {
                new_max = current_->file_descriptor;
            }
        }
        server->max_file_descriptor = new_max;
    }
}

int ProcessingServer_accept_connection(int listen_file_descriptor, struct sockaddr_in *client_address, int *error_flag) {
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

void ProcessingServer_broadcast(ProcessingServer *server, const char *message, size_t message_length) {
    ClientNode *current = server->clients;
    ClientNode *next;

    while (current) {
        next = current->next;
        
        size_t sent = 0;
        while (sent < message_length) {
            ssize_t current_sent = send(current->file_descriptor, message + sent, message_length - sent, 0);
            if (current_sent == 0) {
                Processing_server_detach_client(server, current->file_descriptor);
                break;
            }
            if (current_sent < 0) {
                if (errno == EPIPE || errno == ECONNRESET || errno == ECONNABORTED) {
                    close(current->file_descriptor);
                    Processing_server_detach_client(server, current->file_descriptor);
                } else {
                    perror("send");
                }
                break;
            }
            sent += current_sent;
        }
        current = next;
    }
}

void ProcessingServer_run(ProcessingServer *server, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }

    int max_file_descriptor = server->listen_file_descriptor;
    if (STDIN_FILENO > max_file_descriptor) {
        max_file_descriptor = STDIN_FILENO;
    }

    int create_console_error = 0;
    Console *console = Console_create(&create_console_error);
    if (create_console_error) {
        *error_flag = 1;
        return;
    }

    clear_screen();
    hide_cursor();
    for (int i = 0; i < MESSAGES_DISPLAYED; ++i) {
        Console_add_message(console, "");
    }
    Console_render(console);

    char buffer[BUFSIZ];
    int is_running = 1;
    while (is_running) {
        fd_set read_file_descriptor_set;
        FD_ZERO(&read_file_descriptor_set);
        
        FD_SET(STDIN_FILENO, &read_file_descriptor_set);
        
        FD_SET(server->listen_file_descriptor, &read_file_descriptor_set);
        
        ClientNode *client = server->clients;
        while (client) {
            FD_SET(client->file_descriptor, &read_file_descriptor_set);
            if (client->file_descriptor > max_file_descriptor) {
                max_file_descriptor = client->file_descriptor;
            }
            client = client->next;
        }

        int ready = select(max_file_descriptor + 1, &read_file_descriptor_set, NULL, NULL, NULL);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &read_file_descriptor_set)) {
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                is_running = 0;
                break;
            }
            
            if (strcmp(buffer, "exit()\n") == 0) {
                is_running = 0;
                break;
            }
            
            char message_buffer[BUFSIZ];
            snprintf(message_buffer, sizeof(message_buffer), "[SERVER]: %.*s", (int) strlen(buffer), buffer);
            ProcessingServer_broadcast(server, message_buffer, strlen(message_buffer));

            Console_add_message(console, buffer);
            Console_render(console);
        }

        if (FD_ISSET(server->listen_file_descriptor, &read_file_descriptor_set)) {
            struct sockaddr_in client_address;
            int accept_error = 0;
            int client_fd = ProcessingServer_accept_connection(
                server->listen_file_descriptor, &client_address, &accept_error);

            if (client_fd >= 0) {
                ProcessingServer_attach_client(server, client_fd, &client_address);

                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_address.sin_addr, ip, sizeof(ip));
                unsigned short port = ntohs(client_address.sin_port);
                snprintf(buffer, sizeof(buffer), "Client connected: %s:%d", ip, port);
                Console_add_message(console, buffer);
                Console_render(console);
                
                if (client_fd > max_file_descriptor) {
                    max_file_descriptor = client_fd;
                }
            }
        }

        client = server->clients;
        while (client) {
            ClientNode *next = client->next;
            
            if (FD_ISSET(client->file_descriptor, &read_file_descriptor_set)) {
                int read_error = 0;
                ssize_t bytes_read = safe_read(client->file_descriptor, buffer, sizeof(buffer) - 1, &read_error);

                if (bytes_read <= 0 || read_error) {
                    char ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client->address.sin_addr, ip, sizeof(ip));
                    unsigned short port = ntohs(client->address.sin_port);
                    snprintf(buffer, sizeof(buffer), "Client %s:%d disconnected", ip, port);
                    Console_add_message(console, buffer);
                    Console_render(console);
                    
                    Processing_server_detach_client(server, client->file_descriptor);
                } else {
                    buffer[bytes_read] = '\0';
                    
                    char ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client->address.sin_addr, ip, sizeof(ip));
                    unsigned short port = ntohs(client->address.sin_port);

                    char message_buffer[BUFSIZ];
                    size_t display_length = bytes_read;
                    if (bytes_read >= MAX_MESSAGE_LENGTH) {
                        display_length = MAX_MESSAGE_LENGTH;
                    }
                    
                    snprintf(message_buffer, sizeof(message_buffer), "[%s:%d]: %.*s", ip, port, (int)display_length, buffer);
                    Console_add_message(console, message_buffer);
                    Console_render(console);

                    ProcessingServer_broadcast(server, message_buffer, strlen(message_buffer));
                }
            }
            
            client = next;
        }
    }

    move_cursor(AT_EXIT_MESSAGE_ROW, 1);
    reset_terminal();
    show_cursor();
    printf("Server stopped successfully.\n");
    Console_destroy(console);
}

void ProcessingServer_destroy(ProcessingServer *server) {
    FD_CLR(STDIN_FILENO, &server->master_file_descriptor_set);

    ClientNode *current = server->clients;
    while (current) {
        ClientNode *next = current->next;
        close(current->file_descriptor);
        free(current);
        current = next;
    }

    if (server->listen_file_descriptor >= 0) {
        close(server->listen_file_descriptor);
    }

    pthread_mutex_destroy(&server->mutex);
}

