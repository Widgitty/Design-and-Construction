#ifndef __COMMS_H
#define __COMMS_H

//TODO: include serial and WiFi
#include <stdint.h>
#include "cmsis_os.h"

extern void Comms_Init(void);

extern void Comms_Send(double value, uint8_t mode, uint8_t range);
extern void Comms_Receive(void);
extern void Comms_Check_Mode(int *mode);

#endif
