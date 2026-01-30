#include "core/Client.h"
#include "utils/safe_io.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "core/Console.h"

struct Client {
    int socket_file_descriptor;
    struct sockaddr_in server_address;
    int is_connected;
};

Client *Client_create(const char *server_ip, int port, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }
    
    if (!server_ip || port <= 0 || port > 65535) {
        if (error_flag) {
            *error_flag = 1;
        }
        return NULL;
    }

    Client *client = calloc(1, sizeof(Client));
    if (!client) {
        *error_flag = 1;
        return NULL;
    }

    client->socket_file_descriptor = -1;
    client->is_connected = 0;
    client->server_address.sin_family = AF_INET;
    client->server_address.sin_port = htons((uint16_t) port);

    if (inet_pton(AF_INET, server_ip, &client->server_address.sin_addr) != 1) {
        fprintf(stderr, "Invalid IP address: %s\n", server_ip);
        if (error_flag) {
            *error_flag = 1;
        }
        return NULL;
    }
    return client;
}

void Client_connect(Client *client, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }
    
    if (!client || client->is_connected) {
        if (error_flag) {
            *error_flag = 1;
        }
        return;
    }
    
    client->socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (client->socket_file_descriptor < 0) {
        perror("socket");
        if (error_flag) {
            *error_flag = 1;
        }
        return;
    }
    
    if (connect(client->socket_file_descriptor, (struct sockaddr *) &client->server_address, sizeof(client->server_address)) < 0) {
        perror("connect");
        close(client->socket_file_descriptor);
        client->socket_file_descriptor = -1;
        if (error_flag) {
            *error_flag = 1;
        }
        return;
    }
    
    client->is_connected = 1;
    
    char server_ip_string[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client->server_address.sin_addr, server_ip_string, sizeof(server_ip_string));
    
    printf("Connected to server %s:%d\n", server_ip_string, ntohs(client->server_address.sin_port));
}

ssize_t Client_send(Client *client, const char *message, size_t len, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }
    
    if (!client || !client->is_connected || !message) {
        if (error_flag) {
            *error_flag = 1;
        }
        return -1;
    }
    
    int write_error = 0;
    ssize_t result = safe_write(client->socket_file_descriptor, message, len, &write_error);
    
    if (write_error != 0 && error_flag) {
        *error_flag = write_error;
    }
    
    return result;
}

ssize_t Client_receive(Client *client, char *buffer, size_t buffer_size, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }
    
    if (!client || !client->is_connected || !buffer || buffer_size == 0) {
        if (error_flag) {
            *error_flag = 1;
        }
        return -1;
    }
    
    int read_error = 0;
    ssize_t result = safe_read(client->socket_file_descriptor, buffer, buffer_size, &read_error);
    
    if (read_error != 0 && error_flag) {
        *error_flag = read_error;
    }
    
    return result;
}

void Client_destroy(Client *client) {
    if (!client) {
        return;
    }
    
    if (client->socket_file_descriptor >= 0) {
        close(client->socket_file_descriptor);
    }
    
    free(client);
}

int Client_is_connected(const Client *client) {
    return client && client->is_connected;
}

void Client_run(Client *client, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }

    int max_file_descriptor;
    if (client->socket_file_descriptor > STDIN_FILENO) {
        max_file_descriptor = client->socket_file_descriptor;
    } else {
        max_file_descriptor = STDIN_FILENO;
    }
    
    char buffer[BUFSIZ];

    int create_console_error = 0;
    Console *console = Console_create(&create_console_error);
    if (create_console_error) {
        *error_flag = 1;
        return;
    }

    clear_screen();
    hide_cursor();
    for (int i = 0; i < MAX_MESSAGES_COUNT; ++i) {
        Console_add_message(console, "");
    }
    Console_render(console);
    
    int is_running = 1;
    while (is_running) {
        fd_set read_file_descriptor_set;
        FD_ZERO(&read_file_descriptor_set);
        FD_SET(STDIN_FILENO, &read_file_descriptor_set);
        FD_SET(client->socket_file_descriptor, &read_file_descriptor_set);

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

            size_t length = strlen(buffer);
            if (strcmp(buffer, "exit()\n") == 0) {
                is_running = 0;
                break;
            }

            int send_error = 0;
            ssize_t sent = Client_send(client, buffer, length, &send_error);
            if (send_error != 0 || sent < 0) {
                perror("Failed to send message");
                printf("Server disconnected\n");
                break;
            }
        }

        if (FD_ISSET(client->socket_file_descriptor, &read_file_descriptor_set)) {
            ssize_t received = recv(client->socket_file_descriptor, buffer, sizeof(buffer) - 1, 0);
            if (received <= 0) {
                printf("Server disconnected\n");
                is_running = 0;
                break;
            }
            buffer[received] = '\0';
            Console_add_message(console, buffer);
            Console_render(console);
        }
    }

    move_cursor(AT_EXIT_MESSAGE_ROW, 1);
    reset_terminal();
    show_cursor();
    printf("Exited successfully.\n");
}

