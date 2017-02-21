#include <stdio.h>
#include <math.h>
#include "STM32F4xx.h"
#include "cmsis_os.h"
#include "LED.h"

/* Utility class to perform mathematical operations such as ADC conversion */


// Define function prototypes
double switchRange(int value, int *rangep, double scale);
double adcConv(int mode, double value, int *rangep);



double adcConv(int mode, double value, int *rangep){

	double output;	// MODE dependant digital output. 
	double inputCurrent = 3*pow(10,-6); // 3 microamp system input current - constant defined by hardware team

	
	// CURRENT MODE - input value ranges from [0 to 3], output ranges from [-1 to 1]
	if (mode ==	0){ 
			LED_Out(1);
			output = switchRange(value, rangep, 0.66);
	}
	
	// VOLTAGE MODE input - value ranges from [0 to 3], output ranges from [-10 to 10]
	if (mode ==	1){ 
		LED_Out(2);
		output = switchRange(value, rangep, 6.66);
	} 
	
	// RESISTANCE MODE - Use ohms law to calculate R. 
	if (mode ==	2){ 
		LED_Out(4);
		output = switchRange(value, rangep, 6.66) / inputCurrent;		
	} 	

	return output;		
	
}

double switchRange(int value, int *rangep, double scale){
	
	int maxRange = 1;
	int minRange = 0;
	double output;
	
	if(*rangep == 0){
		output = (((double)value / (pow(2.0, 16.0)) * 3.3)-1.5) * scale;	
		}
	if(*rangep == 1){
		output = (((double)value / (pow(2.0, 16.0)) * 3.3)-1.5) * scale*0.1000;	
		}
	
	if ((output > -0.1) && (output < 0.1)){
 
		if (*rangep < maxRange) {
				*rangep = 1;
		}
		else {
				// TODO: Print error to LCD
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
	
	return output;
}


