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
#include <stm32f0xx_hal.h>
#include <stdlib.h>
#include <string.h>
#include <../Inc/can.h>
#include <../Inc/CanNode.h>

#define IO1_ADC ADC_CHSELR_CHSEL6
#define IO2_ADC ADC_CHSELR_CHSEL7
#define IO3_ADC ADC_CHSELR_CHSEL9

//transmit code or recieve code
#define RECIEVE

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_NVIC_Init(void);
void MX_ADC_Init(void);

/* Private function prototypes -----------------------------------------------*/
void countRTR(CanMessage* data);
void timeRTR(CanMessage* data);

CanNode* wheelCountNode;
CanNode* wheelTimeNode;
volatile uint8_t  wheelCount;
volatile uint32_t wheelTime;

int main(void) {
	wheelCount=0;
	wheelTime=0;
	uint16_t switchState=0;

	// Reset of all peripherals, Initializes the Flash interface and the Systick.
	HAL_Init();

	// Configure the system clock 
	SystemClock_Config();

	// Initialize all configured peripherals
	MX_GPIO_Init();
	MX_NVIC_Init();
	//MX_ADC_Init();

	wheelCountNode = CanNode_init(WHEEL_TACH, countRTR, true);
	wheelTimeNode = CanNode_init(WHEEL_TIME, timeRTR, true);

	//setup name/info strings
	const char* wheelCountName = "Wheel revolution count";
	const char* wheelCountInfo = "Gives the number of wheel revolutions in the "
		                         "last second.";
	CanNode_setName(wheelCountNode, wheelCountName, sizeof(wheelCountName));
	CanNode_setInfo(wheelCountNode, wheelCountInfo, sizeof(wheelCountInfo));

	const char* wheelTimeName = "Wheel rotation time";
	const char* wheelTimeInfo = "Gives the ammount of time that the previous "
		                        "Wheel revolution took.";
	CanNode_setName(wheelTimeNode, wheelTimeName, sizeof(wheelTimeName));
	CanNode_setInfo(wheelTimeNode, wheelTimeInfo, sizeof(wheelTimeInfo));

	while (1) {
		//get the current time
		uint32_t time = HAL_GetTick();
		//check if there is a message necessary for CanNode functionality
		CanNode_checkForMessages();

		switchState = GPIOA->IDR & GPIO_IDR_6;

		if(switchState){
			LED1_GPIO_Port->ODR |= LED1_Pin;
			switchState = 1;
		}
		else{
			LED1_GPIO_Port->ODR &= ~LED1_Pin;
			switchState = 0;
		}
		//send time data once every 0.5s
		if(time % 500 == 0){
			CanNode_sendData_uint32(wheelTimeNode, wheelTime);
		}
		//send and reset count varible every second
		if(time % 1000 == 0){
			CanNode_sendData_uint8(wheelCountNode, wheelCount);
			wheelCount=0;
		}
	}
}

void countRTR(CanMessage* data){
	CanNode_sendData_uint8(wheelCountNode, wheelCount);
}

void timeRTR(CanMessage* data){
	CanNode_sendData_uint32(wheelTimeNode, wheelTime);
}

//callback for pin6 (IO1) interrupt

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	static uint32_t startTime=0;
	uint32_t newTime = HAL_GetTick();
	uint32_t tempTime = newTime - startTime;
	
	//if the supposed time it takes for the wheel to go around is less than
	//31/32 of the last time then it is just contact bounce and should be 
	//ignored
	if(tempTime < wheelTime-(wheelTime>>5) ) {
	    return;
	}
	//get the elapsed time
	wheelTime = tempTime;
	//set the new start time
	startTime = newTime;
	//increment the wheel count
	++wheelCount;
	//toggle led
	LED2_GPIO_Port->ODR ^= LED2_Pin;
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

    //GPIOA->MODER |= //GPIO_MODER_MODER6_0 | GPIO_MODER_MODER6_1 //PA6 leave as input
    //               GPIO_MODER_MODER7_0 | GPIO_MODER_MODER6_1;//PA7

	//configure GPIOA-pin6 as a rising edge interrupt pin
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/** 
 * NVIC Configuration
 */
void MX_NVIC_Init(void) {
	/* EXTI4_15_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}
