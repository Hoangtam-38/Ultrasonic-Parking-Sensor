/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TRIG_LEFT_Pin GPIO_PIN_1
#define TRIG_LEFT_GPIO_Port GPIOA
#define TRIG_RIGHT_Pin GPIO_PIN_6
#define TRIG_RIGHT_GPIO_Port GPIOA
#define SEG_A_Pin GPIO_PIN_0
#define SEG_A_GPIO_Port GPIOB
#define SEG_B_Pin GPIO_PIN_1
#define SEG_B_GPIO_Port GPIOB
#define SEG_C_Pin GPIO_PIN_2
#define SEG_C_GPIO_Port GPIOB
#define LEVEL2_Pin GPIO_PIN_12
#define LEVEL2_GPIO_Port GPIOB
#define LEVEL3_Pin GPIO_PIN_13
#define LEVEL3_GPIO_Port GPIOB
#define LEVEL4_Pin GPIO_PIN_14
#define LEVEL4_GPIO_Port GPIOB
#define LEVEL5_Pin GPIO_PIN_15
#define LEVEL5_GPIO_Port GPIOB
#define DIGIT1_Pin GPIO_PIN_9
#define DIGIT1_GPIO_Port GPIOA
#define DIGIT2_Pin GPIO_PIN_10
#define DIGIT2_GPIO_Port GPIOA
#define DIGIT3_Pin GPIO_PIN_11
#define DIGIT3_GPIO_Port GPIOA
#define DIGIT4_Pin GPIO_PIN_12
#define DIGIT4_GPIO_Port GPIOA
#define SEG_D_Pin GPIO_PIN_3
#define SEG_D_GPIO_Port GPIOB
#define SEG_E_Pin GPIO_PIN_4
#define SEG_E_GPIO_Port GPIOB
#define SEG_F_Pin GPIO_PIN_5
#define SEG_F_GPIO_Port GPIOB
#define SEG_G_Pin GPIO_PIN_6
#define SEG_G_GPIO_Port GPIOB
#define SEG_DP_Pin GPIO_PIN_7
#define SEG_DP_GPIO_Port GPIOB
#define LEVEL1_Pin GPIO_PIN_8
#define LEVEL1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
