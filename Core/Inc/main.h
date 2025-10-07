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
#include "stm32h5xx_hal.h"

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
#define OLED_RST_Pin GPIO_PIN_13
#define OLED_RST_GPIO_Port GPIOC
#define DIN1_Pin GPIO_PIN_14
#define DIN1_GPIO_Port GPIOC
#define DIN2_Pin GPIO_PIN_15
#define DIN2_GPIO_Port GPIOC
#define DIN3_Pin GPIO_PIN_0
#define DIN3_GPIO_Port GPIOH
#define DIN4_Pin GPIO_PIN_1
#define DIN4_GPIO_Port GPIOH
#define DOUT1_Pin GPIO_PIN_0
#define DOUT1_GPIO_Port GPIOC
#define DOUT2_Pin GPIO_PIN_1
#define DOUT2_GPIO_Port GPIOC
#define DOUT3_Pin GPIO_PIN_2
#define DOUT3_GPIO_Port GPIOC
#define DOUT4_Pin GPIO_PIN_3
#define DOUT4_GPIO_Port GPIOC
#define ADC1_Pin GPIO_PIN_0
#define ADC1_GPIO_Port GPIOA
#define ADC2_Pin GPIO_PIN_1
#define ADC2_GPIO_Port GPIOA
#define ADC3_Pin GPIO_PIN_2
#define ADC3_GPIO_Port GPIOA
#define HALL_SENSOR_Pin GPIO_PIN_3
#define HALL_SENSOR_GPIO_Port GPIOA
#define HALL_SENSOR_EXTI_IRQn EXTI3_IRQn
#define AUX1_Pin GPIO_PIN_4
#define AUX1_GPIO_Port GPIOA
#define HSD50_SNS_Pin GPIO_PIN_5
#define HSD50_SNS_GPIO_Port GPIOA
#define HSD120_SNS_Pin GPIO_PIN_6
#define HSD120_SNS_GPIO_Port GPIOA
#define FB1_IN1_Pin GPIO_PIN_4
#define FB1_IN1_GPIO_Port GPIOC
#define FB1_IN2_Pin GPIO_PIN_5
#define FB1_IN2_GPIO_Port GPIOC
#define BME280_CS_Pin GPIO_PIN_0
#define BME280_CS_GPIO_Port GPIOB
#define FB1_NSLEEP_Pin GPIO_PIN_13
#define FB1_NSLEEP_GPIO_Port GPIOB
#define HSD120_EN2_Pin GPIO_PIN_14
#define HSD120_EN2_GPIO_Port GPIOB
#define HSD120_SEL1_Pin GPIO_PIN_15
#define HSD120_SEL1_GPIO_Port GPIOB
#define FB1_MODE_Pin GPIO_PIN_6
#define FB1_MODE_GPIO_Port GPIOC
#define FB1_TRQ_Pin GPIO_PIN_7
#define FB1_TRQ_GPIO_Port GPIOC
#define FB1_IN4_Pin GPIO_PIN_8
#define FB1_IN4_GPIO_Port GPIOA
#define FB1_IN3_Pin GPIO_PIN_9
#define FB1_IN3_GPIO_Port GPIOA
#define FB1_FAULT_Pin GPIO_PIN_10
#define FB1_FAULT_GPIO_Port GPIOA
#define LIFE_LED_Pin GPIO_PIN_15
#define LIFE_LED_GPIO_Port GPIOA
#define HSD120_EN1_Pin GPIO_PIN_10
#define HSD120_EN1_GPIO_Port GPIOC
#define HSD120_LATCH_Pin GPIO_PIN_11
#define HSD120_LATCH_GPIO_Port GPIOC
#define HSD120_DIA_EN_Pin GPIO_PIN_12
#define HSD120_DIA_EN_GPIO_Port GPIOC
#define HSD120_SEL2_Pin GPIO_PIN_2
#define HSD120_SEL2_GPIO_Port GPIOD
#define HSD50_EN1_Pin GPIO_PIN_3
#define HSD50_EN1_GPIO_Port GPIOB
#define HSD50_LATCH_Pin GPIO_PIN_4
#define HSD50_LATCH_GPIO_Port GPIOB
#define HSD50_DIA_EN_Pin GPIO_PIN_5
#define HSD50_DIA_EN_GPIO_Port GPIOB
#define HSD50_SEL2_Pin GPIO_PIN_6
#define HSD50_SEL2_GPIO_Port GPIOB
#define HSD50_SEL1_Pin GPIO_PIN_7
#define HSD50_SEL1_GPIO_Port GPIOB
#define HSD50_EN2_Pin GPIO_PIN_8
#define HSD50_EN2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
