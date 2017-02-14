#ifndef __LCD_Thread_H
#define __LCD_Thread_H
#include "cmsis_os.h"

typedef struct {
    char string[17];
    int x;
    int y;
		int reset;
} LCD_Command_t;

extern osMessageQId MsgBox;
extern osPoolId  mpool;

extern void LCD_Write_At(char string[17], int x, int y, int reset);
extern void Thread_LCD (void const *argument);
extern int Init_Thread_LCD (void);

#endif
