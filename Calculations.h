#ifndef __CALCULATIONS_H
#define __CALCULATIONS_H

extern double adcConv(int mode, double value, int *range);
extern double movAvg(double avgIn, int mode, int *rangep);
void setTimerValue(int timer);

#endif
