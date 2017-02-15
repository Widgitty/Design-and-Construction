#ifndef __Serial_H
#define __Serial_H
#include <stdint.h>

extern void Error(int);
extern void SerialSend(uint8_t *pData, uint16_t Size, uint32_t Timeout);
extern void Serial(void);

#endif
