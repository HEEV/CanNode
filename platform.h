#ifndef _PLATFORM_CAN_H_
#define _PLATFORM_CAN_H_
#include <stm32f0xx.h>
#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_can.h>

#define CAN_EN_GPIO_Port GPIOB
#define CAN_EN_Pin GPIO_PIN_7

//#define HAL_Delay(ms_delay) (usleep(ms_delay * 1000))

#endif //_PLATFORM_CAN_H_