#ifndef __CALCULATIONS_H
#define __CALCULATIONS_H
#include "Calibration.h"
#include "cmsis_os.h"


extern double adcConv(int mode, uint32_t value, int *range, calibAdjustTypeDef *calib_Data);
extern double movAvg(double avgIn, int mode, int *rangep);
void setTimerValue(int timer);
void countUp(void);
void setOutput(void);

#endif
