#ifndef DATABASE_H
#define DATABASE_H

#include <stdint.h>

typedef struct {
  uint8_t uid[7];   // 7 byte RFID length
  uint8_t uid_len;  // 1 byte UID length
  char name[17];    // 17 bytes for 16 byte name + null character
  uint8_t role;     // 1 byte for Admin vs User
} User;

void db_init(void);
int db_find_user(const uint8_t* uid, int uid_len);
User* db_get_user(const uint8_t* uid, int uid_len);
int db_add_user(const uint8_t* uid, uint8_t uid_len, const char* name,
                uint8_t role);
int db_remove_user(const uint8_t* uid, int uid_len);

#endif
