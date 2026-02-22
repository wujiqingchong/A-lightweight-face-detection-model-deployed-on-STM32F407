/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "bsp_i2c.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "ai_datatypes_defines.h"
#include "ai_platform.h"
#include "yoloface50k.h"
#include "yoloface50k_data.h"
#include <math.h> 
#include "yoloface.h"
#include "st7789.h"
#include "bsp_ov2640.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void prepare_yolo_data(void);
void post_process(void);

/* USER CODE BEGIN 0 */
uint16_t image_data[56*56];//每一帧图片存在这个数组里


//ai相关
// 定义网络输入数组
// 网络输入：56x56x3 int8 (预处理后)
AI_ALIGNED(32)
static ai_i8 in_data[AI_YOLOFACE50K_IN_1_SIZE];// 56 * 56 * 3=9408字节

// 定义网络输出数组
// 网络输出：7x7x18 int8
AI_ALIGNED(32)
static ai_i8 out_data[AI_YOLOFACE50K_OUT_1_SIZE];// 7 * 7 * 18=882字节

// 人脸方框的左上右下像素坐标
int x1, y1, x2, y2;
// yoloface的anchor尺寸
uint8_t anchors[3][2] = {{9, 14}, {12, 17}, {22, 21}};

//LCD
#define TEST_COLOR 0x1F00  // 测试色

//ov2640相关
extern  DMA_HandleTypeDef DMA_Handle_dcmi;
extern uint16_t img_width, img_height;
volatile uint8_t img_frame_done = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  OV2640_IDTypeDef OV2640_Camera_ID;	
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  DEBUG_USART_Config();
  MX_SPI1_Init();	
		CAMERA_DEBUG("STM32F407 DCMI驱动\r\n");
		I2CMaster_Init();
    OV2640_HW_Init();
		HAL_Delay(500);

	aiInit();
	
	ST7789_Init();
	printf("AI和显示屏初始化完毕\r\n");
	
		/* 读取摄像头ID*/
	OV2640_ReadID(&OV2640_Camera_ID);

	if(OV2640_Camera_ID.PIDH  == 0x26)
  {
   CAMERA_DEBUG("%x%x",OV2640_Camera_ID.PIDH ,OV2640_Camera_ID.PIDL); 
  }
  else
  {
    
    CAMERA_DEBUG("没检测到摄像头\r\n");

    while(1);  
  }
    OV2640_UXGAConfig();
   
	OV2640_Init();
	printf("OV2640初始化完毕\r\n");
	
  while (1)
  {
		if(img_frame_done == 1)
  {
		printf("img_frame_done中断一次\r\n");
 // 获取图像数据
      img_frame_done = 0;		
		  ST7789_DrawImage(100,100,56, 56,image_data);		
        // 图像预处理
        prepare_yolo_data();
		
        // 调用yoloface.c里面的网络推理函数
        aiRun(in_data, out_data);
        // 图像后处理
        post_process();
	}

  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}


//重定义fputc函数 
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}


// 网络预处理函数（完全保持你的原样）
void prepare_yolo_data(void)
{
    for(int i = 0; i < 56; i++) {
        for(int j = 0; j < 56; j++) {
            uint16_t color = image_data[j + i * 56];  // 存储顺序
            
            // 提取R、G、B并减去128
            in_data[(j + i * 56) * 3]     = (int8_t)((color & 0xF800) >> 9) - 128;  // R
            in_data[(j + i * 56) * 3 + 1] = (int8_t)((color & 0x07E0) >> 3) - 128;  // G
            in_data[(j + i * 56) * 3 + 2] = (int8_t)((color & 0x001F) << 3) - 128;  // B
        }
    }
}


// 定义sigmoid函数
float sigmoid(float x)
{
    return 1.0f / (1.0f + expf(-x));
}

// 网络后处理函数
void post_process(void)
{
    int grid_x, grid_y;
    float x, y, w, h;
    
    for(int i = 0; i < 49; i++) {
        for(int j = 0; j < 3; j++) {
            int8_t conf = out_data[i * 18 + j * 6 + 4];
            
            if(conf > -16) {
                grid_x = i % 7;
                grid_y = (i - grid_x) / 7;
                
                x = ((float)out_data[i * 18 + j * 6] + 15) * 0.14218327403068542f;
                y = ((float)out_data[i * 18 + j * 6 + 1] + 15) * 0.14218327403068542f;
                w = ((float)out_data[i * 18 + j * 6 + 2] + 15) * 0.14218327403068542f;
                h = ((float)out_data[i * 18 + j * 6 + 3] + 15) * 0.14218327403068542f;
                
                x = (sigmoid(x) + grid_x) * 8;
                y = (sigmoid(y) + grid_y) * 8;
                w = expf(w) * anchors[j][0];
                h = expf(h) * anchors[j][1];
                
//				y2 = (x - w/2);
//				y1 = (x + w/2);
//				x1 = y - h/2;
//				x2 = y + h/2;
							  x1 = (int)(x - w/2);  // 左上x
                y1 = (int)(y - h/2);  // 左上y
                x2 = (int)(x + w/2);  // 右下x
                y2 = (int)(y + h/2);  // 右下y
                
                if(x1 < 0) x1 = 0;
                if(y1 < 0) y1 = 0;
                if(x2 > 55) x2 = 55;
                if(y2 > 55) y2 = 55;
                
                // 绘制检测框
                printf("Face detected: [%d,%d] to [%d,%d]\r\n", x1, y1, x2, y2);
								printf("conf: %d\r\n",conf);
                // GUI_Rectangle(x1, y1, x2, y2, RED);
								ST7789_DrawLine(x1+100, y1+100, x2+100, y1+100,0XF800);
								ST7789_DrawLine(x1+100, y1+100, x1+100, y2+100,0XF800);
								ST7789_DrawLine(x1+100, y2+100, x2+100, y2+100,0XF800);
								ST7789_DrawLine(x2+100, y1+100, x2+100, y2+100,0XF800);		
										HAL_Delay(200);								
            }
        }
    }
}


/**
  * @brief  DCMI帧完成中断回调函数（HAL库自动调用）
  * @param  hdcmi: DCMI句柄
  * @retval None
  * @note   此函数是弱定义，重写后会覆盖HAL库的默认空函数
  */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
  if(hdcmi->Instance == DCMI) // 确认是DCMI帧中断
  {
    // 1. 置位帧完成标志位，通知主循环处理图像
    img_frame_done = 1;

//    // 2. 暂停DMA传输（关键！避免YOLO处理图像时，新的摄像头数据覆盖image_data）
//    HAL_DMA_Pause(&DMA_Handle_dcmi);

//    // 3. 清除DCMI帧中断标志位（防止中断重复触发）
//    __HAL_DCMI_CLEAR_FLAG(&DCMI_Handle, DCMI_FLAG_FRAME);
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
