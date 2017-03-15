#ifndef __CALIBRATION_H
#define __CALIBRATION_H

// Structure definitions
typedef struct {
	double point1;
	double point2;
} testStructTpeDef;



typedef struct {
	double lowerPoint;
	double upperPoint;
} calibMeasurementTpeDef;

typedef struct {
	calibMeasurementTpeDef voltage;
	calibMeasurementTpeDef current;
	calibMeasurementTpeDef resistance;
} calibStoreTypeDef;



// Calibration structures
// Only one calibration tructure per mode, as calibration is linear over ranges
typedef struct {
	double zeroOffset;
	double multiplier;
} calibAdjustTpeDef;


typedef struct {
	calibAdjustTpeDef voltage;
	calibAdjustTpeDef current;
	calibAdjustTpeDef resistance;
} calibStructTypeDef;


extern void Calibrate(int mode, int range);
extern calibStoreTypeDef Read_Calibration(void);
extern void Test_Calibration(void);

#endif
