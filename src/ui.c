#include <stdint.h>

#include "database.h"
#include "dtekv-lib.h"
#include "rfid.h"
#include "screen.h"
#include "timer.h"

#define GPIO_DATA ((volatile uint32_t*)0x040000e0)
#define BUTTON_1_BIT 26
#define BUTTON_2_BIT 27

int menu = 0;

int get_btn() { return (*GPIO_DATA >> BUTTON_1_BIT) & 0x1; }

// GPIO bit 27 is used to pick a role while editing a user.
static int get_role_btn() { return (*GPIO_DATA >> BUTTON_2_BIT) & 0x1; }

// Blocks until a card is read
static void wait_for_card(uint8_t* uid) {
  while (rfid_request() == 0 || !rfid_anticoll(uid)) {
  }
}

// Returns 1 for button 1 and 2 for button 2. Waits for release so one
// physical press is handled only once.
static int wait_for_button(void) {
  while (1) {
    if (get_btn()) {
      while (get_btn()) {
        delay(10);
      }
      print("GPIO 26 button pressed\n");
      return 1;
    }
    if (get_role_btn()) {
      while (get_role_btn()) {
        delay(10);
      }
      print("GPIO 27 button pressed\n");
      return 2;
    }
    delay(10);
  }
}

// Prompts for a card and returns 1 if it belongs to an admin
static int scan_admin() {
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Scan admin card", 0);

  uint8_t uid[4];
  wait_for_card(uid);

  User* user = db_get_user(uid);
  return user != 0 && user->role == 1;
}

void add_user() {
  static char* names[] = {"name1", "name2", "name3", "name4", "name5"};
  int name_index = 0;
  int role = 0;

  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Add User", 1000);
  lcd_set_cursor(1, 0);
  lcd_write_string("Scan card", 0);

  uint8_t uid[4];
  wait_for_card(uid);

  if (db_find_user(uid) >= 0) {
    lcd_clear();
    lcd_write_string("User exists", 1000);
    return;
  }

  // Do not treat the button press that entered this menu as a selection.
  while (get_btn() || get_role_btn()) {
    delay(10);
  }

  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Choose name", 0);
  lcd_set_cursor(1, 0);
  lcd_write_string(names[name_index], 0);

  while (1) {
    if (wait_for_button() == 1) {
      name_index = (name_index + 1) % 5;
      lcd_clear();
      lcd_set_cursor(0, 0);
      lcd_write_string("Choose name", 0);
      lcd_set_cursor(1, 0);
      lcd_write_string(names[name_index], 0);
    } else {
      break;
    }
  }

  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Admin mode?", 0);
  lcd_set_cursor(1, 0);
  lcd_write_string("0", 0);

  while (1) {
    if (wait_for_button() == 1) {
      role = !role;
      lcd_clear();
      lcd_set_cursor(0, 0);
      lcd_write_string("Admin mode?", 0);
      lcd_set_cursor(1, 0);
      lcd_write_string(role ? "1" : "0", 0);
    } else {
      break;
    }
  }

  db_add_user(uid, names[name_index], role);
  lcd_clear();
  lcd_write_string("User added", 1000);
}

void remove_user() {
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Remove User", 1000);
  lcd_set_cursor(1, 0);
  lcd_write_string("Scan card", 0);

  uint8_t uid[4];
  wait_for_card(uid);

  lcd_clear();
  if (db_remove_user(uid) == 0) {
    lcd_write_string("User removed", 1000);
  } else {
    lcd_write_string("User not found", 1000);
  }
}

void edit_user() {
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Edit User", 1000);
  lcd_set_cursor(1, 0);
  lcd_write_string("Scan card", 0);

  uint8_t uid[4];
  wait_for_card(uid);

  User* user = db_get_user(uid);
  if (user == 0) {
    lcd_clear();
    lcd_write_string("User not found", 1000);
    return;
  }

  // Do not treat the button press that entered this menu as a selection.
  while (get_btn() || get_role_btn()) {
    delay(10);
  }

  int role = user->role ? 1 : 0;
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_write_string("Admin mode?", 0);
  lcd_set_cursor(1, 0);
  lcd_write_string(role ? "1" : "0", 0);

  // Button 1 toggles the role; button 2 confirms it.
  while (1) {
    if (wait_for_button() == 1) {
      role = !role;
      lcd_clear();
      lcd_set_cursor(0, 0);
      lcd_write_string("Admin mode?", 0);
      lcd_set_cursor(1, 0);
      lcd_write_string(role ? "1" : "0", 0);
    } else {
      break;
    }
  }

  user->role = role;
  lcd_clear();
  lcd_write_string("User updated", 1000);
}

void menu_selector() {
  static char* menu_strings[] = {"Add User", "Remove User", "Edit User",
                                 "Exit"};

  if (!scan_admin()) {
    lcd_clear();
    lcd_write_string("Not admin", 1000);
    return;
  }

  while (1) {
    lcd_clear();
    lcd_write_string(menu_strings[menu], 0);

    // Button 1 cycles through the menu; button 2 confirms the selection.
    while (1) {
      if (wait_for_button() == 1) {
        menu = (menu + 1) % 4;
        lcd_clear();
        lcd_write_string(menu_strings[menu], 0);
      } else {
        break;
      }
    }

    switch (menu) {
      case 0:
        add_user();
        break;
      case 1:
        remove_user();
        break;
      case 2:
        edit_user();
        break;
      case 3:
        return;
    }
  }
}
