#include <stdio.h>
#include <math.h>
#include "STM32F4xx.h"
#include "cmsis_os.h"
#include "LED.h"
#include "Timers.h"
#include "GPIO.h"
#include "stm32f4xx_hal_rcc.h"
#include "Calculations.h"
#include "Defines.h"
#define resistance 10000;
#define capValue 0.000002;
#define M_PI 3.14159265358979323846
/* Utility class to perform mathematical operations such as ADC conversion */


// Define function prototypes
double currVoltCalc(int value, int *rangep, double scale);
double adcConv(int mode, double value, int *rangep);
double capCalc(int *rangep);
double inductanceCalc(int *rangep);
void setTimerValue(int timer);
double scaling(double output, int *rangep);
double frequencyMeasure(int *rangep);
double movAvg(double output, int mode, int *rangep);
void restartCounter(void);

TIM_HandleTypeDef timer_Instance_1 = { .Instance = TIM2};
TIM_HandleTypeDef timer_Instance_2 = { .Instance = TIM3};
TIM_HandleTypeDef timer_Instance_3 = { .Instance = TIM4};
int capacitorState = 0;
int inductanceState = 0;
int frequencyState = 0;
uint32_t timerValueI = 0;
uint32_t timerValueIOld = 0;
double inductanceOld = 0.0;
double avgOut;
int deltaMode = 0;
double avgWaitTime = 0.0;

// Initialise the counter instance.
void restartCounter(void){
	__HAL_TIM_SET_COUNTER(&timer_Instance_2, 0);
	HAL_TIM_Base_Start(&timer_Instance_2);
}


double adcConv(int mode, double value, int *rangep){

	double output;	// MODE dependant digital output. 
	double inputCurrent = 3*pow(10,-6); // 3 microamp system input current - constant defined by hardware team

	
	switch(mode)
	{
		// CURRENT MODE - input value ranges from [0 to 3], output ranges from [-1 to 1]
		case 0:
			output = currVoltCalc(value, rangep, currentScale);
			break;
		// VOLTAGE MODE input - value ranges from [0 to 3], output ranges from [-10 to 10]
		case 1:
			output = currVoltCalc(value, rangep, voltageScale);
			break;
		// RESISTANCE MODE - Use ohms law to calculate R. 
		case 2:
			*rangep = nothing;
			output = scaling((((double)value / (pow(2.0, 16.0)) * 3.3) / inputCurrent), rangep);	
			break;
		// CAPACITANCE MODE - Timer needs to be started.
		case 3:
			output = capCalc(rangep);
			break;
		// INDUCTANCE MODE
		case 4:
			output = inductanceCalc(rangep);
			break;
		case 5:
			output = frequencyMeasure(rangep);
			break;
		default:
			output = 0.0;
			break;
	}
	return output;		
}

// Perform current and voltage range switching calculations.
double currVoltCalc(int value, int *rangep, double scale){
	//Configure range switching between unit volts and millivolts.
	int maxRange = nothing;
	int minRange = milli;
	double output;
	
	output = (((double)value / (pow(2.0, 16.0)) * 3.3)-1.5) * scale;	
	
	//Range switching to milli occurs when output is between +/- 1 volt.
	if ((output > -1) && (output < 1)){
		if (*rangep > minRange) {
				*rangep = milli;
		}
		else {
				//*rangep = 0;
		}
	}
	else {
	// If output is out side of +/- 1 volt convert back to unit volts.
		if (*rangep < maxRange) {
			*rangep = nothing;
		}
		else {
				// TODO: Print error to LCD
		}
	}
	if(*rangep == milli){
	output *= 1000;	
	}
	
	return output;
}

double capCalc(int *rangep){
	
		double output = 0;
		int timerValue = 0;
		double timeSeconds = 0.0;
	// reset the timer to 0 and discharge phase:
		// gets timed to discharge for a bit, I will move this into interrupts.
		// a timeout will set the capacitorState back to 0 (discharge state)
		// start discharge state
		// I will probably move all this into a separate function and make the capacitorState less messy (global variable atm)
		if(capacitorState == 0){
			HAL_TIM_Base_Stop(&timer_Instance_1);
			__HAL_TIM_SET_COUNTER(&timer_Instance_1, 0);
			GPIO_Off(4);
			HAL_TIM_Base_Start(&timer_Instance_1);
			capacitorState = 1;
		}
		//discharge state
		else if(capacitorState == 1)
		{
			// simple polling for this one, as the discharge phase it not as important to time:
			// should still take around 3 seconds to fully discharge
			// 40000 are 4 seconds in this case
			timerValue = __HAL_TIM_GET_COUNTER(&timer_Instance_1);
			if(timerValue > 40000) 
			{
				capacitorState = 2;
			}
			output = 0;
		}
		//charge state
		else if(capacitorState == 2)
		{
			//resets the timer again and tells the cap to charge
			// it will change the capacitorState to a measuring state = 2
			
			HAL_TIM_Base_Stop(&timer_Instance_1);
			__HAL_TIM_SET_COUNTER(&timer_Instance_1, 0);
			GPIO_On(4);
			HAL_TIM_Base_Start(&timer_Instance_1);
			capacitorState = 3;
			output = 0;
		}
		// read state: timer will be read and when the exernal interrupt stops the timer the value will settle.
		else if(capacitorState == 3){
			// just reads the timervalue, the timer will be stopped by the interrupt
			// interrupt is handled in 
			timerValue = __HAL_TIM_GET_COUNTER(&timer_Instance_1);
			//the timer counts to 10000 every second (reason for the conversion, to get actual seconds)
			timeSeconds = timerValue * 0.0001;
			// formula for capacitance
			output = timeSeconds / resistance;
	
			
		}
		
		
		
		
		return scaling(output, rangep);
}
double scaling(double output, int *rangep){
	if(output < 0.000001){
		*rangep = nano;
		output = output * 1000000000;
	}	
	else if(output < 0.001)
	{
		*rangep = micro;
		output = output * 1000000;
	}
	else if(output < 1)
	{
		*rangep = milli;
		output = output * 1000;
	}
	else if(output > 1000)
	{
		*rangep = kilo;
		output = output/1000;
	}
	else if(output > 1000000)
	{
		*rangep = mega;
		output = output/1000000;
	}
	else
	{
		output = output;
		*rangep = nothing;
	}
	return output;
}

