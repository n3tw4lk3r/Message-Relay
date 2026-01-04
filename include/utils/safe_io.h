#pragma once

#include <sys/types.h>

ssize_t safe_read(int file_descriptor, void *buffer, size_t count, int *error_flag);
ssize_t safe_write(int file_descriptor, const void *buffer, size_t count, int *error_flag);
