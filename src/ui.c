#include "rfid.h"
#include "screen.h"
#include "database.h"

// bit 1 = KEY1, used to pick a role while editing a user
static int get_role_btn() {
  volatile int* address = (volatile int*)0x040000d0;
  return (*address >> 1) & 0x1;
}

// Blocks until a card is read
static void wait_for_card(uint8_t* uid) {
  while (rfid_request() == 0 || !rfid_anticoll(uid)) {
  }
}

// Prompts for a card and returns 1 if it belongs to an admin
static int scan_admin() {
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Scan admin card");

  uint8_t uid[4];
  wait_for_card(uid);

  User* user = db_get_user(uid, 4);
  return user != 0 && user->role == 1;
}

void add_user() {
  if (!scan_admin()) {
    lcd_clear();
    lcd_write_string("Not admin");
    return;
  }

  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Add User");
  lcd_set_cursor(1, 0);
  lcd_write_string("Scan card");

  uint8_t uid[4];
  wait_for_card(uid);
  db_add_user(uid, 4, "User", 0);
}

void remove_user() {
  if (!scan_admin()) {
    lcd_clear();
    lcd_write_string("Not admin");
    return;
  }

  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Remove User");
  lcd_set_cursor(1, 0);
  lcd_write_string("Scan card");

  uint8_t uid[4];
  wait_for_card(uid);
  db_remove_user(uid, 4);
}

void edit_user() {
  if (!scan_admin()) {
    lcd_clear();
    lcd_write_string("Not admin");
    return;
  }

  int role = 0;
  int last_btn = get_role_btn();

  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Edit User");
  lcd_set_cursor(1, 0);
  lcd_write_string("Role: User");

  while (1) {
    // KEY1 toggles the role that will be applied
    int btn = get_role_btn();
    if (btn && !last_btn) {
      role = !role;
      lcd_clear();
      lcd_set_cursor(0, 0);
      lcd_write_string("Edit User");
      lcd_set_cursor(1, 0);
      lcd_write_string(role ? "Role: Admin" : "Role: User");
    }
    last_btn = btn;

    // Scan a card to apply the selected role
    uint8_t uid[4];
    if (rfid_request() != 0 && rfid_anticoll(uid)) {
      User* user = db_get_user(uid, 4);
      if (user != 0) {
        user->role = role;
      }
      break;
    }
  }
}
