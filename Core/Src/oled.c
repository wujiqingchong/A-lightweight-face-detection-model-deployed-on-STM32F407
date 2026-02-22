#include "usart.h"
#include "i2c.h"
#include "oled.h"
#include "string.h"
#include "stdlib.h"
/**
  * @brief  检测I2C设备是否存在
  * @param  slave_addr：7位I2C设备地址
  * @retval SUCCESS 表示设备存在，ERROR 表示设备不存在
  */
ErrorStatus OLED_CheckDevice(uint8_t slave_addr)
{
    uint8_t dummy = 0;
    
    // 检测通信
    if (HAL_I2C_Master_Transmit(&hi2c1, (slave_addr << 1), &dummy, 0, IIC_TIMEOUT) == HAL_OK)
    {
        return SUCCESS;
    }
    
    return ERROR;
}

/**
  * @brief  写入单字节数据到 OLED（可以是命令或数据）
  * @param  cmd: 写入模式，取值为 OLED_WR_CMD（写命令）或 OLED_WR_DATA（写数据）
  * @param  byte: 需要写入的具体数据或命令字节
  * @retval SUCCESS 表示写入成功，ERROR 表示写入失败
  */
ErrorStatus OLED_WriteByte(uint8_t cmd, uint8_t byte)
{
    uint8_t buf[2] = {cmd, byte};  // 先放控制字节，再放要写的数据，形成2字节发送数据包

    // 通过I2C主机发送数据包给OLED设备，地址左移1位为7位地址格式，发送2个字节，超时为IIC_TIMEOUT
    if (HAL_I2C_Master_Transmit(&hi2c1, OLED_SLAVER_ADDR << 1, buf, 2, IIC_TIMEOUT) == HAL_OK)
        return SUCCESS;            // 发送成功，返回成功状态

    // 发送失败，打印错误信息，方便调试
    printf("OLED_WriteByte 写入失败：cmd=0x%02X byte=0x%02X\r\n", cmd, byte);
    return ERROR;                  // 返回失败状态
}

/**
  * @brief  向 OLED 写入多个字节（包括命令字节和数据字节）
  * @param  cmd: 写入模式，取值为 OLED_WR_CMD（写命令）或 OLED_WR_DATA（写数据）
  * @param  buffer: 指向需要写入的数据缓冲区指针
  * @param  num: 要写入的数据字节数
  * @retval SUCCESS 表示写入成功，ERROR 表示写入失败
  */
ErrorStatus OLED_WriteBuffer(uint8_t cmd, uint8_t *buffer, uint32_t num)
{
    // 最大256字节，可根据实际OLED限制调整
    uint8_t buf[256];
    if(num > sizeof(buf) - 1)
    {
        printf("OLED_WriteBuffer 数据过长(%u)\r\n", num);
        return ERROR;
    }

    buf[0] = cmd;                 // 第一个字节为命令或数据标识
    memcpy(&buf[1], buffer, num);  // 将实际数据复制到buf数组，从第二个字节开始存放

    // 通过I2C发送数据给OLED，发送长度为num+1字节
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, OLED_SLAVER_ADDR << 1, buf, num + 1, IIC_TIMEOUT);

    if (status == HAL_OK)
        return SUCCESS;          // 发送成功返回成功状态

    // 发送失败，打印调试信息
    printf("OLED_WriteBuffer 写入失败：cmd=0x%02X len=%u\r\n", cmd, num);
    return ERROR;                // 返回失败状态
}

/**
  * @brief  设置 OLED 显示的光标位置
  * @param  y：页地址，范围 0~7，对应 OLED 的行（每页8像素高）
  * @param  x：列地址，范围 0~127，对应 OLED 的列位置
  * @retval 无
  */
void OLED_SetPos(uint8_t y, uint8_t x)
{
    // 构造三个命令字节设置 OLED 显示起始地址
    uint8_t pos_buf[3] = {
        0xB0 + y,               	// 设置页地址命令，0xB0 是页地址起始，后加页码
        ((x & 0xF0) >> 4) | 0x10, // 设置列地址高4位 + 0x10 (列地址高4位起始命令)
        (x & 0x0F)              	// 设置列地址低4位
    };

    // 通过写命令方式将位置命令发送给 OLED，3字节
    OLED_WriteBuffer(OLED_WR_CMD, pos_buf, sizeof(pos_buf));
}

/**
  * @brief  填充整个屏幕
  * @param  fill_data: 要填充的数据（0x00 全黑，0xFF 全亮等）
  * @retval 无
  */
