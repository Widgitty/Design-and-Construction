#ifndef __Serial_H
#define __Serial_H
#include <stdint.h>
#include "cmsis_os.h"

extern int Check_For_Serial(void);
extern void SerialInit(void);
extern void SerialReceiveStart(void);
extern void Serial_Receive(void);
extern void Serial_Check_Mode(int *mode);
extern void Serial_Send(uint8_t *pData, uint16_t Size);

extern void Register_RX_Handler(void (*RXHandlerLocal) (uint8_t));
extern void Deregister_RX_Handler(void);

#endif
