#ifndef __LCD_Thread_H
#define __LCD_Thread_H

extern osMessageQId MsgBox;
extern osPoolId  mpool;

extern void Thread_LCD (void const *argument);
extern int Init_Thread_LCD (void);

#endif