void OLED_Fill(uint8_t fill_data)
{
    uint8_t data_buffer_temp[128] = {0};
    // 将数组的128个字节全部设置为 fill_data，准备写入整行数据
    memset(data_buffer_temp, fill_data, 128);

    // OLED共有8页，每页128列，循环逐页写入数据
    for(uint8_t m = 0; m < 8; m++)
    {
        OLED_SetPos(m, 0); // 设置光标位置到第m页第0列
        OLED_WriteBuffer(OLED_WR_DATA, data_buffer_temp, OLED_ARRAY_SIZE(data_buffer_temp)); // 写入整页数据
    }
}

/**
  * @brief  清屏函数
  * @param  无
  * @retval 无
  * @note   调用 OLED_Fill(0x00) 将屏幕所有像素全部关闭，达到清屏效果
  */
void OLED_CLS(void)
{
    OLED_Fill(0x00);  // 填充 0x00，所有像素点灭，清空屏幕显示内容
}

/**
  * @brief  OLED全屏点亮函数
  * @param  无
  * @retval 无
  * @note   调用 OLED_Fill(0xFF) 将屏幕所有像素全部点亮，显示全白
  */
void OLED_FillFull(void)
{
    OLED_Fill(0xFF);  // 填充 0xFF，所有像素点亮，屏幕亮满
}

/**
  * @brief  OLED 初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
    HAL_Delay(100);  // 延时保证电源稳定

    while (OLED_CheckDevice(OLED_SLAVER_ADDR) != SUCCESS);

		/* 控制显示 */
		OLED_WriteByte(OLED_WR_CMD,0xAE);//设置显示打开/关闭(AFh/AEh)

		/* 控制内存寻址模式 */
		OLED_WriteByte(OLED_WR_CMD,0x20);//设置内存寻址模式(20h)
		OLED_WriteByte(OLED_WR_CMD,0x02);//00b，水平寻址模式;01b，垂直寻址模式;10b，页面寻址模式(RESET);11，无效

		/* 页起始地址 */ 
		OLED_WriteByte(OLED_WR_CMD,0xB0);//设置页面寻址模式的页面起始地址，0-7(B0-B7)(PAGE0-PAGE7)

		/* COM输出扫描方向 */
		OLED_WriteByte(OLED_WR_CMD,0xA1);//设置左右方向，0xA1正常 0xA0左右反置
		OLED_WriteByte(OLED_WR_CMD,0xC8);//设置上下方向，0xC8正常 0xC0上下反置

		/* 页内列起始地址 */
		OLED_WriteByte(OLED_WR_CMD,0x00);//设置列地址低位0-7
		OLED_WriteByte(OLED_WR_CMD,0x10);//设置列地址高位0-F(10h-1Fh) 列序号=列地址低位*列地址高位(最高位不参与乘积)

		/* 页内行起始地址 */
		OLED_WriteByte(OLED_WR_CMD,0x40);//设置起始行地址

		/* 设置对比度 */
		OLED_WriteByte(OLED_WR_CMD,0x81);//设置对比度控制寄存器(81h)
		OLED_WriteByte(OLED_WR_CMD,0xff);//0x00~0xff

		/* 设置显示方向 */
		OLED_WriteByte(OLED_WR_CMD,0xA1);//将列地址0映射到SEG0(A0h)/将列地址127映射到SEG0(A1h)
		OLED_WriteByte(OLED_WR_CMD,0xA6);//设置正常显示(A6h)/倒转显示(A7h)

		/* 设置多路复用率 */
		OLED_WriteByte(OLED_WR_CMD,0xA8);//多路复用率(A8h)
		OLED_WriteByte(OLED_WR_CMD,0x3F);//(1 ~ 64)

		/* 全屏显示 */
		OLED_WriteByte(OLED_WR_CMD,0xA4);//设置整个显示打开/关闭(A4恢复到RAM内容显示,输出遵循RAM内容/A5全屏显示,输出忽略RAM内容)

		/*设置显示偏移量*/
		OLED_WriteByte(OLED_WR_CMD,0xd3);//设置显示偏移量(D3h)
		OLED_WriteByte(OLED_WR_CMD,0x00);

		/* 设置显示时钟分频比/振荡器频率 */
		OLED_WriteByte(OLED_WR_CMD,0xD5);//设置显示时钟分频比/振荡器频率
		OLED_WriteByte(OLED_WR_CMD,0xf0);//设定分割比

		/* 设置预充期 */
		OLED_WriteByte(OLED_WR_CMD,0xD9);//设置预充期
		OLED_WriteByte(OLED_WR_CMD,0x22);

		/* 设置com引脚硬件配置 */
		OLED_WriteByte(OLED_WR_CMD,0xDA);//设置com引脚硬件配置
		OLED_WriteByte(OLED_WR_CMD,0x12);

		/* 设置VCOMH取消选择级别 */
		OLED_WriteByte(OLED_WR_CMD,0xDB);//设置VCOMH取消选择级别
		OLED_WriteByte(OLED_WR_CMD,0x20);//0x20,0.77xVcc

		OLED_WriteByte(OLED_WR_CMD,0x8D);//设置DC-DC使能
		OLED_WriteByte(OLED_WR_CMD,0x14);

		/* 显示打开 */
		OLED_WriteByte(OLED_WR_CMD,0xAF);//设置显示打开/关闭(AFh/AEh)

		OLED_CLS();     // 初始化完成后清屏，确保屏幕全黑，清除残留显示内容
		
		//OLED_FillFull(); // 全屏点亮，便于观察OLED显示是否正常工作

}


