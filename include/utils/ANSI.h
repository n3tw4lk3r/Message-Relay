#pragma once

#include <stdio.h>

void clear_screen();
void clear_line();
void move_cursor(int row, int col);
void hide_cursor();
void show_cursor();
void reset_terminal(); // resets all terminal attributes: colors, cursor, etc
