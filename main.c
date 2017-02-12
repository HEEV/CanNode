/**
 * \file main.c
 * \brief main file for the CanNode project for Cedarville Supermileage.
 * Automatically generated by STCubeMX then hacked beyond recognition by 
 * Samuel Ellicott
 *
 * \author Samuel Ellicott
 * \date 6-8-16
 */

#include <stdint.h>
#include <limits.h>
#include <stm32f0xx_hal.h>
#include <stdlib.h>
#include <string.h>
#include <can.h>
#include <CanNode.h>

#include "usb_device.h"
#include "usbd_cdc_if.h"

#define IO1_ADC ADC_CHANNEL_8
#define IO2_ADC ADC_CHANNEL_7

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_ADC_Init(void);

/* Private function prototypes -----------------------------------------------*/
void countRTR(CanMessage* data);
void timeRTR(CanMessage* data);
void pitotRTR(CanMessage* data);

/// Struct for initilizing ADC
ADC_HandleTypeDef hadc;
/// Struct for transmitting the wheel RPS
CanNode* wheelCountNode;
/// Struct for transmitting the wheel revolution time
CanNode* wheelTimeNode;
/// Struct for transmitting the pitot tube voltage (in mili-volts)
CanNode* pitotNode;

///varible for wheel RPS, modified by a ISR
volatile uint8_t  wheelCount;
/// Varible for wheel revolution time, modified by a ISR
volatile uint32_t wheelTime;
/// Varible used to reset wheelTime if vehicle is stopped
volatile uint32_t wheelStartTime;
// Varible used to hold the pitot voltage in mv
uint16_t pitotVoltage;

/// Timeout for reseting wheelTime (~5s without pulse)
#define WHEEL_TIMEOUT 5000
/// Make the time as long as we can to indicate a stopped wheel
#define WHEEL_STOPPED INT_MAX
///global flag (set in \ref Src/usb_cdc_if.c) for whether USB is connected
volatile uint8_t USBConnected;
uint16_t pitotVoltage;

int main(void) {
	//setup globals
	wheelCount=0;
	wheelTime=WHEEL_STOPPED;
	pitotVoltage=0;
	USBConnected = false;

	//local varibles
	//state varible for switching between transmitting RPS, time, and adc value
	uint8_t count_time_adc=0;
	float voltage;
	uint16_t switchState=0;
	uint16_t buff_len = 50;
	uint8_t buff[50];
	uint16_t data;
	uint16_t adcVal;

	// Reset of all peripherals, Initializes the Flash interface and the Systick.
	HAL_Init();
	// Configure the system clock
	SystemClock_Config();
	// Initialize all configured peripherals
	MX_GPIO_Init();
	MX_ADC_Init();
	MX_USB_DEVICE_Init();

	HAL_Delay(100);

	//setup CAN, ID's, and gives each an RTR callback
	wheelCountNode = CanNode_init(WHEEL_TACH, countRTR, true);
	wheelTimeNode = CanNode_init(WHEEL_TIME, timeRTR, true);
	pitotNode = CanNode_init(PITOT, pitotRTR, true);

	while (1) {
		//check if there is a message necessary for CanNode functionality
		CanNode_checkForMessages();

		//get the current time
		uint32_t time = HAL_GetTick();

		//stuff to do every half second
		if(time % 500 == 0){

			//read ADC value
			HAL_ADC_Start(&hadc);
			HAL_ADC_PollForConversion(&hadc, 5);
			adcVal = HAL_ADC_GetValue(&hadc);

			//do some heavy math
			voltage = adcVal/4096.0; //make the adv value something between 0 and 1
			voltage *= 3600; //multiply by the value necessary to convert to 0-3.6V signal
			pitotVoltage = (uint16_t) voltage; //put into an integer number

			//send ammount of time per revolution
			CanNode_sendData_uint32(wheelTimeNode, wheelTime);
			//send the pitot voltage
			CanNode_sendData_uint16(pitotNode, pitotVoltage);

			// give information over USB, each message is sent every
			// 1500ms.
			if(USBConnected){
				//NOTE: the maximum buffer length is set in the
				//USB code to be 64 bytes.
				char buff[50];

				//setup the buffer with the required information
				//The data is sent in a CSV format like the following
				// RPS, Time per revolution in ms, ADC value
				if(count_time_adc == 0){
					itoa(wheelCount, buff, 10);
					strcat(buff, ", ");
					count_time_adc=1;
				}
				else if(count_time_adc == 1){
					itoa(wheelTime, buff, 10);
					strcat(buff, ", ");
					count_time_adc=2;
				}
				else {
					itoa(pitotVoltage, buff, 10);
					//send a break between data sets
					strcat(buff, "\n\r");
					count_time_adc=0;
				}

				//do the actual transmission
				CDC_Transmit_FS(buff, strlen(buff));
			}
		}

		//stuff to do every second
		if(time % 1000 == 0){
			//send RPS data
			CanNode_sendData_uint8(wheelCountNode, wheelCount);

			//reset RPS varible
			wheelCount=0;

			//check if we should reset wheelTime
			if(time-wheelStartTime >= WHEEL_TIMEOUT){
				wheelTime=WHEEL_STOPPED;
			}

			//blink heartbeat LED
			HAL_GPIO_TogglePin(GPIOB, LED2_Pin);

			//make sure we don't run this code on the next loop
			HAL_Delay(1);
		}
	}
}

