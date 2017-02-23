#ifndef __Serial_H
#define __Serial_H
#include <stdint.h>
#include "cmsis_os.h"

extern void SerialInit(void);
extern void SerialSend(uint8_t *pData, uint16_t Size, uint32_t Timeout);
extern void SerialReceiveStart(void);
extern void SerialReceive(void);
extern void SerialCheckMode(int *mode);

#endif
