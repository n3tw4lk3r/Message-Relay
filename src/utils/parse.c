#include "utils/parse.h"

#include <stdlib.h>

int parse_port(const char *arg, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }
    
    if (!arg) {
        if (error_flag) {
            *error_flag = 1;
        }
        return -1;
    }
    
    char *end = NULL;
    long port = strtol(arg, &end, 10);

    if (*arg == '\0' || *end != '\0') {
        if (error_flag) {
            *error_flag = 1;
        }
        return -1;
    }
    
    if (port <= 0 || port > 65535) {
        if (error_flag) {
            *error_flag = 1;
        }
        return -1;
    }

    return (int) port;
}
