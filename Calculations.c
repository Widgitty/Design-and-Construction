#include <stdio.h>
#include <math.h>
#include "STM32F4xx.h"
#include "cmsis_os.h"
#include "LED.h"
#include "GPIO.h"
#include "stm32f4xx_hal_rcc.h"
#include "Calculations.h"
#include "Calibration.h"
#include "Defines.h"

#include "SWT.h"

#include "Timers.h"

#define resistance 10000;
#define capValue 0.000002;
#define M_PI 3.14159265358979323846


/* Utility class to perform mathematical operations such as ADC conversion */


// Define function prototypes
double currVoltCalc(double value, int *rangep, int mode);
//double adcConv(int mode, uint32_t value, int *rangep, calibAdjustTypeDef *calibData);
double capCalc(int *rangep);
double inductanceCalc(int *rangep);
void setTimerValue(int timer);
void switchVoltRes(int mode, double volt_value);
double scaling(double output, int *rangep);


double movAvg(double output, int mode, int *rangep);
void restartCounter(void);
double m;
double c;
	
 
double frequencyMeasure(int *rangep, int scale);
void countUp(void);
void setOutput(void);


TIM_HandleTypeDef timer_Instance_1 = { .Instance = TIM2};
TIM_HandleTypeDef timer_Instance_2 = { .Instance = TIM3};
TIM_HandleTypeDef timer_Instance_3 = { .Instance = TIM4};

calibAdjustTypeDef calib_Data[NUM_MODES];
 
TIM_HandleTypeDef timer_Instance_4 = { .Instance = TIM5};

int capacitorState = 0;
int inductanceState = 0;
int frequencyState = 0;
double counter = 0.0;
double freqOut = 0.0;
double prevOutput = 0.0;
uint32_t timerValueI = 0;
uint32_t timerValueIOld = 0;
double inductanceOld = 0.0;

double avgOut;
int deltaMode = 0;
double avgWaitTime = 0.0;

int inductanceBreak = 0;

double scaledValue = 0.0;
double scaleFactor = 1.0;
int muxMode = 0;






// Initialise the counter instance.
void restartCounter(void){
	__HAL_TIM_SET_COUNTER(&timer_Instance_2, 0);
	HAL_TIM_Base_Start(&timer_Instance_2);
}


/*void setMuxPins(double scaledValue){
	// calculate mux output pin stuff
	
	  scaleFactor = 10;
		GPIO_Off(1);
		GPIO_Off(2);
}*/
void setMuxPins(double scaledValue){
	// calculate mux output pin stuff
	/*
	switch (muxMode) {
		case 0:
			if((scaledValue <= 1000) && (scaledValue >= -1000)){
				muxMode = 1;
				scaleFactor = 1;
				GPIO_Off(1);
				GPIO_On(2);
			}
			break;
		
		case 1:
			if((scaledValue >= 1000) || (scaledValue <= -1000)){
				muxMode = 0;
				scaleFactor = 10;
				GPIO_Off(1);
				GPIO_Off(2);
			}
			else if((scaledValue <= 100) && (scaledValue >= -100)){
				muxMode = 2;
				scaleFactor = 0.1;
				GPIO_On(1);
				GPIO_Off(2);
			}
		
			break;
		
		case 2:
			if((scaledValue >= 100) || (scaledValue <= -100)){
				muxMode = 1;
				scaleFactor = 1;
				GPIO_Off(1);
				GPIO_On(2);
			}
			else if((scaledValue <= 10) && (scaledValue >= -10)){
				muxMode = 3;
				scaleFactor = 0.01;
				GPIO_On(1);
				GPIO_On(2);
			}
			break;
		
		case 3:
			if((scaledValue >= 10) || (scaledValue <= -10)){
				muxMode = 2;
				scaleFactor = 0.1;
				GPIO_On(1);
				GPIO_Off(2);
			}
			break;
		
		default:
			muxMode = 1;
			scaleFactor = 1;
			GPIO_Off(1);
			GPIO_On(2);
			break;
	}*/
	
	
	if((scaledValue >= 1000) || (scaledValue <= -1000)){
		scaleFactor = 10;
		GPIO_Off(1);
		GPIO_Off(2);
	}
	else if((scaledValue >= 100) || (scaledValue <= -100)){
		scaleFactor = 1;
		GPIO_Off(1);
		GPIO_On(2);
	}
	else if((scaledValue >= 10) || (scaledValue <= -10)){
		scaleFactor = 0.1;
		GPIO_On(1);
		GPIO_Off(2);
	}
	else{
		scaleFactor = 0.01;
		GPIO_On(1);
		GPIO_On(2);
	}
	
}

