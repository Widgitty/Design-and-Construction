#ifndef __Serial_H
#define __Serial_H
#include <stdint.h>
#include "cmsis_os.h"

extern void SerialInit(void);
//extern void SerialSend(uint8_t *pData, uint16_t Size);
extern void SerialSend(double value, uint8_t mode, uint8_t range);
extern void SerialReceiveStart(void);
extern void SerialReceive(void);
extern void SerialCheckMode(int *mode);

#endif
