#ifndef RFID_H
#define RFID_H

#include <stdint.h>

void rfid_init(void);
uint8_t rfid_request(void);
void rfid_write_reg(uint8_t reg, uint8_t value);
uint8_t rfid_read_reg(uint8_t reg);
uint8_t rfid_anticoll(uint8_t* uid_buffer);

#endif