///RTR handler for the RPS id
void countRTR(CanMessage* data){
	CanNode_sendData_uint8(wheelCountNode, wheelCount);
}

///RTR handler for the wheel revolution time id
void timeRTR(CanMessage* data){
	CanNode_sendData_uint32(wheelTimeNode, wheelTime);
}
void pitotRTR(CanMessage* data){
	CanNode_sendData_uint16(pitotNode, pitotVoltage);
}

///callback for pin6 (IO1) interrupt
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	//when was our last pulse
	static uint32_t startTime=0;
	//what is our current time
	uint32_t newTime = HAL_GetTick();
	//how long did this revolution take
	uint32_t tempTime = newTime - startTime;

	//if the supposed time it takes for the wheel to go around is less than
	//31/32 of the last time then it is just bounce contact and should be
	//ignored
	/*
	if(tempTime < wheelTime-(wheelTime>>5) ) {
		return;
	}
	 */

	/* If the new time is less than 50ms than it was a fluke
	 */
	if(tempTime < 50){
		return;
	}

	//set the new start time
	startTime = wheelStartTime = newTime;

	wheelTime = tempTime; //Valid pulse, save the value
	++wheelCount; //increment the wheel count

	//toggle led
	HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14
	                                 | RCC_OSCILLATORTYPE_HSI48
	                                 | RCC_OSCILLATORTYPE_LSI;

	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
	RCC_OscInitStruct.HSI14CalibrationValue = 16;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI48;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
	RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV6;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK
	                            | RCC_CLOCKTYPE_SYSCLK
	                            | RCC_CLOCKTYPE_PCLK1;

	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_RTC;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC init function */
void MX_ADC_Init(void) {
	ADC_ChannelConfTypeDef sConfig;

	/**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	*/
	hadc.Instance = ADC1;
	hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc.Init.Resolution = ADC_RESOLUTION_12B;
	hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
	hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc.Init.LowPowerAutoWait = DISABLE;
	hadc.Init.LowPowerAutoPowerOff = DISABLE;
	hadc.Init.ContinuousConvMode = DISABLE;
	hadc.Init.DiscontinuousConvMode = ENABLE;
	hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc.Init.DMAContinuousRequests = DISABLE;
	hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	HAL_ADC_Init(&hadc);

	/**Configure for the selected ADC regular channel to be converted.
	*/
	sConfig.Channel = IO1_ADC;
	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	HAL_ADC_ConfigChannel(&hadc, &sConfig);
}


/** Configure pins as
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 * Free pins are configured automatically as Analog (this feature is enabled through
 * the Code Generation settings)
 */
void MX_GPIO_Init(void) {

GPIO_InitTypeDef GPIO_InitStruct;

	// Make all unused pins analog to save power
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();

	/*Configure GPIO pins : PC13 PC14 PC15 */
	GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PF0 PF1 PF11 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pins : PA0 PA1 PA2 PA3
		               PA4 PA5 PA6 PA8
		               PA9 PA10 PA15 */
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
		              |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_8
		              |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB1 PB2 PB10 PB11
		               PB12 PB13 PB14 PB15
		               PB5 PB6 PB7 */
	GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_11
		              |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
		              |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	__HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOF_CLK_DISABLE();

	//Enable used io pins
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pins : LED2_Pin LED1_Pin */
	GPIO_InitStruct.Pin = LED2_Pin|LED1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LED2_Pin|LED1_Pin, GPIO_PIN_SET);


//configure IO1 as a rising edge interrupt pin
	GPIO_InitStruct.Pin = IO2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(IO2_GPIO_Port, &GPIO_InitStruct);

	/* EXTI4_15_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}
