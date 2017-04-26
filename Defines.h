#define NANO 0
#define MICRO 1
#define MILLI 2
#define UNIT 3
#define KILO 4
#define MEGA 5
#define UNIT30 6



#define CURRENTSCALE 0.66
#define VOLTAGESCALE 6.66
#define INPUTCURRENT 0.000003


// 
#define CURRMODE 0
#define VOLTMODE 1
#define RESMODE 2
#define CAPMODE 3
#define INDMODE 4
#define FREQMODE 5
#define CONTMODE 6
#define RMS 7
#define DIODE 8

// different M_ultiplexing ranges.

#define M_UNIT 0
#define M_TEN 1
#define M_HUNDRET 2
#define M_THOUSAND 3


// timer related defines:

// these are values that the clock frequency is divided by to generate a prescaler
#define PRESCALERXL 500000
#define PRESCALERL 50000
#define PRESCALERS 5000
#define PRESCALERM 10000
// these are the related PERIODS, (values the timers count up to)
#define PERIODL 50000
#define PERIODS 1000

// define to get frequency in seconds rather than MICROseconds:
#define FREQSCALER 1000000

