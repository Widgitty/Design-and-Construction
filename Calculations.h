#ifndef __CALCULATIONS_H
#define __CALCULATIONS_H

extern double switchRange(int value, int *rangep, double scale);
extern double adcConv(int mode, double value, int *range);
extern double movAvg(double avgIn, int mode);

#endif
