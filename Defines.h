#define nano 0
#define micro 1
#define milli 2
#define UNIT 3
#define kilo 4
#define mega 5
#define UNIT30 6

#define currentScale 0.66
#define voltageScale 6.66
#define inputCurrent 0.000003
#define currMode 0
#define voltMode 1
#define resMode 2
#define capMode 3
#define indMode 4
#define freqMode 5


// timer related defines:

// these are values that the clock frequency is divided by to generate a prescaler
#define prescalerXL 500000
#define prescalerL 50000
#define prescalerS 5000
#define prescalerM 10000
// these are the related periods, (values the timers count up to)
#define periodL 50000
#define periodS 1000

// define to get frequency in seconds rather than microseconds:
#define frequencyScaler 1000000

