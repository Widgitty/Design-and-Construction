#include "lcd_driver.h"

BUFFER lcd_commandBuffer;

void lcd_clear_display(void){
	lcd_write(LCD_WRITE_COMMAND, 0x01, 0); /*Write command clear display*/
	lcd_write(LCD_WRITE_COMMAND, 0x80, 2); /*add pause since clear is a long command*/
	lcd_move_cursor(0,0);
	
}

void lcd_move_cursor(uint16_t line, uint16_t offset){
	/*Moves cursor to specified position. Upper line = 0, first char = 0*/
	lcd_write(LCD_WRITE_COMMAND, (0x80 | ((line&1)<<6) | (offset & 0x1F)), 0);
}

void lcd_change_settings(int power, int cursor, int blink) {
	/*Can turn the LCD on or off, toggle the cursor's existance and blinkiness*/
	lcd_write(LCD_WRITE_COMMAND, (0x08 | power << 2 | cursor << 1 | blink), 0);
}

void lcd_init(int lines, int cursor, int blink, int buffer_size) {
	/* Sets up the LCD */
	
	/*Setup STM32F4 registers */
	lcd_setup_GPIO();
	lcd_setup_timer();
	
	/*Setup command buffer*/
	buffer_init(&lcd_commandBuffer, buffer_size);

	/*Follow initialisation protocols*/
	/* Wait for more than 40ms after Vcc rises to 2.7V */
	lcd_write(LCD_WRITE_COMMAND, 0x38, 41); /*Send command*/
	/*Wait more than 4.1ms */
	lcd_write(LCD_WRITE_COMMAND, 0x38,5);
	/*wait more than 100us*/
	lcd_write(LCD_WRITE_COMMAND, (0x30 | lines<<3), 1);
	
	lcd_write(LCD_WRITE_COMMAND, (0x30 | lines<<3), 0); /*Initialise properly (mode according to parameters passed)*/
	lcd_write(LCD_WRITE_COMMAND, 0x01, 0); /*Clear display*/
	lcd_write(LCD_WRITE_COMMAND, (0x0C | (cursor << 1) | blink), 1); /*Turn on display */
	lcd_write(LCD_WRITE_COMMAND, 0x06, 0); /*Set entry mode with "increment counter"*/
}

void lcd_write_string(char *string, int line, int offset) {
	/* Splits a string into 
	 * individual chars and
	 * puts them on the buffer */
	
	int done = 0;
	int place = 0;
	
	/*go to place x chars in on row y*/
	lcd_move_cursor(line, offset);
	/*Loop through string and add to buffer until reach termination char*/
	while (!done) {
		if (string[place] == '\0') {
			done = 1;
		} else {
			lcd_write(LCD_WRITE_DATA, string[place], 0);
		}
		place++;
	}
}

void lcd_write(int cmd, uint16_t byte, uint16_t time_delay) {
	/*Appends data to command buffer according to this protocol
	 * 32 bit word
	 * bits 0-7: data to put on the bus to the screen
	 * bits 8-15: command bits:
	 * 		0x----00-- = RS line low (screen will interpret as data)
	 *		0x----80-- = RS line high (screen will interpres as command)
	 *		0x----40-- = Scrolling on
	 *		0x----20-- = Scrolling off
	 * bits 16-30: timer bits. Delays data transfer by this many milliseconds.
	 * bit 31: unused
	*/
	
	int buffer_empty_var;
	
	buffer_empty_var = buffer_empty(&lcd_commandBuffer);
	buffer_put(&lcd_commandBuffer, (uint32_t) (byte | cmd | ((time_delay)<<16)));  /*Appends command word to buffer */
	
	/*Restarts the timer if there were no commands on the buffer */
	if (buffer_empty_var || buffer_empty(&lcd_commandBuffer)) {
		TIM6->ARR = _LCD_SHORT_AMOUNT_OF_TIME;
		TIM6->CR1 |= TIM_CR1_CEN;
	}
}

void lcd_setup_GPIO(void) {
	/*enable GPIO clocks for relevant ports */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
	
	/*setup GPIO ports D (Screen data pins) as outputs*/
	GPIOD->MODER	|= 0x00005555;
	
	/*Set to pull-down for when used as read pins*/
	GPIOD->PUPDR &= 0xFFFF0000;
	GPIOD->PUPDR |= 0x0000AAAA;
	
	/*Enable logic shifter on pin 15 of port A*/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER &= 0x3FFFFFFF;
	GPIOA->MODER |= 0x40000000;
	GPIOA->ODR |= 0x8000;
	
	/*Enable correct pins on GPIO port B (0, 1 and 2)*/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->MODER &= 0xFFFFFFC0;
	GPIOB->MODER |= 0x00000015;
	
}

