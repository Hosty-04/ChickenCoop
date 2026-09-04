/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    radio_board_if.c
  * @author  MCD Application Team
  * @brief   This file provides an interface layer between MW and Radio Board
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

/* Includes ------------------------------------------------------------------*/
#include "radio_board_if.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/

int32_t RBI_Init(void)
{
  /* USER CODE BEGIN RBI_Init_1 */
  /* USER CODE END RBI_Init_1 */
#if defined(USE_BSP_DRIVER)
  return BSP_RADIO_Init();
#else
  int32_t retcode = 0;
  /* USER CODE BEGIN RBI_Init_2 */
  // GPIO for RF switch already initialized by CubeMX (MX_GPIO_Init)
  /* USER CODE END RBI_Init_2 */
  return retcode;
#endif /* USE_BSP_DRIVER */
}

int32_t RBI_DeInit(void)
{
  /* USER CODE BEGIN RBI_DeInit_1 */
  /* USER CODE END RBI_DeInit_1 */
#if defined(USE_BSP_DRIVER)
  return BSP_RADIO_DeInit();
#else
  int32_t retcode = 0;
  /* USER CODE BEGIN RBI_DeInit_2 */
  /* USER CODE END RBI_DeInit_2 */
  return retcode;
#endif /* USE_BSP_DRIVER */
}

int32_t RBI_ConfigRFSwitch(RBI_Switch_TypeDef Config)
{
  /* USER CODE BEGIN RBI_ConfigRFSwitch_1 */
  /* USER CODE END RBI_ConfigRFSwitch_1 */
#if defined(USE_BSP_DRIVER)
  return BSP_RADIO_ConfigRFSwitch((BSP_RADIO_Switch_TypeDef) Config);
#else
  int32_t retcode = 0;
  /* USER CODE BEGIN RBI_ConfigRFSwitch_2 */

  switch (Config)
  {
    case RBI_SWITCH_OFF:
    {
      /* Turn off RF switch */
      HAL_GPIO_WritePin(RF_CTRL1_GPIO_Port, RF_CTRL1_Pin, GPIO_PIN_RESET); // PA4
      HAL_GPIO_WritePin(RF_CTRL2_GPIO_Port, RF_CTRL2_Pin, GPIO_PIN_RESET); // PA5
      break;
    }
    case RBI_SWITCH_RX:
    {
      /* Receive: PA4 = 1, PA5 = 0 */
      HAL_GPIO_WritePin(RF_CTRL1_GPIO_Port, RF_CTRL1_Pin, GPIO_PIN_SET); // PA4 = 1
      HAL_GPIO_WritePin(RF_CTRL2_GPIO_Port, RF_CTRL2_Pin, GPIO_PIN_RESET);   // PA5 = 0
      break;
    }
    case RBI_SWITCH_RFO_LP:
    case RBI_SWITCH_RFO_HP:
    {
      /* Transmit High Power (RFO_HP): PA4 = 0, PA5 = 1 */
      HAL_GPIO_WritePin(RF_CTRL1_GPIO_Port, RF_CTRL1_Pin, GPIO_PIN_RESET);   // PA4 = 0
      HAL_GPIO_WritePin(RF_CTRL2_GPIO_Port, RF_CTRL2_Pin, GPIO_PIN_SET); // PA5 = 1
      break;
    }
    default:
      break;
  }

  /* USER CODE END RBI_ConfigRFSwitch_2 */
  return retcode;
#endif /* USE_BSP_DRIVER */
}

int32_t RBI_GetTxConfig(void)
{
  /* USER CODE BEGIN RBI_GetTxConfig_1 */
  /* USER CODE END RBI_GetTxConfig_1 */
#if defined(USE_BSP_DRIVER)
  return BSP_RADIO_GetTxConfig();
#else
  /* LoRa-E5 transmits only via RFO_HP (High Power) */
  int32_t retcode = RBI_CONF_RFO_HP;
  /* USER CODE BEGIN RBI_GetTxConfig_2 */
  /* USER CODE END RBI_GetTxConfig_2 */
  return retcode;
#endif /* USE_BSP_DRIVER */
}

int32_t RBI_IsTCXO(void)
{
  /* USER CODE BEGIN RBI_IsTCXO_1 */
  /* USER CODE END RBI_IsTCXO_1 */
#if defined(USE_BSP_DRIVER)
  return BSP_RADIO_IsTCXO();
#else
  /* LoRa-E5 has TCXO */
  int32_t retcode = 1;
  /* USER CODE BEGIN RBI_IsTCXO_2 */
  /* USER CODE END RBI_IsTCXO_2 */
  return retcode;
#endif /* USE_BSP_DRIVER */
}

int32_t RBI_IsDCDC(void)
{
  /* USER CODE BEGIN RBI_IsDCDC_1 */
  /* USER CODE END RBI_IsDCDC_1 */
#if defined(USE_BSP_DRIVER)
  return BSP_RADIO_IsDCDC();
#else
  /* Turn on SMPS (DCDC) */
  int32_t retcode = 1;
  /* USER CODE BEGIN RBI_IsDCDC_2 */
  /* USER CODE END RBI_IsDCDC_2 */
  return retcode;
#endif /* USE_BSP_DRIVER */
}

int32_t RBI_GetRFOMaxPowerConfig(RBI_RFOMaxPowerConfig_TypeDef Config)
{
  /* USER CODE BEGIN RBI_GetRFOMaxPowerConfig_1 */
  /* USER CODE END RBI_GetRFOMaxPowerConfig_1 */
#if defined(USE_BSP_DRIVER)
  return BSP_RADIO_GetRFOMaxPowerConfig((BSP_RADIO_RFOMaxPowerConfig_TypeDef) Config);
#else
  int32_t ret = 0;
  /* USER CODE BEGIN RBI_GetRFOMaxPowerConfig_2 */

  if (Config == RBI_RFO_LP_MAXPOWER)
  {
    ret = 15; /* dBm */
  }
  else
  {
    ret = 22; /* dBm - High Power */
  }
  
  /* USER CODE END RBI_GetRFOMaxPowerConfig_2 */
  return ret;
#endif /* USE_BSP_DRIVER */
}

/* USER CODE BEGIN EF */
/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/
/* USER CODE BEGIN PrFD */
/* USER CODE END PrFD */