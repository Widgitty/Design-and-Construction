
#include "cmsis_os.h"
//#include <stdio.h>
#include "LCD.h"

#include "LCD_Thread.h"

#define Delay osDelay


void Thread_LCD (void const *argument);                 // thread function
osThreadId tid_Thread_LCD;                              // thread id
// Thread priority set to high, as system thread should not be blockable
osThreadDef(Thread_LCD, osPriorityHigh, 1, 0);        // thread object

osMessageQId MsgBox;

int Init_Thread_LCD (void) {

  tid_Thread_LCD = osThreadCreate(osThread(Thread_LCD), NULL);
  if (!tid_Thread_LCD) return(-1);
  
  return(0);
}

void Thread_LCD (void const *argument) {
	osMessageQDef(MsgBox, 16, char[17]);
	MsgBox = osMessageCreate(osMessageQ(MsgBox), NULL);
	LCD_GotoXY(0,0);
  LCD_PutS("BLAH");

	while(1){
		osEvent evt = osMessageGet(MsgBox, 10000);  // wait for message
    if (evt.status == osEventMessage) {
      char *stringp = (char*)evt.value.p;
			LCD_GotoXY(0,0);
      LCD_PutS(stringp);
			Delay(100);
    }
	}
}
