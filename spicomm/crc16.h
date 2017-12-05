#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define CRC16_INIT 0xFFFF
   
uint16_t crc16(const uint8_t *buffer, uint8_t len, uint16_t crc);


#ifdef __cplusplus
}
#endif

#endif 
