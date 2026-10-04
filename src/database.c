#include <stddef.h>
#include <stdint.h>

#include "database.h"

#define MAX_USERS 20

static User database[MAX_USERS];
static int user_count;

void db_init() { user_count = 0; }

// Checks if user exists in database and returns its index if it does, otherwise
// returns -1
int db_find_user(const uint8_t* uid, int uid_len) {
  for (int i = 0; i < user_count; i++) {
    if (database[i].uid_len != uid_len) {
      continue;
    }
    // assume match
    int match = 1;
    // compare byte by byte
    for (int j = 0; j < uid_len; j++) {
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
User* db_get_user(const uint8_t* uid, int uid_len) {
  int index = db_find_user(uid, uid_len);
  if (index < 0) {
    return NULL;
  }
  return &database[index];
}

// Adds a user to database and returns a status code.
// Guarantee null termination - Success
// -1 - Database full
// -2 - User already exists
int db_add_user(const uint8_t* uid, uint8_t uid_len, const char* name,
                uint8_t role) {
  if (user_count >= MAX_USERS) {
    return -1;
  }

  if (db_find_user(uid, uid_len) >= 0) {
    return -2;
  }

  for (int i = 0; i < uid_len; i++) {
    database[user_count].uid[i] = uid[i];
  }

  database[user_count].uid_len = uid_len;

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
int db_remove_user(const uint8_t* uid, int uid_len) {
  int index = db_find_user(uid, uid_len);
  if (index < 0) {
    return -1;
  }

  for (int i = index; i < user_count; i++) {
    // Every field in a user struct is copied field by field to avoid the
    // compilar using memcpy which the files in dtekv-lib does not support.
    // database[i] = database[i + 1];
    // everything in this if statement below this line replaces the line
    // above.
    database[i].uid_len = database[i + 1].uid_len;
    database[i].role = database[i + 1].role;

    // Copy UID array
    for (int k = 0; k < database[i + 1].uid_len; k++) {
      database[i].uid[k] = database[i + 1].uid[k];
    }

    // Copy Name string
    int k = 0;
    while (database[i + 1].name[k] != '\0' && k < 16) {
      database[i].name[k] = database[i + 1].name[k];
      k++;
    }
  }

  user_count--;
  return 0;
}