void setTimerValue(int timer){
	
	timerValueIOld = timerValueI;
	timerValueI = timer;
	
}
double inductanceCalc(int *rangep){
	
	double output = 0.0;
	double output_Modifier = 4*M_PI*M_PI * capValue;
	
	if(inductanceState == 0)
	{
		HAL_TIM_Base_Stop(&timer_Instance_1);
		__HAL_TIM_SET_COUNTER(&timer_Instance_1, 0);
		HAL_TIM_Base_Start(&timer_Instance_1);
		
		// this is the pulse generating timer
		HAL_TIM_Base_Start(&timer_Instance_3);
		HAL_TIM_Base_Start_IT(&timer_Instance_3);
		inductanceState = 1;
	}
	else if(inductanceState == 1){
		
		
		if(timerValueI > timerValueIOld){	
			output = timerValueI - timerValueIOld;
		}
		else
		{
			output = 50000 + timerValueI - timerValueIOld;
		}
		
		output = output*0.0001;
			
		if(output != 0){
			output = 1/output;
			output = 1/(output*output*output_Modifier);
			inductanceOld = output;
		}
		else
		{
			output = 0;
		}
		
	}
	
	return scaling(output, rangep);
}
double frequencyMeasure(int *rangep){
	
	
	double output = 0.0;
	if(frequencyState == 0)
	{
		HAL_TIM_Base_Stop(&timer_Instance_1);
		__HAL_TIM_SET_COUNTER(&timer_Instance_1, 0);
		HAL_TIM_Base_Start(&timer_Instance_1);
		frequencyState = 1;
	}
	
	else if(frequencyState == 1){
		if(timerValueI > timerValueIOld){	
			output = timerValueI - timerValueIOld;
		}
		
		else
		{
			output = 50000 + timerValueI - timerValueIOld;
		}
		output = output*0.0001;
		
		// output is difference between timer values at this point.
		
		output = 1/output;
		
		
	}
	return scaling(output, rangep);
}

double movAvg(double avgIn, int mode, int *rangep){

	double aCoeff = 0.9999999;
	double bCoeff = 0.0000001;
		
		// CURRENT MODE - averaging conditions for the milliamps range.
		if(mode == 0 && (avgIn > avgOut + 10 || avgIn < avgOut - 10)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
		// VOLTAGE MODE - averaging conditions for the volts range.
		if(mode == 1 && *rangep == 0 && (avgIn > avgOut + 1 || avgIn < avgOut - 1)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
		// VOLTAGE MODE - averaging conditions for the millivolts range.
		if(mode == 1 && *rangep == 1 && (avgIn > avgOut + 100 || avgIn < avgOut - 100)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
		// RESISTANCE MODE - averaging conditions for the ohms range.
		if(mode == 2 && *rangep == nothing && (avgIn > avgOut + 1 || avgIn < avgOut - 1)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
		// RESISTANCE MODE - averaging conditions for the kilohms range.
		if(mode == 2 && *rangep == kilo && (avgIn > avgOut + 100 || avgIn < avgOut - 100)){
			avgOut = avgIn;
			restartCounter();
				
		}
	 
		// Restart the counter if the mode is changed.
		if (mode != deltaMode){
			deltaMode = mode;

			restartCounter();
			avgOut = avgIn;	 
	 }
	 
	 // Perform the averaging calculation.
	 if (mode == deltaMode){
			// Convert value of timer instance to seconds.
			avgWaitTime = __HAL_TIM_GET_COUNTER(&timer_Instance_2)*0.0001;
			
		 
		 // Get output direct input if timer is less than 10ms.
		 if(avgWaitTime < 0.1){
				avgOut = avgIn;
			}	
			
			//Stop the timer and begin averaging as input begins fluctuating.
			if(avgOut > avgIn + 0.001 || avgOut < avgIn - 0.001){
				HAL_TIM_Base_Stop(&timer_Instance_2);
				avgOut = (aCoeff*avgOut + bCoeff*avgIn);
			}
	 }		

	return avgOut;
	
}
