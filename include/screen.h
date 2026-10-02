#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>
#include <sys/types.h>

void lcd_init();
void send_command(uint8_t);

void clear_screen();
void set_cursor(uint8_t line, uint8_t index);
void write_char(char c);
void write_string(char *str);

#endif
