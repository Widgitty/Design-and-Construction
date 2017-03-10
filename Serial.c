#include "Serial.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_uart.h"
#include "LED.h"
#include "LCD_Thread.h"
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

int doneFlag = 0;
int errorFlag = 0;

char rx_Out[17];

int mode_Int = 0;

int rxState = 0;

int WiFiEnabled = 0;

int ATResponse = 0;



//=====================================================//
//================ Interface functions ================//
//=====================================================//
void Error(int err) {
	int i;
	char string[17];
	sprintf(string, "ERROR: %d", err);
	LCD_Write_At(string, 0, 0, 1);
	for (i = 0; i<10; i++) {
		LED_Out(0xFF);
		Delay(100);
		LED_Out(err);
		Delay(100);
	}
}

void SendString(char* string) {
	
	doneFlag = 0;
	HAL_StatusTypeDef Ret = HAL_UART_Transmit_DMA(&UART_Handle, (uint8_t*)string, strlen(string));
	while (doneFlag == 0) {
		Delay(100);
	}
	Delay(100);
}

int WiFiInit() {
	rxState = 3; // Check for AT responses
	int i = 0;
	int ret = 1;
	
	for (i=0; i<3; i++) { 
		SendString("AT+RST\r\n");
		Delay(100);
		//check response
		if (ATResponse == 1) {
			WiFiEnabled = 1;
			ret = 0;
			break;
		}
	}
	ATResponse = 0;
	
	if (WiFiEnabled == 1) {
		
		SendString("AT+CWMODE=2\r\n");
		if (ATResponse != 1)
			ret = 1;
		ATResponse = 0;
		
		SendString("AT+CWSAP=\"MultiMeter!\",\"123\",1,0\r\n");
		if (ATResponse != 1)
			ret = 1;
		ATResponse = 0;
		
		SendString("AT+CIPMUX=1\r\n");
		if (ATResponse != 1)
			ret = 1;
		ATResponse = 0;
				
		SendString("AT+CWDHCP=0,0\r\n");
		if (ATResponse != 1)
			ret = 1;
		ATResponse = 0;
		
		SendString("AT+CIPAP=\"192.168.1.1\"\r\n");
		if (ATResponse != 1)
			ret = 1;
		ATResponse = 0;
		
		SendString("AT+CIPSERVER=1,1138\r\n");
		if (ATResponse != 1)
			ret = 1;
		ATResponse = 0;
	}
	
	return(ret);
}


void SerialReceiveDump() {
	// Start DMA recieve
	__HAL_UART_FLUSH_DRREGISTER(&UART_Handle);
	//__HAL_DMA_ENABLE(&DMA_Rx_Handle);
	rxState = 10;
	HAL_UART_Receive_DMA(&UART_Handle, dumpBuffer, 100);
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
	if (WiFiInit() != 0) {
		sprintf(rx_Out, "WiFi error");
		WiFiEnabled = 0;
		rxState = 0;
	}
	else {
		rxState = 4;
	}
}


// Blocking send for now, could be better implemented in the future
void SerialSend(uint8_t *pData, uint16_t Size, uint32_t Timeout) {
	HAL_StatusTypeDef Ret;
	
	// If in WiFI mode, send the data AT command
	if (WiFiEnabled == 1) {
		char string[17];
		sprintf(string, "AT+CIPSEND=0,%d\r\n", Size);
		SendString(string);
		doneFlag = 0;
		Ret = HAL_UART_Transmit_DMA(&UART_Handle, pData, Size);
		while (doneFlag == 0) {
			Delay(100);
		}
		sprintf(string, "AT+CIPSEND=1,%d\r\n", Size);
		SendString(string);
		doneFlag = 0;
		Ret = HAL_UART_Transmit_DMA(&UART_Handle, pData, Size);
		while (doneFlag == 0) {
			Delay(100);
		}
	}
	else {
		doneFlag = 0;
		Ret = HAL_UART_Transmit_DMA(&UART_Handle, pData, Size);
		while (doneFlag == 0) {
			Delay(100);
		}
	}
	
}


void SerialReceiveStart() {
	// Start DMA recieve
	__HAL_UART_FLUSH_DRREGISTER(&UART_Handle);
	//__HAL_DMA_ENABLE(&DMA_Rx_Handle);
	rxState = 0;
	HAL_UART_Receive_DMA(&UART_Handle, &rxBuffer, 1);
}




void SerialReceive() {
	if (strcmp(rx_Out, "") != 0) {
		char string[17];
		sprintf(string, "Serial:");
		LCD_Write_At(string, 0, 0, 1);
		sprintf(string, "%s", rx_Out);
		LCD_Write_At(string, 0, 1, 0);
		Delay(5000);
		LCD_Write_At("", 0, 0, 1);
		sprintf(rx_Out ,"");
	}
}


void SerialCheckMode(int *mode) {
	if (mode_Int != 10) {
		*mode = mode_Int;
		mode_Int = 10;
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
	doneFlag = 17;
	__HAL_UART_FLUSH_DRREGISTER(&UART_Handle); // Clear the buffer to prevent overrun
	//HAL_UART_DMAStop(huart);
	Custom_HAL_UART_AbortTransmit(huart);
	
}


int pos = 0;
	
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  __HAL_UART_FLUSH_DRREGISTER(&UART_Handle); // Clear the buffer to prevent overrun
	
	char WiFiString[5];
	sprintf(WiFiString, "+IPD");
	
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
			if (WiFiEnabled == 1)
				rxState = 4;
			else
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
						if (WiFiEnabled == 1)
							rxState = 4;
						else
							rxState = 0;
						sprintf(rx_Out, "String too long");
        }
    }
	}
	
	else if (rxState == 2) { // Expect mode
		if (((rxBuffer - '0') < 3) & (rxBuffer >= '0')){ // valid mode
			mode_Int = (int) rxBuffer - '0';
		}
		// RX complete, go back to check
		if (WiFiEnabled == 1)
			rxState = 4;
		else
			rxState = 0;
	}
	
	else if (rxState == 3) { // Check response to AT command
		int i = 0;
		if (rxBuffer == '\r') {  // Ignore /r
		
		}
		else if (rxBuffer == '\n') { // If new line
			rxString[rxindex] = 0;
			if (strcmp((char*)rxString, "OK") == 0) {
				ATResponse = 1;
			}
			if (strcmp((char*)rxString, "ERROR") == 0) {
				ATResponse = 2;
			}
			rxindex = 0;
			for (i = 0; i < 17; i++) rxString[i] = 0; // Clear the string buffer
    }

    else
    {
        rxString[rxindex] = rxBuffer; // Add that character to the string
        rxindex++;
        if (rxindex >= 17) // User typing too much, we can't have commands that big
        {
            rxindex = 0;
            for (i = 0; i < 17; i++) rxString[i] = 0; // Clear the string buffer
						sprintf(rx_Out, "");
        }
    }
	}
	
	else if (rxState == 4) {
		if (pos == 0) {
			if (rxBuffer == WiFiString[0]) {
				pos++;
			}
		}
		else if (rxBuffer == WiFiString[pos]) {
			pos++;
		}
		else {
			pos = 0;
		}
		
		if (pos >=3) {
			rxState = 0;
		}
	}
	
}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	errorFlag = 1;
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

