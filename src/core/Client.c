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

void client_init(Client *client, const char *server_ip, int port, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }
    
    if (!client || !server_ip || port <= 0 || port > 65535) {
        if (error_flag) {
            *error_flag = 1;
        }
        return;
    }

    memset(client, 0, sizeof(Client));

    client->socket_file_descriptor = -1;
    client->is_connected = 0;
    client->server_address.sin_family = AF_INET;
    client->server_address.sin_port = htons((uint16_t) port);
    
    if (inet_pton(AF_INET, server_ip, &client->server_address.sin_addr) != 1) {
        fprintf(stderr, "Invalid IP address: %s\n", server_ip);
        if (error_flag) {
            *error_flag = 1;
        }
        return;
    }
}

void client_connect(Client *client, int *error_flag) {
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

ssize_t client_send(Client *client, const char *message, size_t len, int *error_flag) {
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

ssize_t client_receive(Client *client, char *buffer, size_t buffer_size, int *error_flag) {
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

void client_destroy(Client *client) {
    if (!client) {
        return;
    }
    
    if (client->socket_file_descriptor >= 0) {
        close(client->socket_file_descriptor);
        client->socket_file_descriptor = -1;
    }
    
    client->is_connected = 0;
    memset(&client->server_address, 0, sizeof(client->server_address));
}

int client_is_connected(const Client *client) {
    return client && client->is_connected;
}

void client_run(Client *client) {
    printf("Type your messages (press Ctrl+D to exit):\n");

    int max_file_descriptor;
    if (client->socket_file_descriptor > STDIN_FILENO) {
        max_file_descriptor = client->socket_file_descriptor;
    } else {
        max_file_descriptor = STDIN_FILENO;
    }
    
    char buffer[BUFSIZ];
    while (1) {
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
                printf("Exiting\n");
                break;
            }

            size_t length = strlen(buffer);
            if (length == 0) {
                continue;
            }

            int send_error = 0;
            ssize_t sent = client_send(client, buffer, length, &send_error);
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
                break;
            }
            buffer[received] = '\0';
            printf("%s", buffer);
        }
    }

}
