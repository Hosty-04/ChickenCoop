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
#include "stm32wlxx_hal.h"

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
#define RTC_N_PREDIV_S 10
#define RTC_PREDIV_S ((1<<RTC_N_PREDIV_S)-1)
#define RTC_PREDIV_A ((1<<(15-RTC_N_PREDIV_S))-1)
#define RTC_PREDIV_A ((1<<(15-RTC_N_PREDIV_S))-1)
#define RTC_N_PREDIV_S 10
#define RTC_PREDIV_S ((1<<RTC_N_PREDIV_S)-1)
#define DEBUG_SWCLK_Pin GPIO_PIN_14
#define DEBUG_SWCLK_GPIO_Port GPIOA
#define I2C_SDA_Pin GPIO_PIN_15
#define I2C_SDA_GPIO_Port GPIOA
#define I2C_SCL_Pin GPIO_PIN_15
#define I2C_SCL_GPIO_Port GPIOB
#define DEBUG_SWDIO_Pin GPIO_PIN_13
#define DEBUG_SWDIO_GPIO_Port GPIOA
#define DIV_Pin GPIO_PIN_3
#define DIV_GPIO_Port GPIOB
#define MAIN_SW_Pin GPIO_PIN_7
#define MAIN_SW_GPIO_Port GPIOB
#define EN_Pin GPIO_PIN_9
#define EN_GPIO_Port GPIOB
#define nSLEEP_Pin GPIO_PIN_14
#define nSLEEP_GPIO_Port GPIOB
#define LPUART_CRTL_Pin GPIO_PIN_0
#define LPUART_CRTL_GPIO_Port GPIOA
#define PH_Pin GPIO_PIN_13
#define PH_GPIO_Port GPIOB
#define SEP_Pin GPIO_PIN_6
#define SEP_GPIO_Port GPIOB
#define LIM_SW2_Pin GPIO_PIN_9
#define LIM_SW2_GPIO_Port GPIOA
#define LPUART_TX_Pin GPIO_PIN_1
#define LPUART_TX_GPIO_Port GPIOC
#define LPUART_RX_Pin GPIO_PIN_0
#define LPUART_RX_GPIO_Port GPIOC
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define LIM_SW1_Pin GPIO_PIN_10
#define LIM_SW1_GPIO_Port GPIOB
#define RF_CTRL1_Pin GPIO_PIN_4
#define RF_CTRL1_GPIO_Port GPIOA
#define RF_CTRL2_Pin GPIO_PIN_5
#define RF_CTRL2_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
