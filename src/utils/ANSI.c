#include "utils/ANSI.h"

#include <stdio.h>

void clear_screen() {
    printf("\033[2J");
}

void clear_line() {
    printf("\033[2K");
}

void move_cursor(int row, int col) {
    printf("\033[%d;%dH", row, col);
}

void hide_cursor() {
    printf("\033[?25l");
}

void show_cursor() {
    printf("\033[?25h");
}

void reset_terminal() {
    printf("\033[0m");
}
