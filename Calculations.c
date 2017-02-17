#include <stdio.h>
#include <math.h>
#include "STM32F4xx.h"
#include "cmsis_os.h"

/* Utility class to perform mathematical operations such as ADC conversion */

double adcConv(int mode, double value, int *rangep){
	
	int range = *rangep;
	double output;	// MODE dependant digital output. 
	double inputCurrent = 3*pow(10,-6); // 3 microamp system input current - constant defined by hardware team
	int maxRange = 1;
	int minRange = 0;
	
	// CURRENT MODE - input value ranges from [0 to 3], output ranges from [-1 to 1]
	if (mode ==	0){ 
		if(range == 0){
		output = (((double)value / (pow(2.0, 16.0)) * 3.3)-1.5) / 1.5;	
		}
		if(range == 1){
		output = (((double)value / (pow(2.0, 16.0)) * 3.3)-1.5) / 15;	
		}	
			
		if ((output > -0.1) & (output < 0.1)){
			if (range < maxRange) {
				range++;
			}
			else {
				// TODO: Print error to LCD
			}
		}
		else {
			if (range > minRange) {
				range--;
			}
			else {
				// TODO: Print error to LCD
			}
		}
	}
	
	// VOLTAGE MODE input - value ranges from [0 to 0], output ranges from [-10 to 10]
	if (mode ==	1){ 
		output = (((double)value / (pow(2.0, 16.0)) * 3.3)-1.5) * 6.66;	
	} 
	
	// RESISTANCE MODE - Use ohms law to calculate R. 
	if (mode ==	2){ 
		output = ((double)value / (pow(2.0, 16.0)) * 3.3) / inputCurrent;		
	} 	

	*rangep = range;
	return output;
	
			
	
}
