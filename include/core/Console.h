#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/ANSI.h"

enum {
    MAX_MESSAGES = 10, // maximum number of messages in history
    MAX_MESSAGE_LENGTH = 100, // maximum length of a single message
    PROMPT_ROW = MAX_MESSAGES + 2,
    AT_EXIT_MESSAGE_ROW = 13 // row for message after user exited
};

typedef struct {
    char messages[MAX_MESSAGES][MAX_MESSAGE_LENGTH];
    int count;
} Console;

void Console_init(Console *console);
void Console_add_message(Console *console, const char *message);
void Console_render(const Console *console);

