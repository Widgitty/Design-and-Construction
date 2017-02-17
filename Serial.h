#ifndef __Serial_H
#define __Serial_H
#include <stdint.h>
#include "cmsis_os.h"

typedef struct {
    char string[17];
} Serial_rx_t;

extern osMessageQId SerialMsgBox;
extern osPoolId  SerialMpool;

extern void Error(int);
extern void SerialSend(uint8_t *pData, uint16_t Size, uint32_t Timeout);
extern void SerialReceive(void);
extern void SerialInit(void);

#endif
