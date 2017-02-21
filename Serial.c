#include "Serial.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_uart.h"
#include "LED.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "LCD.h"
#include "String.h"


// replace Delay with osDelay for compatibility with RTOS
#define Delay osDelay

//=================================================//
//============ Global defines =======================//
//=================================================//

UART_HandleTypeDef UART_Handle;
DMA_HandleTypeDef DMA_Rx_Handle;
DMA_HandleTypeDef DMA_Tx_Handle;
uint8_t rxBuffer = '\000';
uint8_t rxString[100]; // where we build our string from characters coming in
int rxindex = 0; // index for going though rxString
char rx_Out[100];

int doneFlag = 0;



void SerialSend(uint8_t *pData, uint16_t Size, uint32_t Timeout) {
	//UART_Handle.gState = HAL_UART_STATE_READY;
	//HAL_StatusTypeDef Ret = HAL_UART_Transmit(&UART_Handle, pData, Size, 1000);
	//HAL_StatusTypeDef Ret = HAL_UART_Transmit_IT(&UART_Handle, pData, Size);
	doneFlag = 0;
	LED_Out(3);
	HAL_StatusTypeDef Ret = HAL_UART_Transmit_DMA(&UART_Handle, pData, Size);
	LED_Out(4);
	while (doneFlag == 0) {
		Delay(100);
	}
	LED_Out(doneFlag);
	//HAL_StatusTypeDef Ret = 2;
	//Delay(5000);
	/*
	if (Ret != HAL_OK)
		Error(Ret);
	else {
		char string[17];
		sprintf(string, "Serial sent");
		LCD_Write_At(string, 0, 0, 1);
		Delay(1000);
		LCD_Write_At("", 0, 0, 1);
	}*/
		
}


void SerialReceiveStart() {
	// Start DMA recieve
	__HAL_UART_FLUSH_DRREGISTER(&UART_Handle);
	HAL_UART_Receive_DMA(&UART_Handle, &rxBuffer, 1);
}

void SerialReceive() {
	LED_Out(1);
	LCD_Clear();
	LCD_GotoXY(0, 0);
	LCD_PutS(rx_Out);
	SerialSend((uint8_t*)rx_Out, strlen(rx_Out), 1000);
}




//=================================================//
//============ Callbacks =======================//
//=================================================//


/*
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	char string[17];
	Info(8);
	sprintf(string, "Send Success");
	LCD_Write_At(string, 0, 0, 1);
	Delay(5000);
}
*/



void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	doneFlag = 17;
	__HAL_UART_FLUSH_DRREGISTER(&UART_Handle); // Clear the buffer to prevent overrun
	//HAL_UART_DMAStop(huart);
	HAL_UART_AbortTransmit(huart);
	
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
		LED_Out(9);
    __HAL_UART_FLUSH_DRREGISTER(&UART_Handle); // Clear the buffer to prevent overrun
	
    int i = 0;

    if (rxBuffer == '\n' || rxBuffer == '\r') // If Enter
    {
			/*
			Serial_rx_t *rx = (Serial_rx_t*)osPoolAlloc(SerialMpool);
			sprintf(rx->string, "%s",rxString);
			osMessagePut(SerialMsgBox, (uint32_t)rx, osWaitForever);
*/
			rxString[rxindex] = 0;
			sprintf(rx_Out, "%s", rxString);
			rxindex = 0;
			for (i = 0; i < 100; i++) rxString[i] = 0; // Clear the string buffer
    }

    else
    {
        rxString[rxindex] = rxBuffer; // Add that character to the string
        rxindex++;
        if (rxindex > 100) // User typing too much, we can't have commands that big
        {
            rxindex = 0;
            for (i = 0; i < 100; i++) rxString[i] = 0; // Clear the string buffer
        }
    }
		
}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	doneFlag = 18;
}



void DMA1_Stream6_IRQHandler(void)
{
	//doneFlag = 20;
	NVIC_ClearPendingIRQ(DMA1_Stream6_IRQn);
	HAL_DMA_IRQHandler(&DMA_Tx_Handle);
}

