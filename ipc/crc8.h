#ifndef CRC8_H
#define CRC8_H

#include <stdint.h>

#define CRC8_INIT 0x00

extern const uint8_t crc8_table[];

uint8_t crc8_bytes(uint8_t crc, const uint8_t *buffer, uint8_t len);
static inline uint8_t crc8_byte(uint8_t crc, uint8_t data) { return crc8_table[crc ^ data]; }

#endif 
