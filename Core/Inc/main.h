/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32f3xx_hal.h"

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
#define NTC1_Pin GPIO_PIN_3
#define NTC1_GPIO_Port GPIOA
#define NTC2_Pin GPIO_PIN_4
#define NTC2_GPIO_Port GPIOA
#define FAN_Pin GPIO_PIN_5
#define FAN_GPIO_Port GPIOA
#define HEATER_Pin GPIO_PIN_6
#define HEATER_GPIO_Port GPIOA
#define BTN_Pin GPIO_PIN_11
#define BTN_GPIO_Port GPIOB
#define BTN_EXTI_IRQn EXTI15_10_IRQn
#define LCD_CMD_Pin GPIO_PIN_12
#define LCD_CMD_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_8
#define LCD_RST_GPIO_Port GPIOA
#define LCD_CS_Pin GPIO_PIN_9
#define LCD_CS_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
#define AUTOTUNE

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
