#ifndef __WiFi_H
#define __WiFi_H

#include <stdint.h>
#include "cmsis_os.h"

extern int WiFiEnabled;

extern int Check_For_WiFi(void);
extern void WiFi_Init(void);

extern void WiFi_Send(uint8_t *pData, uint16_t Size);
extern void WiFi_Receive(void);
extern void WiFi_Check_Mode(int *mode);

#endif
