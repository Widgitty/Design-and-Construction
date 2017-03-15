#ifndef __CALIBRATION_H
#define __CALIBRATION_H

// Structure definitions
typedef struct {
	double point1;
	double point2;
} testStructTpeDef;


// Calibration structures
typedef struct {
	double lowerPoint;
	double upperPoint;
	double zeroOffset;
	double multiplier;
} calibRangeTpeDef;


typedef struct {
	calibRangeTpeDef voltage;
	calibRangeTpeDef current;
	calibRangeTpeDef resistance;
} calibStructTypeDef;


extern void Calibrate(int mode, int range);


/* LED Definitions */
//#define GPIO_NUM     5                        /* Number of user LEDs          */

//extern void GPIO_Init(void);
//extern void GPIO_On  (unsigned int num);
//extern void GPIO_Off (unsigned int num);
//extern void GPIO_Out (unsigned int value);

#endif
