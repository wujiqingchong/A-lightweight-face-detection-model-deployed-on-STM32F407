#ifndef __OLED_H__
#define __OLED_H__

#include "main.h"

#define OLED_SLAVER_ADDR   (0x3C)    // OLED屏幕I2C从机地址，0x3C
#define IIC_TIMEOUT        100       // I2C通信超时时间，单位是毫秒

#define OLED_WR_CMD        (0x00)    // OLED写指令时的数据标志，表示接下来写的是命令
#define OLED_WR_DATA       (0x40)    // OLED写数据时的数据标志，表示接下来写的是数据

#define OLED_ARRAY_SIZE(A) (sizeof(A)/sizeof(A[0]))  // 计算数组A的元素个数，常用宏

#define TEXTSIZE_F6X8       6  
#define TEXTSIZE_F8X16      8  
#define TEXTSIZE_F16X16     16  

/************ 函数声明 ************/
void OLED_Init(void);
ErrorStatus OLED_CheckDevice(uint8_t slave_addr);
ErrorStatus OLED_WriteByte(uint8_t cmd, uint8_t byte);
ErrorStatus OLED_WriteBuffer(uint8_t cmd, uint8_t* buffer, uint32_t num);
void OLED_SetPos(uint8_t y, uint8_t x);
void OLED_Fill(uint8_t fill_data);
void OLED_CLS(void);
void OLED_FillFull(void);

//void OLED_ShowChinese(uint8_t y, uint8_t x, uint8_t n, const uint8_t data_cn[][16]);
////void OLED_ShowChinese_F16X16(uint8_t line, uint8_t offset, uint8_t n);
//void OLED_ShowChar(uint8_t y, uint8_t x, uint8_t char_data, uint8_t textsize);
//void OLED_ShowString(uint8_t y, uint8_t x, uint8_t *string_data, uint8_t textsize);
//void OLED_ShowString_F8X16(uint8_t line, uint8_t offset, uint8_t *string_data);
void OLED_DrawBitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

#endif /* _OLED_H__ */
