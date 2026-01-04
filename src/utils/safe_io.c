#include "utils/safe_io.h"

#include <errno.h>
#include <unistd.h>

ssize_t safe_read(int file_descriptor, void *buffer, size_t count, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }
    
    while (1) {
        ssize_t bytes_read = read(file_descriptor, buffer, count);
        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (error_flag) {
                *error_flag = 1;
            }
        }
        return bytes_read;
    }
}

ssize_t safe_write(int file_descriptor, const void *buffer, size_t count, int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }
    
    size_t written_total = 0;
    const char *char_buffer = (const char *) buffer;

    while (written_total < count) {
        ssize_t written = write(file_descriptor, char_buffer + written_total, count - written_total);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (error_flag) {
                *error_flag = 1;
            }
            return -1;
        }
        written_total += (size_t) written;
    }
    return (ssize_t) written_total;
}
