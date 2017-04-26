#include "Serial.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_uart.h"
#include "LED.h"
#include "lcd_driver.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "String.h"

// replace Delay with osDelay for compatibility with RTOS
#define Delay osDelay



//=====================================================//
//================== Global defines ===================//
//=====================================================//

UART_HandleTypeDef UART_Handle;
DMA_HandleTypeDef DMA_Rx_Handle;
DMA_HandleTypeDef DMA_Tx_Handle;
uint8_t rxBuffer = '\000';
uint8_t dumpBuffer[100];
uint8_t rxString[17]; // where we build our string from characters coming in
int rxindex = 0; // index for going though rxString

int errorFlag = 0;

char rx_Out[17];

int Serial_Mode_Int = -1;

int rxState = 0;

void (*RXHandlerCallback) (uint8_t);
int RXHandlerRegistered = 0;

int serial_Busy = 0;

uint8_t pData_Local[100];


//=====================================================//
//================ Interface functions ================//
//=====================================================//
void Error(int err) {
	int i;
	char string[17];
	sprintf(string, "ERROR: %d", err);
	lcd_clear_display();
	lcd_write_string(string, 0, 0);
	for (i = 0; i<10; i++) {
		LED_Out(0xFF);
		Delay(100);
		LED_Out(err);
		Delay(100);
	}
	lcd_clear_display();
}

int Check_For_Serial() {
//	int state = 0;
//	// Enable clock
//	__HAL_RCC_GPIOA_CLK_ENABLE();	
//	
//	// Initialise GPIOs
//	GPIO_InitTypeDef GPIO_InitStructure;
//	GPIO_InitStructure.Pin = GPIO_PIN_3;
//	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
//	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
//	GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
//	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	state = HAL_GPIO_ReadPin(GPIOA, 3);
//	
//	// TODO: deinit GPIO?
	
	return 1;
}

void Register_RX_Handler(void (*RXHandlerLocal) (uint8_t)) {
	RXHandlerCallback = RXHandlerLocal;
	RXHandlerRegistered = 1;
}

void Deregister_RX_Handler() {
	RXHandlerCallback = NULL;
	RXHandlerRegistered = 0;
}


void SerialInit() {
	HAL_StatusTypeDef Ret;
	
	sprintf(rx_Out, "");
	sprintf((char*)rxString, "");
	
	// UART handle and configguration
	//UART_HandleTypeDef UART_Handle; // Made global for now
	UART_Handle.Instance = USART2;
  UART_Handle.Init.BaudRate = 115200;
  UART_Handle.Init.WordLength = UART_WORDLENGTH_8B;
  UART_Handle.Init.StopBits = UART_STOPBITS_1;
  UART_Handle.Init.Parity = UART_PARITY_NONE;
  UART_Handle.Init.Mode = UART_MODE_TX_RX;
  UART_Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  UART_Handle.Init.OverSampling = UART_OVERSAMPLING_16;

	Ret = HAL_UART_Init(&UART_Handle);
	if (Ret != HAL_OK) {
		//TODO: handle this
	}
	SerialReceiveStart();
	//SerialReceiveDump();
	Delay(100);

	rxState = 0;
}


// Blocking send for now, could be better implemented in the future
Serial_StatusTypeDef Serial_Send(uint8_t *pData, uint16_t Size) {
	if (serial_Busy == 1) {
		return SERIAL_BUSY;
	}
	else {
		serial_Busy = 1;
		
		int i;
		for (i=0; i<Size; i++) {
			pData_Local[i] = pData[i];
		}
		//memcpy (pData_Local, pData, Size);
		HAL_StatusTypeDef Ret;
		Ret = HAL_UART_Transmit_DMA(&UART_Handle, pData_Local, Size);
		// Block for now
		/*
		while (serial_Busy == 1) {
			Delay(100);
		}
		*/
		if (Ret == HAL_OK)
			return SERIAL_OK;
		else
			return SERIAL_ERROR;
	}
}