//on = +- 0.1
//off = +- 1

//1: 1.2
//0: 119m

//1: 119 ma
//0: 1.2A

//1: 24mA
//0: 246mA

//1:3mA
//0:3A

void setCurrentMuxPins(double scaledValue){
		switch (muxMode) {
		case 0:
			if((scaledValue <= 0.09) && (scaledValue >= -0.09)){
				muxMode = 1;
				scaleFactor = 0.133; // With fudge factor
				GPIO_On(0);
			}
			break;
		
		case 1:
			if((scaledValue >= 0.11) || (scaledValue <= -0.11)){
				muxMode = 0;
				scaleFactor = 0.9; // With fudge factor
				GPIO_Off(0);
			}
			break;
		
		default:
			muxMode = 0;
			scaleFactor = 1;
			GPIO_Off(0);
			break;
	}
}


//void setMuxPins(double scaledValue){
//	// calculate mux output pin stuff
//	
//	scaleFactor = 0.1;
//	GPIO_On(1);
//	GPIO_Off(2);

//}

double adcConv(int mode, uint32_t value, int *rangep, calibAdjustTypeDef *calibData){
	
	double output;	// MODE dependant digital output. 	
	
	m = calibData[mode].multiplier;
	c = calibData[mode].zeroOffset;
	
	double tempOut = (((double)value * m) + c);
	
	
	
	switch(mode)
	{
		// CURRENT MODE - input value ranges from [0 to 3], output ranges from [-1 to 1]
		case CURRMODE:
			*rangep = MILLI;
		
			tempOut = tempOut * scaleFactor;
			setCurrentMuxPins(tempOut);
		
			output = currVoltCalc(tempOut, rangep, mode);
			break;
		// VOLTAGE MODE input - value ranges from [0 to 3], output ranges from [-10 to 10]
		case VOLTMODE:
			
		
		
			*rangep = UNIT;	
			
			if(scaleFactor == 10.0)
				scaleFactor = 3.0;
			
			scaledValue = tempOut * 100 * scaleFactor;
			
			
			tempOut = tempOut * scaleFactor;
			setMuxPins(scaledValue);
			
		
			output = currVoltCalc(tempOut, rangep, mode);
		  break;
			case RMS:
			
		
		
			*rangep = UNIT;	
			
			if(scaleFactor == 10.0)
				scaleFactor = 3.0;
			
			scaledValue = tempOut * 100 * scaleFactor;
			
			
			tempOut = tempOut * scaleFactor;
			setMuxPins(scaledValue);
			
		
			output = currVoltCalc(tempOut, rangep, mode) * -1;
			GPIO_Off(1);
			GPIO_On(2);
		  break;
		// RESISTANCE MODE - Use ohms law to calculate R. 
		case RESMODE:
			*rangep = UNIT;
		
			scaledValue = tempOut * 0.01 * scaleFactor;
			tempOut = tempOut * scaleFactor;
			setMuxPins(scaledValue);
		
			output = scaling((tempOut /* INPUTCURRENT*/), rangep);	
			break;
		// CAPACITANCE MODE - Timer needs to be started.
		case CAPMODE:
			output = capCalc(rangep);
			break;
		// INDUCTANCE MODE
		case INDMODE:
			output = inductanceCalc(rangep);
			break;
		case FREQMODE:
			output = frequencyMeasure(rangep, 1);
			break;
		default:
			output = 0.0;
			break;
	}
	
	if(mode == RESMODE || mode == VOLTMODE){
		switch(1){
			case 0:
				break;
			case 1:
				break;
			case 2:
				break;
			default:
				break;
		}
	}
	
	
	return output;		
}