void DMA1_Stream5_IRQHandler(void)
{
	//doneFlag = 20;
	LED_Out(8);
	NVIC_ClearPendingIRQ(DMA1_Stream5_IRQn);
	HAL_DMA_IRQHandler(&DMA_Rx_Handle);
}





//=================================================//
//============ Init Callbacks =======================//
//=================================================//


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
	
	// Enable callbacks
	//__NVIC_SetPriority(USART2_IRQn, 10);
	//__NVIC_EnableIRQ(USART2_IRQn);
}




// Callback from HAL
void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
	
	// Debug
	/*
	char string[17];
	sprintf(string, "HAL_StausTypeDef");
	LCD_Write_At(string, 0, 0, 1);
	Delay(1000);
	*/

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
	
	SetupCallbacks();
	SetupDMA(huart);
	
	
	// Other config if required

		/*
	    (#) Initialize the UART low level resources by implementing the HAL_UART_MspInit() API:
        (##) Enable the USARTx interface clock.
        (##) UART pins configuration:
            (+++) Enable the clock for the UART GPIOs.
            (+++) Configure these UART pins as alternate function pull-up.
        (##) NVIC configuration if you need to use interrupt process (HAL_UART_Transmit_IT()
             and HAL_UART_Receive_IT() APIs):
            (+++) Configure the USARTx interrupt priority.
            (+++) Enable the NVIC USART IRQ handle.
        (##) DMA Configuration if you need to use DMA process (HAL_UART_Transmit_DMA()
             and HAL_UART_Receive_DMA() APIs):
            (+++) Declare a DMA handle structure for the Tx/Rx stream.
            (+++) Enable the DMAx interface clock.
            (+++) Configure the declared DMA handle structure with the required 
                  Tx/Rx parameters.                
            (+++) Configure the DMA Tx/Rx Stream.
            (+++) Associate the initialized DMA handle to the UART DMA Tx/Rx handle.
            (+++) Configure the priority and enable the NVIC for the transfer complete 
                  interrupt on the DMA Tx/Rx Stream.
	*/
	
	
	
}







//=================================================//
//============ Main Init Function =======================//
//=================================================//

