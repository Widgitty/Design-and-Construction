#ifndef __CALIBRATION_H
#define __CALIBRATION_H

#define NUM_MODES 3

// Structure definitions

typedef struct {
	double lowerPointIn;
	double lowerPointOut;
	double upperPointIn;
	double upperPointOut;
} calibMeasurementTypeDef;

typedef struct {
	calibMeasurementTypeDef voltage;
	calibMeasurementTypeDef current;
	calibMeasurementTypeDef resistance;
} calibStoreTypeDef;



// Calibration structures
// Only one calibration tructure per mode, as calibration is linear over ranges
typedef struct {
	double zeroOffset;
	double multiplier;
} calibAdjustTypeDef;


typedef struct {
	calibAdjustTypeDef voltage;
	calibAdjustTypeDef current;
	calibAdjustTypeDef resistance;
} calibStructTypeDef;


extern calibStructTypeDef Get_Calibration(void); // TODO: remove this?

// Sort throught these
extern void Calibration_Init(calibAdjustTypeDef *calib_Data); // TODO: remove this?
extern double Calib_Conv_Test(int mode, double value, int *rangep, calibAdjustTypeDef *calib_Data);
extern void Calibrate(int mode, int range, calibAdjustTypeDef *calib_Data);
extern void Read_Calibration(calibMeasurementTypeDef *calib_Points);
extern void Test_Calibration(calibAdjustTypeDef *calib_Data);
extern void Set_Calibration_Flag(void);
extern void Check_Calibration_Flag(int mode, int range, calibAdjustTypeDef *calib_Data);

#endif