// Perform current and voltage range switching calculations.
double currVoltCalc(double output, int *rangep, int mode){
	//Configure range switching between UNIT volts and MILLIvolts.

	//Range switching to MILLI occurs when output is between +/- 1.
	if ((output > -1) && (output < 1)){
		if (*rangep != MILLI) {
				*rangep = MILLI;
		}

	}
	else {
	// If output is outside of +/- 1 convert back to UNIT volts.
		if (*rangep != UNIT) {
			*rangep = UNIT;
		}
	}
	
	//Resolution switching to low occurs when output is outside +/- 10.
	if (((output < -10) || (output > 10)) && mode == VOLTMODE && *rangep == UNIT){
		//Set range to lower resolution mode.
		*rangep = UNIT30;
	}
	//Resolution switching to high occurs when output is inside +/- 10.
	if ((output > -10) && (output < 10) && mode == VOLTMODE && *rangep == UNIT30){
		//Set range to lower resolution mode.
		*rangep = UNIT;
	}

	
	if(*rangep == MILLI){
	output *= 1000;	
	}
	
	if(*rangep == UNIT30){
		
	output *= 1; // was 3
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
			uint32_t clockFreq = HAL_RCC_GetPCLK1Freq();
			HAL_TIM_Base_Stop(&timer_Instance_4);
			__HAL_TIM_SET_COUNTER(&timer_Instance_4, 0);
			GPIO_Off(4);
			HAL_TIM_Base_Start(&timer_Instance_4);
			capacitorState = 1;
		}
		//discharge state
		else if(capacitorState == 1)
		{
			// simple polling for this one, as the discharge phase it not as important to time:
			// should still take around 3 seconds to fully discharge
			// 40000 are 4 seconds in this case
			timerValue = __HAL_TIM_GET_COUNTER(&timer_Instance_4);
			if(timerValue > 20000) 
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
			
			HAL_TIM_Base_Stop(&timer_Instance_4);
			__HAL_TIM_SET_COUNTER(&timer_Instance_4, 0);
			GPIO_On(4);
			HAL_TIM_Base_Start(&timer_Instance_4);
			capacitorState = 3;
			output = 0;
		}
		// read state: timer will be read and when the exernal interrupt stops the timer the value will settle.
		else if(capacitorState == 3){
			// just reads the timervalue, the timer will be stopped by the interrupt
			// interrupt is handled in 
			timerValue = __HAL_TIM_GET_COUNTER(&timer_Instance_4);
			//the timer counts to 10000 every second (reason for the conversion, to get actual seconds)
			timeSeconds = timerValue * 0.0001;
			// formula for capacitance
			output = timeSeconds / resistance;

		}
		
		
		
		
		return scaling(output, rangep);
}