void SerialReceiveStart() {
	// Start DMA recieve
	__HAL_UART_FLUSH_DRREGISTER(&UART_Handle);
	//__HAL_DMA_ENABLE(&DMA_Rx_Handle);
	rxState = 0;
	HAL_UART_Receive_DMA(&UART_Handle, &rxBuffer, 1);
}




void Serial_Receive() {
	if (strcmp(rx_Out, "") != 0) {
		char string[17];
		sprintf(string, "Serial:");
		lcd_clear_display();
		lcd_write_string(string, 0, 0);
		sprintf(string, "%s", rx_Out);
		lcd_write_string(string, 1, 0);
		Delay(5000);
		lcd_clear_display();
		sprintf(rx_Out ,"");
	}
}


void Serial_Check_Mode(int *mode) {
	if (Serial_Mode_Int != -1) {
		*mode = Serial_Mode_Int;
		Serial_Mode_Int = -1;
	}
}



//=====================================================//
//= Redefine HAL function for backwards compatibility =//
//=====================================================//
// This function has been copied as-is from the
// HAL V1.6.0 UART library.

HAL_StatusTypeDef Custom_HAL_UART_AbortTransmit(UART_HandleTypeDef *huart)
{
  /* Disable TXEIE and TCIE interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_TXEIE | USART_CR1_TCIE));

  /* Disable the UART DMA Tx request if enabled */
  if(HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAT))
  {
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAT);

    /* Abort the UART DMA Tx channel : use blocking DMA Abort API (no callback) */
    if(huart->hdmatx != NULL)
    {
      /* Set the UART DMA Abort callback to Null. 
         No call back execution at end of DMA abort procedure */
      huart->hdmatx->XferAbortCallback = NULL;

      HAL_DMA_Abort(huart->hdmatx);
    }
  }

  /* Reset Tx transfer counter */
  huart->TxXferCount = 0x00U;

  /* Restore huart->gState to Ready */
  huart->gState = HAL_UART_STATE_READY;

  return HAL_OK;
}



//=====================================================//
//===================== Callbacks =====================//
//=====================================================//

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	serial_Busy = 0;
	__HAL_UART_FLUSH_DRREGISTER(&UART_Handle); // Clear the buffer to prevent overrun
	//HAL_UART_DMAStop(huart);
	Custom_HAL_UART_AbortTransmit(huart);
}


int pos = 0;
	
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  __HAL_UART_FLUSH_DRREGISTER(&UART_Handle); // Clear the buffer to prevent overrun
	
	if (RXHandlerRegistered == 1) {
		RXHandlerCallback(rxBuffer);
	}
	else {
		
		if (rxState == 0) { // Check what to expect
			if (rxBuffer == 's') {
				rxState = 1;
			}
			if (rxBuffer == 'm') {
				rxState = 2;
			} 
		}
		
		else if (rxState == 1) { // expect string
		
			int i = 0;

			if (rxBuffer == '\n' || rxBuffer == '\r') // If Enter
			{
				rxString[rxindex] = 0;
				sprintf(rx_Out, "%s", rxString);
				rxindex = 0;
				for (i = 0; i < 17; i++) rxString[i] = 0; // Clear the string buffer
				
				// String complete, go back to check
				rxState = 0;
			}

			else
			{
					rxString[rxindex] = rxBuffer; // Add that character to the string
					rxindex++;
					if (rxindex >= 17) // User typing too much, we can't have commands that big
					{
							rxindex = 0;
							for (i = 0; i < 17; i++) rxString[i] = 0; // Clear the string buffer
							rxState = 0;
							sprintf(rx_Out, "String too long");
					}
			}
		}
		
		else if (rxState == 2) { // Expect mode
			if (((rxBuffer - '0') < 3) & (rxBuffer >= '0')){ // valid mode
				Serial_Mode_Int = (int) rxBuffer - '0';
			}
			// RX complete, go back to check
			rxState = 0;
		}
		
		else if (rxState == 3) { // Check response to AT command
			rxState = 0;
		}

	}
	
}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	errorFlag = 1;
	char string[17];
	Error(7);
	sprintf(string, "Serial Fail");
	lcd_clear_display();
	lcd_write_string(string, 0,0);
	Delay(5000);
	lcd_clear_display();
	serial_Busy = 0;
}


