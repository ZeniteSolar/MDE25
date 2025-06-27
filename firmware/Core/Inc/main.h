/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DEBUG_0_Pin GPIO_PIN_13
#define DEBUG_0_GPIO_Port GPIOC
#define DEBUG_1_Pin GPIO_PIN_14
#define DEBUG_1_GPIO_Port GPIOC
#define ADC_POSITION_Pin GPIO_PIN_0
#define ADC_POSITION_GPIO_Port GPIOC
#define ADC_INPUT_VOLTAGE_Pin GPIO_PIN_1
#define ADC_INPUT_VOLTAGE_GPIO_Port GPIOC
#define ADC_OUTPUT_VOLTAGE_P_Pin GPIO_PIN_2
#define ADC_OUTPUT_VOLTAGE_P_GPIO_Port GPIOC
#define ADC_OUTPUT_VOLTAGE_N_Pin GPIO_PIN_3
#define ADC_OUTPUT_VOLTAGE_N_GPIO_Port GPIOC
#define ADC_INPUT_CURRENT_Pin GPIO_PIN_0
#define ADC_INPUT_CURRENT_GPIO_Port GPIOA
#define ENCODER_1_Pin GPIO_PIN_1
#define ENCODER_1_GPIO_Port GPIOA
#define ADC_TEMPERATURE_Pin GPIO_PIN_2
#define ADC_TEMPERATURE_GPIO_Port GPIOA
#define PWM_LEFT_LOW_Pin GPIO_PIN_7
#define PWM_LEFT_LOW_GPIO_Port GPIOA
#define PWM_RIGHT_LOW_Pin GPIO_PIN_0
#define PWM_RIGHT_LOW_GPIO_Port GPIOB
#define RED_LED_Pin GPIO_PIN_12
#define RED_LED_GPIO_Port GPIOB
#define YELLOW_LED_Pin GPIO_PIN_13
#define YELLOW_LED_GPIO_Port GPIOB
#define GREEN_LED_Pin GPIO_PIN_14
#define GREEN_LED_GPIO_Port GPIOB
#define CENTER_SWITCH_Pin GPIO_PIN_15
#define CENTER_SWITCH_GPIO_Port GPIOB
#define START_SWITCH_Pin GPIO_PIN_6
#define START_SWITCH_GPIO_Port GPIOC
#define END_SWITCH_Pin GPIO_PIN_7
#define END_SWITCH_GPIO_Port GPIOC
#define PWM_LEFT_HIGH_Pin GPIO_PIN_8
#define PWM_LEFT_HIGH_GPIO_Port GPIOA
#define PWM_RIGHT_HIGH_Pin GPIO_PIN_9
#define PWM_RIGHT_HIGH_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define ENCODER_2_Pin GPIO_PIN_15
#define ENCODER_2_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
