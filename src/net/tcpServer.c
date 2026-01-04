#include "net/TcpServer.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

enum {
    LISTEN_BACKLOG = 10
};

int tcpServer_create_listening_socket(int port, int *error_flag) {
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

int tcpServer_accept_client(int listen_file_descriptor, struct sockaddr_in *client_address, int *error_flag) {
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
