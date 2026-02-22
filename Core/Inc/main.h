
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
#include "stdio.h"
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
#define ST7789_CS_Pin GPIO_PIN_3
#define ST7789_CS_GPIO_Port GPIOA
#define ST7789_DC_Pin GPIO_PIN_11
#define ST7789_DC_GPIO_Port GPIOA
#define ST7789_SCK_Pin GPIO_PIN_5
#define ST7789_SCK_GPIO_Port GPIOA
#define ST7789_RST_Pin GPIO_PIN_8
#define ST7789_RST_GPIO_Port GPIOA
#define ST7789_MOSI_Pin GPIO_PIN_7
#define ST7789_MOSI_GPIO_Port GPIOA
#define ST7789_BLK_Pin GPIO_PIN_1
#define ST7789_BLK_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */
int fputc(int ch, FILE *f);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
