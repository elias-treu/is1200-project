#include "timer.h"
#include <stdint.h>

extern void print(const char *);

// Define GPIO addresses
#define GPIO_DATA ((volatile uint32_t *)0x040000e0)
#define GPIO_DIR ((volatile uint32_t *)0x040000e4)

#define RS_BIT (1 << 10)           // GPIO pin 10, physical pin 13
#define ENABLE_BIT (1 << 11)       // GPIO pin 11, physical pin 14
#define DATA_BITS (1 << 12)        // GPIO pin 12-15, physical pin 15-18
#define DATA_BITS_MASK (0xF << 12) // Mask for data bits, setting bits 12-15
#define GPIO_10_18 (RS_BIT | ENABLE_BIT | DATA_BITS_MASK)

int main(void) {
  // Configure GPIO ports 10 through 15 as output.
  *GPIO_DIR = *GPIO_DIR | GPIO_10_18;

  while (1) {
    // Set ENABLE to HIGH.
    print("ENABLE HIGH\n");
    *GPIO_DATA |= ENABLE_BIT;
    delay(1000);
    // Set ENABLE to LOW. NOT operator ~ on ENABLE_BIT causes it to drive pin 11
    // low.
    print("ENABLE LOW\n");
    *GPIO_DATA &= ~ENABLE_BIT;
    delay(1000);
  }
  return 0;
}
