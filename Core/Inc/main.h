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
#include "stm32f4xx_hal.h"

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
#define KEY1_Pin GPIO_PIN_2
#define KEY1_GPIO_Port GPIOE
#define KEY1_EXTI_IRQn EXTI2_IRQn
#define RIGHT_ENCODER_A_Pin GPIO_PIN_0
#define RIGHT_ENCODER_A_GPIO_Port GPIOA
#define RIGHT_ENCODER_B_Pin GPIO_PIN_1
#define RIGHT_ENCODER_B_GPIO_Port GPIOA
#define MAIXCAM_SIDE_TX_Pin GPIO_PIN_2
#define MAIXCAM_SIDE_TX_GPIO_Port GPIOA
#define MAIXCAM_SIDE_RX_Pin GPIO_PIN_3
#define MAIXCAM_SIDE_RX_GPIO_Port GPIOA
#define MOTOR_L_IN1_Pin GPIO_PIN_9
#define MOTOR_L_IN1_GPIO_Port GPIOE
#define MOTOR_LEFT_IN2_Pin GPIO_PIN_11
#define MOTOR_LEFT_IN2_GPIO_Port GPIOE
#define MOTOR_RIGHT_IN1_Pin GPIO_PIN_13
#define MOTOR_RIGHT_IN1_GPIO_Port GPIOE
#define MOTOR_RIGHT_IN2_Pin GPIO_PIN_14
#define MOTOR_RIGHT_IN2_GPIO_Port GPIOE
#define LEFT_ENCODER_B_Pin GPIO_PIN_12
#define LEFT_ENCODER_B_GPIO_Port GPIOD
#define LEFT_ENCODER_A_Pin GPIO_PIN_13
#define LEFT_ENCODER_A_GPIO_Port GPIOD
#define MAIXCAM_FRONT_TX_Pin GPIO_PIN_9
#define MAIXCAM_FRONT_TX_GPIO_Port GPIOA
#define MAIXCAM_FRONT_RX_Pin GPIO_PIN_10
#define MAIXCAM_FRONT_RX_GPIO_Port GPIOA
#define LOG_TX_Pin GPIO_PIN_12
#define LOG_TX_GPIO_Port GPIOC
#define LOG_RX_Pin GPIO_PIN_2
#define LOG_RX_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