void lcd_scrolling(int on, int ms) {
	/*adds commands to the buffer stopping or starting scrolling */
	if (on) {
		lcd_write(LCD_WRITE_COMMAND, _LCD_SCROLLING_ON_COMMAND, 0);
		lcd_write(LCD_WRITE_COMMAND, _LCD_SCROLLING_COMMAND_DATA_WORD, ms);
	} else {
		lcd_write(LCD_WRITE_COMMAND, _LCD_SCROLLING_OFF_COMMAND, 0);
	}
}

void lcd_setup_timer(void) {
	/*Setup timer 6 for most of the timed interrupt work*/
 	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN; /*Enable clock*/
 	TIM6->PSC = 10; /*Prescaler to zero for smallest time resolution*/
	TIM6->DIER |= TIM_DIER_UIE; /*Enable interrupts */
	NVIC_EnableIRQ(TIM6_DAC_IRQn); /*Register interrupt*/
}

void TIM6_DAC_IRQHandler(void) {
	/*Handles the timing of the normal data transfer */
	
	static int stage = 0;
	static uint32_t buffer_data;
	static uint8_t scrolling;
	
	/*Resets the interrupt flag*/
	TIM6->SR &= ~TIM_SR_UIF;
	
	switch(stage) {
		case 0:
			/*Get the new data*/
			buffer_data = buffer_get(&lcd_commandBuffer);
			
			/*Toggles scrolling if command is there and skips the rest of the stages*/
			if ((buffer_data & _LCD_SCROLLING_ON_COMMAND)) {
				scrolling = 1;
				stage = 3;
			} else if (buffer_data & _LCD_SCROLLING_OFF_COMMAND) {
				scrolling = 0;
				stage = 3;
			} else {
				
				/* adds the move command again if in scrolling mode */
				if ((buffer_data & 0x000000FF) == _LCD_SCROLLING_COMMAND_DATA_WORD){
					if (scrolling)
						lcd_write(LCD_WRITE_COMMAND, _LCD_SCROLLING_COMMAND_DATA_WORD, buffer_data>>16);
					else 
						stage = 3;
				}
				
				/*if buffer_data is zero, it's a null command generated by the empty buffer returning zero. We can safely ignore it.*/
				if (!buffer_data)
					stage = 3;
				
				/*Wait for time determined in command*/
				if (buffer_data>>16) {
					TIM6->PSC = 41000; /*Prescaler to tick every half millisecond*/
					TIM6->ARR = (buffer_data>>16) << 1; /*convert the time given in the command 
																									word to a suitable number which corresponds to ms*/
					TIM6->EGR = TIM_EGR_UG; /* Force register update in timer */
					TIM6->SR &= ~TIM_SR_UIF; /*Reset interrupt (gets set by prev. line)*/
				}
			}
			
			stage++; /*goes to next step */
			break;
			
		case 1:
			buffer_data &= 0x0000FFFF;
			if (buffer_data>>15) {
				/*Set the RS line to "write command" */
				GPIOB->ODR &= ~_LCD_RS_PIN;
			} else {
				/*Set the RS line to "write data" */
				GPIOB->ODR |= _LCD_RS_PIN;
			}
			
			/*Set the R/!W line to write (0) */
			GPIOB->ODR &= ~_LCD_RW_PIN;
			
			/*reset counter and set to interrupt after a suitable amount of time.*/
			TIM6->PSC = 0;
			TIM6->CNT = 0;
			TIM14->EGR = TIM_EGR_UG;
			TIM6->SR &= ~TIM_SR_UIF;
			TIM6->ARR = _LCD_STAGE_1_TIME;
			
			/*increment stage*/
			stage++;
			
			break;
			
		case 2:
			/*Bring the Enable line high*/
			GPIOB->ODR |= _LCD_E_PIN;
			
			/*Set the data on the bus */
			GPIOD->ODR &= 0xFF00;
			GPIOD->ODR |= 0x00FF & buffer_data;
		
			/*reset counter and set to interrupt after a suitable amount of time.*/
			TIM6->CNT = 0;
			TIM6->ARR = _LCD_STAGE_2_TIME;
			
			/*increment stage*/
			stage++;
			
			break;
		
		case 3:
			/*Take E line low again */
			GPIOB->ODR &= ~_LCD_E_PIN;
		
			/*reset counter and get interrupt in 7000*6ns=42us.*/
			TIM6->CNT = 0;
			TIM6->ARR = _LCD_STAGE_3_TIME;
			
			/*increment stage*/
			stage++;
			
			break;
		
		case 4:
			/*restarts data transfer */
			stage = 0;
		
			/*If the buffer is empty, stops timer. Nothing for the ISR to do any more */
			/*Otherwise, makes the timer execute almost immediately */
			if (buffer_empty(&lcd_commandBuffer)) {
				TIM6->CR1 &= ~TIM_CR1_CEN;
			} else {
				TIM6->ARR = _LCD_SHORT_AMOUNT_OF_TIME; /*send next char in a short amount of time */
			}
			break;
	}
}
