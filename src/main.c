#include <stdint.h>

#include "database.h"
#include "screen.h"

extern void print(const char*);

void handle_interrupt(unsigned cause) { (void)cause; }

void boot_setup() {
  // clear database
  db_init();

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
  lcd_init();

  lcd_set_cursor(0, 0);
  lcd_write_string("TEST STRING!");

  lcd_set_cursor(1, 0);
  lcd_write_string("LOWER STRING!");
  print("end of main()\n");

  boot_setup();

  while (1);
}
