#define nano 0
#define micro 1
#define milli 2
#define nothing 3
#define kilo 4
#define mega 5

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