void DMA1_Stream6_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(DMA1_Stream6_IRQn);
	HAL_DMA_IRQHandler(&DMA_Tx_Handle);
}


void DMA1_Stream5_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(DMA1_Stream5_IRQn);
	HAL_DMA_IRQHandler(&DMA_Rx_Handle);
}




//=====================================================//
//= Initialisation callback and supporting functions  =//
//=====================================================//


void SetupDMA(UART_HandleTypeDef *huart) {

	uint32_t priorityGroup;
	uint32_t priority;
	
	__DMA1_CLK_ENABLE();
	
	// RX

	DMA_Rx_Handle.Instance = DMA1_Stream5;
	DMA_Rx_Handle.Init.Channel = DMA_CHANNEL_4;
	DMA_Rx_Handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
	DMA_Rx_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
	DMA_Rx_Handle.Init.MemInc = DMA_MINC_ENABLE;
	DMA_Rx_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	DMA_Rx_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	DMA_Rx_Handle.Init.Mode = DMA_CIRCULAR;
	DMA_Rx_Handle.Init.Priority = DMA_PRIORITY_LOW;
	DMA_Rx_Handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	HAL_DMA_Init(&DMA_Rx_Handle);
	//__HAL_DMA_DISABLE(&DMA_Rx_Handle);

	__HAL_LINKDMA(huart, hdmarx, DMA_Rx_Handle);

	//NVIC_SetPriorityGrouping(5);
	priorityGroup =  NVIC_GetPriorityGrouping();  
	priority = NVIC_EncodePriority(priorityGroup, 6, 0);
	NVIC_SetPriority(DMA1_Stream5_IRQn, priority);
	NVIC_EnableIRQ(DMA1_Stream5_IRQn);

	
	// TX

	DMA_Tx_Handle.Instance = DMA1_Stream6;
	DMA_Tx_Handle.Init.Channel = DMA_CHANNEL_4;
	DMA_Tx_Handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
	DMA_Tx_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
	DMA_Tx_Handle.Init.MemInc = DMA_MINC_ENABLE;
	DMA_Tx_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	DMA_Tx_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	DMA_Tx_Handle.Init.Mode = DMA_CIRCULAR;
	DMA_Tx_Handle.Init.Priority = DMA_PRIORITY_LOW;
	DMA_Tx_Handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	
	//DMA_Tx_Handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	//DMA_Tx_Handle.Init.MemBurst = DMA_MBURST_SINGLE;
	//DMA_Tx_Handle.Init.PeriphBurst = DMA_PBURST_SINGLE;
	
	HAL_DMA_Init(&DMA_Tx_Handle);

	__HAL_LINKDMA(huart, hdmatx, DMA_Tx_Handle);

	//NVIC_SetPriorityGrouping(5);
	priorityGroup =  NVIC_GetPriorityGrouping();  
	priority = NVIC_EncodePriority(priorityGroup, 6, 0);
	NVIC_SetPriority(DMA1_Stream6_IRQn, priority);
	NVIC_EnableIRQ(DMA1_Stream6_IRQn);
	
}


void SetupCallbacks() {

	uint32_t priorityGroup;
	uint32_t priority;
	
	//NVIC_SetPriorityGrouping(5);
	priorityGroup =  NVIC_GetPriorityGrouping();  
	priority = NVIC_EncodePriority(priorityGroup, 6, 0); // Group, Preempt, Sub
	NVIC_SetPriority(USART2_IRQn, priority);
	NVIC_EnableIRQ(USART2_IRQn);

}


// Callback from HAL
void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
	
	// Enable clocks
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__USART2_CLK_ENABLE();	
	
	// Initialise GPIOs
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
	GPIO_InitStructure.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//SetupCallbacks();
	SetupDMA(huart);	
}

