#include <stdio.h>
#include <math.h>
#include "STM32F4xx.h"
#include "cmsis_os.h"
#include "LED.h"
#include "Timers.h"
#include "GPIO.h"
#include "stm32f4xx_hal_rcc.h"
#define resistance 10000;
/* Utility class to perform mathematical operations such as ADC conversion */


// Define function prototypes
double switchRange(int value, int *rangep, double scale);
double adcConv(int mode, double value, int *rangep);

double movAvg(double output, int mode);

// Define class variables
	double avgOut;
	int deltaMode = 0;
	double avgDiff;

double capCalc(void);

TIM_HandleTypeDef timer_Instance_1 = { .Instance = TIM2};
TIM_HandleTypeDef timer_Instance_2 = { .Instance = TIM3};
int capacitorState = 0;


// Define function definitions

double adcConv(int mode, double value, int *rangep){

	double output;	// MODE dependant digital output. 
	double inputCurrent = 3*pow(10,-6); // 3 microamp system input current - constant defined by hardware team

	
	// CURRENT MODE - input value ranges from [0 to 3], output ranges from [-1 to 1]
	if (mode ==	0){ 
			output = switchRange(value, rangep, 0.66);
	}
	
	// VOLTAGE MODE input - value ranges from [0 to 3], output ranges from [-10 to 10]
	if (mode ==	1){ 
		output = switchRange(value, rangep, 6.66);
	} 
	
	// RESISTANCE MODE - Use ohms law to calculate R. 
	if (mode ==	2){ 
		*rangep = 0;
		output = ((double)value / (pow(2.0, 16.0)) * 3.3) / inputCurrent;		
		if (output >= 1000 && output){
			*rangep = 1;
			output/=1000;
		}
		else *rangep = 0;
	} 	

	
	

	// CAPACITANCE MODE - Timer needs to be started.
	if (mode == 3){
		output = capCalc();
	}


	return output;		
	
}

double switchRange(int value, int *rangep, double scale){
	
	int maxRange = 1;
	int minRange = 0;
	double output;
	
	output = (((double)value / (pow(2.0, 16.0)) * 3.0)-1.5) * scale;	
	
	
	
	if ((output > -1) && (output < 1)){
		if (*rangep < maxRange) {
				*rangep = 1;
		}
		else {
				//*rangep = 0;
		}
	}
	else {
	
		if (*rangep > minRange) {
			*rangep = 0;
		}
		else {
				// TODO: Print error to LCD
		}
	}
	
	if(*rangep == 1){
	output *= 1000;	
	}
	
	return output;
}

double capCalc(){
	
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
		
		return output;
}


double movAvg(double avGin, int mode){
	double aCoeff = 0.9;
	double bCoeff = 0.1;
	avgDiff = avgOut - avGin;
	
	 if (mode != deltaMode){
			deltaMode = mode;
		avgOut = 0;
	 }
	 
	 if (mode == deltaMode){
		avgOut = (aCoeff*avgOut + bCoeff*avGin);
	 }
	 

		

	return avgOut;
	
};


	
	


