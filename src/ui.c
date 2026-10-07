#include "rfid.h"
#include "screen.h"
void add_user() {
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Add User");
  lcd_set_cursor(1, 0);
  lcd_write_string("Scan card to add user");
}

void remove_user() {
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Remove User");
  lcd_set_cursor(1, 0);
  lcd_write_string("Scan card to remove user");
}

void edit_user() {
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Edit User");
  lcd_set_cursor(1, 0);
  lcd_write_string("Scan card to edit user permissions");
}