/**
  * @brief  显示16x16点阵的单个汉字，页寻址模式，一个汉字占横着的两页（一个page有8字节），竖着的16列
  * @param  y: 页地址（0~7），OLED以8像素为一页
  * @param  x: 列地址（0~127）
  * @param  n: 汉字索引号，在汉字库中的序号
  * @param  data_cn: 指向汉字点阵数据的指针（每个汉字32字节）
  * @retval 无
  */
void OLED_ShowChinese(uint8_t y, uint8_t x, uint8_t n, const uint8_t data_cn[][16])
{
    OLED_SetPos(y, x);  // 设置到第一页位置
    for (uint8_t i = 0; i < 16; i++)  // 写入上半部分 16 字节
    {
        OLED_WriteByte(OLED_WR_DATA, data_cn[n * 2][i]);
    }

    OLED_SetPos(y + 1, x);  // 设置到第二页位置
    for (uint8_t i = 0; i < 16; i++)  // 写入下半部分 16 字节
    {
        OLED_WriteByte(OLED_WR_DATA, data_cn[n * 2 + 1][i]);
    }
}




///**
//  * @brief  显示字符串，8x16字体版      ***对上一个函数封装了一下
//  * @param  line: 行号（0~3），每行16像素高（2页）
//  * @param  offset: 列偏移（0~15）
//  * @param  string_data: 指向字符串的指针
//  * @retval 无
//  */
//void OLED_ShowString_F8X16(uint8_t line, uint8_t offset, uint8_t *string_data)
//{
//    OLED_ShowString(line * 2, offset * 8, string_data, TEXTSIZE_F8X16);
//}


//支持bmp图片显示
/**
  * @brief  OLED 显示位图（支持自动对齐与裁剪）
  * @param  x: 起始列坐标（0~127）
* @param  y: 起始像素行坐标（0~63）、、		***y必须是8的倍数，不然没法用按页写入
  * @param  width: 图像宽度（像素）
  * @param  height: 图像高度（像素）
  * @param  bitmap: 图像数据指针（每列每页排布）
  * @retval 无
  */
void OLED_DrawBitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap)
{
    uint8_t page_start, page_count;
    uint8_t x_end;
    uint32_t raw_size;
    uint32_t j = 0;

    // 起始页处理（每页8行，向上对齐）
    if ((y % 8) != 0)
    {
        y = (y / 8 + 1) * 8;
    }
    page_start = y / 8;

    // 图像页数，非8倍高度自动补页
    page_count = (height + 7) / 8;

    // 列范围裁剪
    if (x >= 128) return;
    if (x + width > 128) width = 128 - x;

    // 行范围裁剪
    if (page_start >= 8) return;
    if (page_start + page_count > 8) page_count = 8 - page_start;

    // 数据总量限制
    raw_size = width * page_count;
    if (raw_size > 1024) raw_size = 1024;

    x_end = x + width;

    // 分页写入图像数据
    for (uint8_t page = 0; page < page_count; page++)
    {
        OLED_SetPos(page_start + page, x);  // 设置页+列起始位置

        for (uint8_t col = x; col < x_end; col++)
        {
            OLED_WriteByte(OLED_WR_DATA, bitmap[j++]);
            if (j >= raw_size)
                return;  // 数据发送完毕
        }
    }
}


/*********************************************END OF FILE**********************/


