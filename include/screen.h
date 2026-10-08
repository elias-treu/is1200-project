#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

void lcd_init();
void lcd_send_command(uint8_t);

void lcd_clear();
void lcd_set_cursor(uint8_t line, uint8_t index);
void lcd_write_char(char c);
void lcd_write_string(char* str, int delay_ms);

#endif
