#ifndef __CALCULATIONS_H
#define __CALCULATIONS_H

extern double adcConv(int mode, int *range);
extern double movAvg(double avgIn, int mode, int *rangep);
extern void calibrate(int mode, int *rangep);

#endif
