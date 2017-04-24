#ifndef __CALCULATIONS_H
#define __CALCULATIONS_H
#include "Calibration.h"

extern double adcConv(int mode, double value, int *range, calibAdjustTypeDef *calib_Data);
extern double movAvg(double avgIn, int mode, int *rangep);
void setTimerValue(int timer);
void countUp(void);
void setOutput(void);

#endif
