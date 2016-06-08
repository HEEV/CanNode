/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <stdlib.h>
#include <string.h>

#define IO1_ADC ADC_CHSELR_CHSEL6
#define IO2_ADC ADC_CHSELR_CHSEL7
#define IO3_ADC ADC_CHSELR_CHSEL9
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{
  uint8_t buff[] = "Hello World\n\r";
  uint16_t buff_len = 14;
  uint16_t data;
 
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC_Init();
  MX_USB_DEVICE_Init();

  //select IO1 for ADC conversion
  ADC1->CHSELR = IO3_ADC;
  
  HAL_Delay(300);
  
  while (1)
  {
     //start ADC conversion
     ADC1->CR |= ADC_CR_ADSTART;
     
     LED1_GPIO_Port->BSRR |= LED1_Pin;
     LED2_GPIO_Port->BSRR |= LED2_Pin << 16;
     HAL_Delay(100);
     
     //wait for conversion to finish
     while(ADC1->CR & ADC_CR_ADSTART) {
        HAL_Delay(1);
     }
     data = ADC1->DR;

     itoa(data, buff, 10);
     strcat(buff, "\n\r");
     CDC_Transmit_FS(buff, strlen(buff));
     
     //start ADC conversion
     ADC1->CR |= ADC_CR_ADSTART;
     
     LED1_GPIO_Port->BSRR |= LED1_Pin << 16;
     LED2_GPIO_Port->BSRR |= LED2_Pin;
     HAL_Delay(100);

     //wait for conversion to finish
     while(ADC1->CR & ADC_CR_ADSTART) {
        HAL_Delay(1);
     }
     data = ADC1->DR;
     
     itoa(data, buff, 10);
     strcat(buff, "\n\r");
     CDC_Transmit_FS(buff, strlen(buff));

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/

void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

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

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC init function */

void MX_ADC_Init(void)
{
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
/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
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
