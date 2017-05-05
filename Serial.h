#ifndef __Serial_H
#define __Serial_H
#include <stdint.h>
#include "cmsis_os.h"

typedef enum 
{
  SERIAL_OK       = 0x00U,
  SERIAL_ERROR    = 0x01U,
  SERIAL_BUSY     = 0x02U,
} Serial_StatusTypeDef;

extern int serial_Busy;

extern void SerialInit(void);
extern void SerialReceiveStart(void);
extern void Serial_Receive(void);
extern void Serial_Check_Mode(int *mode);
extern Serial_StatusTypeDef Serial_Send(uint8_t *pData, uint16_t Size);

extern void Register_RX_Handler(void (*RXHandlerLocal) (uint8_t));
extern void Deregister_RX_Handler(void);

#endif
