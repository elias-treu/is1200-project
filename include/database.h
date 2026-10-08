#ifndef DATABASE_H
#define DATABASE_H

#include <stdint.h>

typedef struct {
  uint8_t uid[4];  // 4-byte UIDs
  char name[17];   // 17 bytes for 16 byte name + null character
  uint8_t role;    // 1 byte for Admin vs User
} User;

void db_init(void);
int db_find_user(const uint8_t* uid);
User* db_get_user(const uint8_t* uid);
int db_add_user(const uint8_t* uid, const char* name, uint8_t role);
int db_remove_user(const uint8_t* uid);

#endif
