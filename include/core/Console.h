#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/ANSI.h"

enum {
    MESSAGES_DISPLAYED = 10,
    MAX_MESSAGE_LENGTH = 100,
    PROMPT_ROW = MESSAGES_DISPLAYED + 2,
    AT_EXIT_MESSAGE_ROW = 13 // row for message after user exited
};

typedef struct Console Console;

Console *Console_create(int *error_flag);
void Console_destroy(Console *console);

void Console_add_message(Console *console, const char *message);
void Console_render(const Console *console);

