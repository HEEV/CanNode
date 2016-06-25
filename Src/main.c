/* main.c - main file for the CanNode project for Cedarville Supermileage.
 * Automatically generated by STCubeMX then hacked beyond recognition by 
 * Samuel Ellicott
 *
 * Author:	Samuel Ellicott
 * Date:	6-8-16
 */

#include <stdint.h>
#include "stm32f0xx_hal.h"
#include <stdlib.h>
#include <string.h>
#include "../Inc/can.h"
#include "../Inc/CanNode.h"

#define IO1_ADC ADC_CHSELR_CHSEL6
#define IO2_ADC ADC_CHSELR_CHSEL7
#define IO3_ADC ADC_CHSELR_CHSEL9

//transmit code or recieve code
//#define RECIEVE

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);

/* Private function prototypes -----------------------------------------------*/
int getDelay(uint16_t data);
void nodeHandler(CanMessage* data);
uint16_t canData;
CanNode node;

int main(void) {
	int tick;
	int timeRemoved = 0;
#ifdef RECIEVE
	CanMessage rx_msg;
#else
	CanNode hiNode;
	uint16_t adcVal;
#endif
	uint32_t status;

	// Reset of all peripherals, Initializes the Flash interface and the Systick.
	HAL_Init();

	// Configure the system clock 
	SystemClock_Config();

	// Initialize all configured peripherals
	MX_GPIO_Init();
	MX_ADC_Init();

	//select IO1 for ADC conversion
	ADC1->CHSELR = IO3_ADC;
	
#ifndef RECIEVE
	//setup basic can frame
	CanNode_init(&node, ANALOG, CAN_LOW_PRIORITY);
	node.id = 1200;
	CanNode_init(&hiNode, UNCONFIG, CAN_LOW_PRIORITY);
	hiNode.id = 0x7EF;
#else
	CanNode_init(&node, UNCONFIG, CAN_LOW_PRIORITY);
	CanNode_addFilter(&node, 1200, nodeHandler);
	CanNode_addFilter(&node, 0x7EF, nodeHandler);
#endif

	while (1) {
#ifdef RECIEVE
		CanNode_checkForMessages();
		//check if there is a message
		tick = getDelay(canData);
#else
		//start ADC conversion
		ADC1->CR |= ADC_CR_ADSTART;

		//wait for conversion to finish
		while(ADC1->CR & ADC_CR_ADSTART) {
			//HAL_Delay(1);
		}
		adcVal = ADC1->DR;
		tick = getDelay(adcVal);
		CanNode_sendData_uint16(&node, adcVal);
		char msg[4] = "Hi!";
		CanNode_sendDataArr_int8(&hiNode, (int8_t*) msg, 3);
#endif
		
		if(tick - timeRemoved <= 0){ //toggle lights
			timeRemoved=0;

			LED2_GPIO_Port->ODR ^= LED2_Pin;
		}
		timeRemoved++; 

		HAL_Delay(1);

	}
}

int getDelay(uint16_t data){

	const uint16_t MAX_DELAY = 300; //max delay 500ms
	const uint8_t MIN_DELAY = 20; //min delay 10ms
	const uint16_t MAX_ADC = 4096;
	
	uint32_t temp = (data) * (MAX_DELAY - MIN_DELAY);
	
	return temp / MAX_ADC + MIN_DELAY;
}

void nodeHandler(CanMessage* data){
	if(data->id == 1200){
		canData = 0;
		canData = data->data[0];
		//bit shift second byte of data then mask it
		canData |= (data->data[1] << 8) & 0xFF00;
	}
	else{
		char msg[6] = "Poke!";
		CanNode_sendDataArr_int8(&node, (int8_t*) msg, 6);
	}
}

/** System Clock Configuration
*/
void SystemClock_Config(void) {

	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14|RCC_OSCILLATORTYPE_HSI48;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
	RCC_OscInitStruct.HSI14CalibrationValue = 16;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void MX_ADC_Init(void) {
    //configure ADC clock
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; //enable ADC clock
    RCC->CR2 |= RCC_CR2_HSI14ON; //enable sample clock
    while ((RCC->CR2 & RCC_CR2_HSI14RDY) == 0) { //wait for clock to start up
        HAL_Delay(1);
    }

    //configure ADC
    ADC1->CR |= ADC_CR_ADEN; //enable ADC
    while ((ADC1->ISR & ADC_ISR_ADRDY) == 0){ //wait until ready
        HAL_Delay(1);
    }
    ADC1->CFGR2 &= (~ADC_CFGR2_CKMODE); //set ADC clock mode

}

void MX_GPIO_Init(void) {

    //GPIO Clock enable
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;

    //set initial output to low
    GPIOA->BSRR |= (LED1_Pin | LED2_Pin) << 16;

    //configure the pin settings LED1 (PA4) and LED2 (PA5)
    GPIOA->MODER |= GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0; //set to output modes
    //all other settings are okay

    //setup PA6 (IO1), PA7 (IO2), and PB1(IO3) as analog inputs
    GPIOA->MODER |= GPIO_MODER_MODER6_0 | GPIO_MODER_MODER6_1 //PA6
                 |  GPIO_MODER_MODER7_0 | GPIO_MODER_MODER6_1;//PA7

    GPIOB->MODER |= GPIO_MODER_MODER1_0 | GPIO_MODER_MODER1_1;//PB1
}

