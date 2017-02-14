
#include "cmsis_os.h"
#include <stdio.h>
#include "LCD.h"
#include "LCD_Thread.h"

#define Delay osDelay

void LCD_Write_At(char *string, int x, int y, int reset) {
	LCD_Command_t *command = (LCD_Command_t*)osPoolAlloc(mpool);
	sprintf(command->string, "%s",string);
	command->x = x;
	command->y = y;
	command->reset = reset;
	osMessagePut(MsgBox, (uint32_t)command, osWaitForever);
}


void Thread_LCD (void const *argument);                 // thread function
osThreadId tid_Thread_LCD;                              // thread id
// Thread priority set to high, as system thread should not be blockable
osThreadDef(Thread_LCD, osPriorityHigh, 1, 0);        // thread object

osMessageQId MsgBox;
osPoolId  mpool;

int Init_Thread_LCD (void) {

  tid_Thread_LCD = osThreadCreate(osThread(Thread_LCD), NULL);
  if (!tid_Thread_LCD) return(-1);
  
  return(0);
}

void Thread_LCD (void const *argument) {
	osPoolDef(mpool, 16, LCD_Command_t);
	mpool = osPoolCreate(osPool(mpool));
	osMessageQDef(MsgBox, 16, LCD_Command_t);
	MsgBox = osMessageCreate(osMessageQ(MsgBox), NULL);

	while(1){
		osEvent evt = osMessageGet(MsgBox, osWaitForever);  // wait for message
    if (evt.status == osEventMessage) {
      LCD_Command_t *command = (LCD_Command_t*)evt.value.p;
			if (command->reset == 1){
				LCD_Clear();
			}
			LCD_GotoXY(command->x,command->y);
      LCD_PutS(command->string);
			osPoolFree(mpool, command);
    }
	}
}