void SerialInit()
{
	HAL_StatusTypeDef Ret;
	LED_Out(1);
	
	sprintf(rx_Out, "Blank");
	
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
	if (Ret != HAL_OK)
		LED_Out(15);
	
	LED_Out(2);
	

/*
    The UART HAL driver can be used as follows:
    
    (#) Declare a UART_HandleTypeDef handle structure.
  
    (#) Initialize the UART low level resources by implementing the HAL_UART_MspInit() API:
        (##) Enable the USARTx interface clock.
        (##) UART pins configuration:
            (+++) Enable the clock for the UART GPIOs.
            (+++) Configure these UART pins as alternate function pull-up.
        (##) NVIC configuration if you need to use interrupt process (HAL_UART_Transmit_IT()
             and HAL_UART_Receive_IT() APIs):
            (+++) Configure the USARTx interrupt priority.
            (+++) Enable the NVIC USART IRQ handle.
        (##) DMA Configuration if you need to use DMA process (HAL_UART_Transmit_DMA()
             and HAL_UART_Receive_DMA() APIs):
            (+++) Declare a DMA handle structure for the Tx/Rx stream.
            (+++) Enable the DMAx interface clock.
            (+++) Configure the declared DMA handle structure with the required 
                  Tx/Rx parameters.                
            (+++) Configure the DMA Tx/Rx Stream.
            (+++) Associate the initialized DMA handle to the UART DMA Tx/Rx handle.
            (+++) Configure the priority and enable the NVIC for the transfer complete 
                  interrupt on the DMA Tx/Rx Stream.

    (#) Program the Baud Rate, Word Length, Stop Bit, Parity, Hardware 
        flow control and Mode(Receiver/Transmitter) in the Init structure.

    (#) For the UART asynchronous mode, initialize the UART registers by calling
        the HAL_UART_Init() API.
    
    (#) For the UART Half duplex mode, initialize the UART registers by calling 
        the HAL_HalfDuplex_Init() API.
    
    (#) For the LIN mode, initialize the UART registers by calling the HAL_LIN_Init() API.
    
    (#) For the Multi-Processor mode, initialize the UART registers by calling 
        the HAL_MultiProcessor_Init() API.
        
     [..] 
       (@) The specific UART interrupts (Transmission complete interrupt, 
            RXNE interrupt and Error Interrupts) will be managed using the macros
            __HAL_UART_ENABLE_IT() and __HAL_UART_DISABLE_IT() inside the transmit 
            and receive process.
          
     [..] 
       (@) These APIs (HAL_UART_Init() and HAL_HalfDuplex_Init()) configure also the 
            low level Hardware GPIO, CLOCK, CORTEX...etc) by calling the customized 
            HAL_UART_MspInit() API.
          
     [..] 
        Three operation modes are available within this driver :     
  
     *** Polling mode IO operation ***
     =================================
     [..]    
       (+) Send an amount of data in blocking mode using HAL_UART_Transmit() 
       (+) Receive an amount of data in blocking mode using HAL_UART_Receive()
       
     *** Interrupt mode IO operation ***    
     ===================================
     [..]    
       (+) Send an amount of data in non blocking mode using HAL_UART_Transmit_IT() 
       (+) At transmission end of transfer HAL_UART_TxCpltCallback is executed and user can 
            add his own code by customization of function pointer HAL_UART_TxCpltCallback
       (+) Receive an amount of data in non blocking mode using HAL_UART_Receive_IT() 
       (+) At reception end of transfer HAL_UART_RxCpltCallback is executed and user can 
            add his own code by customization of function pointer HAL_UART_RxCpltCallback
       (+) In case of transfer Error, HAL_UART_ErrorCallback() function is executed and user can 
            add his own code by customization of function pointer HAL_UART_ErrorCallback

     *** DMA mode IO operation ***    
     ==============================
     [..] 
       (+) Send an amount of data in non blocking mode (DMA) using HAL_UART_Transmit_DMA() 
       (+) At transmission end of half transfer HAL_UART_TxHalfCpltCallback is executed and user can 
            add his own code by customization of function pointer HAL_UART_TxHalfCpltCallback 
       (+) At transmission end of transfer HAL_UART_TxCpltCallback is executed and user can 
            add his own code by customization of function pointer HAL_UART_TxCpltCallback
       (+) Receive an amount of data in non blocking mode (DMA) using HAL_UART_Receive_DMA() 
       (+) At reception end of half transfer HAL_UART_RxHalfCpltCallback is executed and user can 
            add his own code by customization of function pointer HAL_UART_RxHalfCpltCallback 
       (+) At reception end of transfer HAL_UART_RxCpltCallback is executed and user can 
            add his own code by customization of function pointer HAL_UART_RxCpltCallback
       (+) In case of transfer Error, HAL_UART_ErrorCallback() function is executed and user can 
            add his own code by customization of function pointer HAL_UART_ErrorCallback
       (+) Pause the DMA Transfer using HAL_UART_DMAPause()      
       (+) Resume the DMA Transfer using HAL_UART_DMAResume()  
       (+) Stop the DMA Transfer using HAL_UART_DMAStop()      
    
     *** UART HAL driver macros list ***
     ============================================= 
     [..]
       Below the list of most used macros in UART HAL driver.
       
      (+) __HAL_UART_ENABLE: Enable the UART peripheral 
      (+) __HAL_UART_DISABLE: Disable the UART peripheral     
      (+) __HAL_UART_GET_FLAG : Check whether the specified UART flag is set or not
      (+) __HAL_UART_CLEAR_FLAG : Clear the specified UART pending flag
      (+) __HAL_UART_ENABLE_IT: Enable the specified UART interrupt
      (+) __HAL_UART_DISABLE_IT: Disable the specified UART interrupt
      (+) __HAL_UART_GET_IT_SOURCE: Check whether the specified UART interrupt has occurred or not
      
     [..] 
       (@) You can refer to the UART HAL driver header file for more useful macros 
      */




	
}
