#ifndef __CALIBRATION_H
#define __CALIBRATION_H

// Structure definitions
typedef struct {
	double point1;
	double point2;
} testStructTpeDef;



typedef struct {
	double lowerPointIn;
	double lowerPointOut;
	double upperPointIn;
	double upperPointOut;
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

extern void Calibration_Init(void); // TODO: remove this?
extern double Calib_Conv_Test(int mode, double value, int *rangep);
extern void Calibrate(int mode, int range);
extern calibStoreTypeDef Read_Calibration(void);
extern void Test_Calibration(void);
extern void Set_Calibration_Flag(void);
extern void Check_Calibration_Flag(int mode, int range);

#endif