// scaling function can take any input and will scale the value to a readable form
double scaling(double output, int *rangep){
	if((output <= 0.000001)&& (output >= -0.000001)){
		*rangep = NANO;
		output = output * 1000000000;
	}	
	else if((output <= 0.001) && (output >= -0.001))
	{
		*rangep = MICRO;
		output = output * 1000000;
	}
	else if((output <= 1) && (output >= -1))
	{
		*rangep = MILLI;
		output = output * 1000;
	}
	else if((output >= 1000000) || (output <= -1000000))
	{
		*rangep = MEGA;
		output = output/1000000;
	}
	else if((output >= 1000) || (output <= -1000))
	{
		*rangep = KILO;
		output = output/1000;
	}

	else
	{
		output = output;
		*rangep = UNIT;
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
		
		//this timer measures the difference of the different intervals
		HAL_TIM_Base_Start(&timer_Instance_1);
		HAL_TIM_Base_Start_IT(&timer_Instance_1);
		
		// this is the pulse generating timer
		HAL_TIM_Base_Start(&timer_Instance_3);
		HAL_TIM_Base_Start_IT(&timer_Instance_3);
		inductanceState = 1;
		
		
	}
	else if(inductanceState == 1){
		
		NVIC_EnableIRQ(EXTI4_IRQn);
		//calculates difference of two last timers
		if(timerValueI > timerValueIOld){	
			output = timerValueI - timerValueIOld;
		}
		else
		{
			output = 50000 + timerValueI - timerValueIOld;
		}
		
		
		if(output < 10000)
		{
			// use updated version
			// average it out
			prevOutput = (output + prevOutput)/2;
			output = prevOutput;
		}
		else
		{
			// indicates a long break, don't use updated version
			// can also store there was a break, to count multiple steps later.
			inductanceBreak = 1;
			output = prevOutput;
		}
		
		// gets frequency
		// timer counts to 50000 every 0.05 seconds so that is equivalent of 
		// counting to 1000000 every second.
		output = 1000000.0/output;
		
		if(output != 0){
			output = 1/(output*output*output_Modifier);
		}
		else
		{
			output = 0;
		}
		
	}
	
	return scaling(output, rangep);
}
void countUp(void){
	counter++;
}
void setOutput(void){
	freqOut = counter/2;
	counter = 0;
}
double frequencyMeasure(int *rangep, int scale){
	
	if(frequencyState == 0){
		HAL_TIM_Base_Stop(&timer_Instance_1);
		__HAL_TIM_SET_COUNTER(&timer_Instance_1, 0);
		HAL_TIM_Base_Start(&timer_Instance_1);
		frequencyState = 1;
	}
	else if(frequencyState == 1){		
		if(scale == 1){
			return scaling(freqOut, rangep);
		}
		else
		{
			return freqOut;
		}
	}
	
	return freqOut;

}

double movAvg(double avgIn, int mode, int *rangep){

	double aCoeff = 0.9999999;
	double bCoeff = 0.0000001;
		
		// CURRENT MODE - averaging conditions for the MILLIamps range.
		if(mode == CURRMODE && (avgIn > avgOut + 10 || avgIn < avgOut - 10)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
		// VOLTAGE MODE - averaging conditions for the volts range.
		else if(mode == VOLTMODE && *rangep == UNIT && (avgIn > avgOut + 0.1 || avgIn < avgOut - 0.1)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
		// VOLTAGE MODE - averaging conditions for the 30 volts range.
		else if(mode == VOLTMODE && *rangep == UNIT30 && (avgIn > avgOut + 1 || avgIn < avgOut - 1)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
		// VOLTAGE MODE - averaging conditions for the MILLIvolts range.
		else if(mode == VOLTMODE && *rangep == MILLI && (avgIn > avgOut + 50 || avgIn < avgOut - 50)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
	
		// RESISTANCE MODE - averaging conditions for the ohms range.
		else if(mode == RESMODE && *rangep == UNIT && (avgIn > avgOut + 1 || avgIn < avgOut - 1)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
		// RESISTANCE MODE - averaging conditions for the KILOhms range.
		else if(mode == RESMODE && *rangep == KILO && (avgIn > avgOut + 10 || avgIn < avgOut - 10)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
		// RESISTANCE MODE - averaging conditions for the MEGAohms range.
		else if(mode == RESMODE && *rangep == MEGA && (avgIn > avgOut + 100 || avgIn < avgOut - 100)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
		else if ((mode != CURRMODE) && (mode != VOLTMODE) && (mode != RESMODE)){
			avgOut = avgIn;
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
