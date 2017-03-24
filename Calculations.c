#include <stdio.h>
#include <math.h>
#include "STM32F4xx.h"
#include "cmsis_os.h"
#include "LED.h"
#include "Timers.h"
#include "LCD_Thread.h"
#include "SWT.h"
#include "ADC.h"
#include "GPIO.h"
#include "stm32f4xx_hal_rcc.h"
#define resistance 10000;
#define Delay osDelay;
/* Utility class to perform mathematical operations such as ADC conversion */


// Define function prototypes
void restartCounter(void);
void calibrate(int mode, int *rangep);
double switchRange(int value, int *rangep, double scale);
double adcConv(int mode, int *rangep);
double movAvg(double output, int mode, int *rangep);
double capCalc(void);


// Define class variables
	double avgOut;
	double avgWaitTime = 0.0;
	int deltaMode = 0;
	char string[17];



TIM_HandleTypeDef timer_Instance_1 = { .Instance = TIM2};
TIM_HandleTypeDef timer_Instance_2 = { .Instance = TIM3};
int capacitorState = 0;


// Define function definitions.

// Initialise the counter instance.
void restartCounter(void){
	__HAL_TIM_SET_COUNTER(&timer_Instance_2, 0);
	HAL_TIM_Base_Start(&timer_Instance_2);
}
	
// Perform analog to digital conversion.
double adcConv(int mode, int *rangep){
	double value = read_ADC1()*16;
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
		output = ((double)value / (pow(2.0, 16.0)) * 3.0) / inputCurrent;		
		if (output < 1000){
			*rangep = 0;
		}	
		if (output >= 1000){
			*rangep = 1;
			output/=1000;
		}

	} 	

	// CAPACITANCE MODE - Timer needs to be started.
	if (mode == 3){
		output = capCalc();
	}
	return output;		
	
}

//Utililty function to perform scaled auto ranging. Used in adcConv().
double switchRange(int value, int *rangep, double scale){
	
	// Define the minimum and max number of ranges. 
	int maxRange = 1;
	int minRange = 0;
	double output;
	
	output = (((double)value / (pow(2.0, 16.0)) * 3.0)-1.5) * scale;	
	
	// Convert TO and FROM milli-units depending on the range.
	if ((output > -1) && (output < 1)){
		if (*rangep < maxRange) {
				*rangep = 1;
		}
	}
	else {
		if (*rangep > minRange) {
			*rangep = 0;
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

// Calculate a weighted moving average - slows down output display.
// Averaging begins once a certain noise threshold or timer value has been reached.
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
		if(mode == 2 && *rangep == 0 && (avgIn > avgOut + 1 || avgIn < avgOut - 1)){
			avgOut = avgIn;
			restartCounter();
				
		}
		
		// RESISTANCE MODE - averaging conditions for the kilohms range.
		if(mode == 2 && *rangep == 1 && (avgIn > avgOut + 100 || avgIn < avgOut - 100)){
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
	
};

void calibrate(int mode, int *rangep) {
	int cursor = 0;
	int pos = 0;
	double number = 10.000;
	char string[17];
	
	uint32_t btns = 0;
	LCD_Write_At("", 0, 0, 1);
	
	while ((btns = SWT_Debounce()) != 0x8000) {

		
		switch (btns) {
			case 0x0100:
				if (cursor > 0) {
					cursor --;
					pos --;
					if (cursor == 1)
						cursor --;
				}
			break;
			case 0x0200:
				if (cursor < 4) {
					cursor ++;
					pos ++;
					if (cursor == 1)
						cursor ++;
				}
			break;
			case 0x0400:
				number -= pow(10, (-pos));
			break;
			case 0x0800:
				number += pow(10, (-pos));
			break;
			default:
				//blah
			break;
		}
		
		sprintf(string, "%1.3lf", number);
		LCD_Write_At(string, 0, 0, 0);
		LCD_Write_At("     ", 0, 1, 0);
		LCD_Write_At("^", cursor, 1, 0); // TODO: Replace with proper cursor
		osDelay(100);
	}
	
	// Calculate error
	double error = number - avgOut;

	
	// Correct
	
	sprintf(string, "Calibrated at:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", number);
	LCD_Write_At(string, 0, 1, 0);
	osDelay(2000);
	LCD_Write_At("", 0, 0, 1);
	sprintf(string, "Adjusted:");
	LCD_Write_At(string, 0, 0, 1);
	sprintf(string, "%1.3lf", error);
	LCD_Write_At(string, 0, 1, 0);
	osDelay(2000);
	LCD_Write_At("", 0, 0, 1);
}



	



