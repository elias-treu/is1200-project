#include "timer.h"
#include <stdint.h>

extern void print(const char *);

// GPIO addresses, pins 0-31
#define GPIO_DATA ((volatile uint32_t *)0x040000e0)
#define GPIO_DIR ((volatile uint32_t *)0x040000e4)

#define RS_BIT (1 << 10) // GPIO pin 10, physical pin 13
#define E_BIT (1 << 11)  // GPIO pin 11, physical pin 14
#define DATA_BITS_MASK                                                         \
  (0xF << 12) // Mask for data bits, setting GPIO_12-15, physical pins 15-18
#define GPIO_10_18 (RS_BIT | E_BIT | DATA_BITS_MASK)

int main() {
  // Configure GPIO ports 10 through 15 as output.
  *GPIO_DIR = *GPIO_DIR | GPIO_10_18;

  while (1) {
    // Set ENABLE to HIGH.
    print("ENABLE HIGH\n");
    *GPIO_DATA |= E_BIT;
    delay(1000);
    // Set ENABLE to LOW. NOT operator ~ on ENABLE_BIT causes it to drive pin 11
    // low.
    print("ENABLE LOW\n");
    *GPIO_DATA &= ~E_BIT;
    delay(1000);
  }
  return 0;
}

void lcd_init() {
  // Follows 4-bit initialization procedure described in LCD documentation.

  // Set data bits as output
  *GPIO_DIR |= GPIO_10_18;
  // Set data bits to 0
  *GPIO_DATA &= ~GPIO_10_18;

  // Wait >40ms.
  delay(50);
}
