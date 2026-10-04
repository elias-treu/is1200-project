#include <stdint.h>

#include "timer.h"

extern void print(const char*);

// GPIO addresses, pins 0-31
#define GPIO_DATA ((volatile uint32_t*)0x040000e0)
#define GPIO_DIR ((volatile uint32_t*)0x040000e4)

#define RS_BIT (1 << 10)  // GPIO pin 10, physical pin 13
#define E_BIT (1 << 11)   // GPIO pin 11, physical pin 14
#define DATA_BITS_MASK \
  (0xF << 12)  // Mask for data bits, setting GPIO_12-15, physical pins 15-18
#define GPIO_10_15 (RS_BIT | E_BIT | DATA_BITS_MASK)

// Sends a 4-bit nibble. If is_data is 1, nibble is sent as data. If 0, it is
// sent as a command
void lcd_send_nibble(uint8_t nibble, int is_data) {
  uint32_t gpio_reg_value = *GPIO_DATA;

  // Clear GPIO 10 to 15
  gpio_reg_value &= ~GPIO_10_15;

  // Set RS bit to 1 if is_data
  if (is_data) {
    gpio_reg_value |= RS_BIT;
  }

  // Set all data bits to given nibble
  gpio_reg_value |= (nibble & 0x0F) << 12;

  // Apply to register
  *GPIO_DATA = gpio_reg_value;

  // Pulse ENABLE HIGH
  *GPIO_DATA |= E_BIT;
  delay(1);
  *GPIO_DATA &= ~E_BIT;
  delay(1);
}

// Sends an 8 bit command in 4-bit mode by splitting them into two nibbles that
// get sent one after another
void lcd_send_byte(uint8_t byte, int is_data) {
  uint8_t high_nibble = byte >> 4 & 0x0F;
  uint8_t low_nibble = byte & 0x0F;

  lcd_send_nibble(high_nibble, is_data);
  lcd_send_nibble(low_nibble, is_data);
}

// Sends a full 8-bit command. Handles longer delays for clear and home
// commands
void lcd_send_command(uint8_t command) {
  lcd_send_byte(command, 0);
  // Clear and Home commands both take about 1.53ms to execute, and are
  // the only commands that take longer than a couple microseconds.
  if (command == 0x01 || command == 0x02) {
    delay(2);
  }
}

void lcd_write_char(char c) { lcd_send_byte((uint8_t)c, 1); }

void lcd_write_string(char* str) {
  while (*str != 0) {
    lcd_write_char(*str);
    str++;
  }
}

void lcd_set_cursor(uint8_t line, uint8_t index) {
  // Line 1 starts at 0x00, line 2 starts at 0x40
  int address = 0;
  if (line == 0) {
    address = 0x00 + index;
  } else {
    address = 0x40 + index;
  }
  // Set DDRAM address command
  lcd_send_command(0x80 | address);
}

void lcd_clear() { lcd_send_command(0x01); }

void lcd_init() {
  // Follows 4-bit initialization procedure described in LCD documentation

  // Set data bits as output
  *GPIO_DIR |= GPIO_10_15;
  // Set data bits to 0
  *GPIO_DATA &= ~GPIO_10_15;

  // Wait > 40ms.
  delay(50);

  // Resynchronization
  lcd_send_nibble(0x03, 0);
  // Wait > 4.1ms
  delay(5);

  lcd_send_nibble(0x03, 0);
  // Wait > 100 us
  delay(1);

  lcd_send_nibble(0x03, 0);
  // Wait > 150 us
  delay(1);

  // The screen is now in 8-bit mode
  // Send 0x2 in 8-bit mode, setting the screen to 4-bit mode
  lcd_send_nibble(0x02, 0);
  delay(1);

  // Function set command: 4-bit mode, 2 lines, 5x8 font
  lcd_send_command(0x28);
  // Display ON/OFF command: Display ON, cursor OFF, blink OFF
  lcd_send_command(0x0C);
  lcd_clear();
  // Entry Mode Set command: Increment cursor, Shift off
  lcd_send_command(0x06);
  print("Completed LCD initialization.\n");
}
