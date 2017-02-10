
#include "cmsis_os.h"
#include <stdio.h>
#include "STM32F4xx_hal.h"
#include "LED.h"
#include "SWT.h"
#include "LCD.h"
#include "RTE_Components.h"
#include "ADC.h"
#include "math.h"
#include "System_Init.h"
#include "System_Thread.h"
#include "LCD_Thread.h"

#define Delay osDelay


void Thread_LCD (void const *argument);                 // thread function
osThreadId tid_Thread_LCD;                              // thread id
// Thread priority set to high, as system thread should not be blockable
osThreadDef(Thread_LCD, osPriorityHigh, 1, 0);        // thread object

int Init_Thread_LCD (void) {

  tid_Thread_LCD = osThreadCreate(osThread(Thread_LCD), NULL);
  if (!tid_Thread_LCD) return(-1);
  
  return(0);
}

void Thread_LCD (void const *argument) {
	LCD_GotoXY(0,0);
  LCD_PutS("BLAH");
	osEvent evt;
	osMessageQDef(MsgBox, 16, char[17]);
	osMessageQId  MsgBox;
	char *string;
	while(1){
		evt = osMessageGet(MsgBox, osWaitForever);  // wait for message
    if (evt.status == osEventMessage) {
      string = evt.value.p;
			LCD_GotoXY(0,0);
      LCD_PutS("Bloop");
    }
	}
}
