#include <stdint.h>

#include "database.h"
#include "dtekv-lib.h"
#include "rfid.h"
#include "screen.h"
#include "timer.h"
#include "ui.h"

#define BUTTON_BASE ((volatile uint32_t*)0x040000d0)
#define GPIO_DIR ((volatile uint32_t*)0x040000e4)
#define GPIO_INPUT_BUTTONS ((1u << 26) | (1u << 27))

extern void print_dec(unsigned int);
extern void enable_interrupt();

static volatile int menu_requested;

void handle_interrupt(unsigned cause) {
  if (cause == 18) {
    BUTTON_BASE[3] = 1;
    if (*BUTTON_BASE == 1) {
      menu_requested = 1;
    }
  }
}

void boot_setup() {
  // clear database
  db_init();
  // initialize RFID
  rfid_init();
  // initialize LCD
  lcd_init();

  // Receive admin info.
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Tap Admin card", 0);

  uint8_t admin_uid[4];

  while (1) {
    if (rfid_request() != 0) {
      if (rfid_anticoll(admin_uid)) {
        break;
      }
    }
  }

  db_add_user(admin_uid, "admin", 1);

  lcd_clear();
  lcd_write_string("Admin Set!", 1000);
}

int main() {
  // Configure GPIO 26 and GPIO 27 as inputs.
  *GPIO_DIR &= ~GPIO_INPUT_BUTTONS;

  BUTTON_BASE[3] = 1;
  BUTTON_BASE[2] |= 1;

  boot_setup();
  enable_interrupt();

  lcd_clear();
  lcd_write_string("Scan card", 0);

  while (1) {
    if (menu_requested) {
      menu_requested = 0;
      menu_selector();
      lcd_clear();
      lcd_write_string("Scan card", 0);
    }

    // Default mode: scan for RFID tags
    uint8_t card_uid[4];
    if (rfid_request() != 0) {
      if (rfid_anticoll(card_uid)) {
        // Successfully read a card!
        // card_uid[0..3] now holds the unique 4-byte ID (e.g., DE AD BE EF)
        if (db_find_user(card_uid) >= 0) {
          lcd_clear();
          lcd_write_string("Welcome, ", 0);
          lcd_set_cursor(1, 0);
          lcd_write_string(db_get_user(card_uid)->name, 1000);
          lcd_clear();
          lcd_write_string("Scan card", 0);
        } else {
          lcd_clear();
          lcd_write_string("Unknown card", 1000);
          delay(1000);
          lcd_clear();
          lcd_write_string("Scan card", 0);
        }
      }
    }
  }
}
