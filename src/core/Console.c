#include "core/Console.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/ANSI.h"

struct Console {
    char messages[MAX_MESSAGES_COUNT][MAX_MESSAGE_LENGTH];
    int count;
};

Console *Console_create(int *error_flag) {
    if (error_flag) {
        *error_flag = 0;
    }

    Console *console = calloc(1, sizeof(Console));
    if (!console) {
        *error_flag = 0;
        return NULL;
    }

    for (int i = 0; i < MAX_MESSAGES_COUNT; ++i) {
        console->messages[i][0] = '\0';
    }
    console->count = 0;
    
    return console;
}

void Console_add_message(Console *console, const char *message) {
    if (console == NULL || message == NULL) {
        return;
    }
    
    for (int i = 0; i < MAX_MESSAGES_COUNT - 1; ++i) {
        strcpy(console->messages[i], console->messages[i + 1]);
    }
    
    strncpy(console->messages[MAX_MESSAGES_COUNT - 1], message, MAX_MESSAGE_LENGTH - 1);
    console->messages[MAX_MESSAGES_COUNT - 1][MAX_MESSAGE_LENGTH - 1] = '\0';
    
    if (console->count < MAX_MESSAGES_COUNT) {
        ++console->count;
    }
}

void Console_render(const Console *console) {
    if (console == NULL) {
        return;
    }
    
    move_cursor(1, 1);
    printf("=== Enter commands (type 'exit()' to quit) ===\n");
    for (int i = 0; i < MAX_MESSAGES_COUNT; ++i) {
        move_cursor(i + 2, 1); // + 2 because row 1 is header
        clear_line();
        
        if (i < console->count && console->messages[i][0] != '\0') {
            printf("%s", console->messages[i]);
        }
        
        if (i < MAX_MESSAGES_COUNT - 1) {
            printf("\n");
        }
    }
    
    move_cursor(PROMPT_ROW, 1);
    clear_line();
    printf("> ");
    
    fflush(stdout);
}
