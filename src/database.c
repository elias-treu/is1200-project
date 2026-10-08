// Mostly implemented by Erik Forsberg, reviewed with Elias Treutiger

#include <stddef.h>
#include <stdint.h>

#include "database.h"

#define MAX_USERS 20

static User database[MAX_USERS];
static int user_count;

void db_init() { user_count = 0; }

// Checks if user exists in database and returns its index if it does, otherwise
// returns -1
int db_find_user(const uint8_t* uid) {
  for (int i = 0; i < user_count; i++) {
    // assume match
    int match = 1;
    // compare byte by byte
    for (int j = 0; j < 4; j++) {
      if (database[i].uid[j] != uid[j]) {
        // declar not matching and escape for loop
        match = 0;
        break;
      }
    }
    // return current index if match, otherwise return -1
    if (match) {
      return i;
    }
  }
  return -1;
}

// Looks up a user and returns a pointer to the user if found,
// otherwise returns NULL
User* db_get_user(const uint8_t* uid) {
  int index = db_find_user(uid);
  if (index < 0) {
    return NULL;
  }
  return &database[index];
}

// Adds a user to database and returns a status code.
// Guarantee null termination
// -1 - Database full
// -2 - User already exists
int db_add_user(const uint8_t* uid, const char* name, uint8_t role) {
  if (user_count >= MAX_USERS) {
    return -1;
  }

  if (db_find_user(uid) >= 0) {
    return -2;
  }

  for (int i = 0; i < 4; i++) {
    database[user_count].uid[i] = uid[i];
  }

  int i = 0;
  while (name[i] != '\0' && i < 16) {
    database[user_count].name[i] = name[i];
    i++;
  }
  // ensure null termination
  database[user_count].name[i] = '\0';

  database[user_count].role = role;

  user_count++;
  return 0;
}

// Removes user from database.
// Return 0 on successful removal
// Returns -1 if user is not in database
int db_remove_user(const uint8_t* uid) {
  int index = db_find_user(uid);
  if (index < 0) {
    return -1;
  }

  // Shift remaining users left without reading past the last occupied slot
  for (int i = index; i < user_count - 1; i++) {
    database[i].role = database[i + 1].role;

    for (int k = 0; k < 4; k++) {
      database[i].uid[k] = database[i + 1].uid[k];
    }
    for (int k = 0; k < 17; k++) {
      database[i].name[k] = database[i + 1].name[k];
    }
  }

  user_count--;
  // Clear the last user's data
  database[user_count].role = 0;
  for (int k = 0; k < 4; k++) {
    database[user_count].uid[k] = 0;
  }
  for (int k = 0; k < 17; k++) {
    database[user_count].name[k] = '\0';
  }
  return 0;
}
