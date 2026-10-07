#include <stdint.h>

#include "database.h"
#include "dtekv-lib.h"
#include "rfid.h"
#include "screen.h"
#include "ui.h"

extern void print_dec(unsigned int);

void handle_interrupt(unsigned cause) { (void)cause; }

int get_btn() {
  volatile int* address = (volatile int*)0x040000d0;
  // returns an integer with all but the LSB set to 0.
  return *address & 0x1;
}

void boot_setup() {
  // clear database
  db_init();
  // initialize RFID
  rfid_init();
  // initialize LCD
  lcd_init();

  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Tap Admin card");

  uint8_t admin_uid[7];
  uint8_t admin_uid_len = 0;

  // Get RFID from card

  // Dummy values for testing until RFID code is ready
  admin_uid[0] = 0xAA;
  admin_uid[1] = 0xBB;
  admin_uid[2] = 0xCC;
  admin_uid[3] = 0xDD;
  admin_uid_len = 4;

  // Add user as admin
  db_add_user(admin_uid, admin_uid_len, "Admin", 1);

  lcd_clear();
  lcd_write_string("Admin Set!");
}

int main() {
  boot_setup();

  lcd_clear();
  lcd_write_string("Scan card");

  int menu = 0;  // 0 = scan, 1 = add, 2 = remove, 3 = edit
  while (1) {
    if (get_btn() == 1) {
      menu = (menu + 1) % 4;
      switch (menu) {
        case 0:
          lcd_clear();
          lcd_write_string("Scan card");
          break;
        case 1:
          add_user();
          break;
        case 2:
          remove_user();
          break;
        case 3:
          edit_user();
          break;
      }
    }

    // Default mode: scan for RFID tags
    if (menu == 0) {
      uint8_t card_uid[4];
      if (rfid_request() != 0) {
        if (rfid_anticoll(card_uid)) {
          // Successfully read a card!
          // card_uid[0..3] now holds the unique 4-byte ID (e.g., DE AD BE EF)
          for (int i = 0; i < 4; i++) {
            print_dec(card_uid[i]);
            print(" ");
          }
        }
      }
    }
  }
}
