#include "main.h"
#include "yoloface.h"
#include <math.h> 
#include <stdio.h>
#include "ai_platform_interface.h"
#include "yoloface50k.h"
#include "yoloface50k_data.h"
#include "ff.h"
////原来放在main.c里，会报错，移到这里来
///**
//  * @brief  读取BMP并转换为56x56 RGB565格式（直接适配你的代码）
//  * @param  filename: BMP文件名
//  * @param  image_data: 输出数组，必须是56 * 56的uint16_t数组
//  * @retval FRESULT状态
//  */
//FRESULT Read_BMP_For_YOLO_56x56(const char* filename, uint16_t* image_data)
//{
//    FIL file;
//    FRESULT res = f_open(&file, filename, FA_READ);
//    if(res != FR_OK) return res;
//    
//    // 1. 读取文件头
//    uint8_t header[54];
//    UINT br;
//    res = f_read(&file, header, 54, &br);
//    if(res != FR_OK || br != 54) {
//        f_close(&file);
//        return FR_INT_ERR;
//    }
//    
//    // 2. 验证BMP
//    uint16_t type = *(uint16_t*)header;
//    if(type != 0x4D42) {
//        f_close(&file);
//        return FR_NO_FILE;
//    }
//    
//    // 3. 获取尺寸
//    int32_t bmp_width = *(int32_t*)(header + 18);
//    int32_t bmp_height = *(int32_t*)(header + 22);
//    uint16_t bpp = *(uint16_t*)(header + 28);
//    uint32_t offset = *(uint32_t*)(header + 10);
//    
//    printf("BMP: %dx%d -> Resizing to 56x56\r\n", 
//           (int)bmp_width, (int)bmp_height);
//    
//    if(bpp != 24) {
//        f_close(&file);
//        return FR_INVALID_PARAMETER;
//    }
//    
//    // 4. 读取像素数据
//    uint32_t bmp_rowSize = ((bmp_width * 3 + 3) / 4) * 4;
//    uint8_t* bmp_buffer = (uint8_t*)malloc(bmp_rowSize * bmp_height);
//    
//    f_lseek(&file, offset);
//    f_read(&file, bmp_buffer, bmp_rowSize * bmp_height, &br);
//    f_close(&file);
//    
//    // 5. 转换并缩放到56x56 RGB565
//    // 简化的最近邻插值缩放
//    for(int y = 0; y < 56; y++) {
//        // 计算BMP中的对应行（翻转Y坐标）
//        int src_y = (bmp_height - 1) - (y * bmp_height / 56);
//        if(src_y < 0) src_y = 0;
//        if(src_y >= bmp_height) src_y = bmp_height - 1;
//        
//        uint8_t* src_row = bmp_buffer + src_y * bmp_rowSize;
//        
//        for(int x = 0; x < 56; x++) {
//            // 计算BMP中的对应列
//            int src_x = x * bmp_width / 56;
//            if(src_x >= bmp_width) src_x = bmp_width - 1;
//            
//            // 获取BGR
//            uint8_t b = src_row[src_x * 3];
//            uint8_t g = src_row[src_x * 3 + 1];
//            uint8_t r = src_row[src_x * 3 + 2];
//            
//            // 转换为RGB565（和你代码中的格式一致）
//            uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
//            
//            // 存储到image_data（注意你的存储顺序是先行后列）
//            image_data[x + y * 56] = color;
//        }
//    }
//    
//    free(bmp_buffer);
//    printf("Image resized to 56x56 RGB565\r\n");
//    return FR_OK;
//}


//// 定义网络输入数组
//// 网络输入：56x56x3 int8 (预处理后)
//AI_ALIGNED(32)
//static ai_i8 in_data[9408];// 56 * 56 * 3=9408字节

//// 定义网络输出数组
//// 网络输出：7x7x18 int8
//AI_ALIGNED(32)
//static ai_i8 out_data[882];// 7 * 7 * 18=882字节
//// 定义图像数组（和你的代码一致）
//uint16_t image_data[56 * 56] = {0};  // 56x56 RGB565

///**
//  * @brief  从BMP文件获取图像（替代GetImage函数）
//  * @param  image: 输出图像数组
//  */
//void GetImage_From_BMP(uint16_t* image, const char* filename)
//{
//    FRESULT res = Read_BMP_For_YOLO_56x56(filename, image);
//    
//    if(res == FR_OK) {
//        printf("Image loaded from BMP successfully\r\n");
//    } else {
//        printf("Failed to load image: %d\r\n", res);
//        // 填充测试图案
//        for(int i = 0; i < 56 * 56; i++) {
//            image[i] = 0x0000;  // 黑色
//        }
//    }
//}
//// 人脸方框的左上右下像素坐标
//int x1, y1, x2, y2;
//// yoloface的anchor尺寸
//uint8_t anchors[3][2] = {{9, 14}, {12, 17}, {22, 21}};

//// 网络预处理函数（完全保持你的原样）
//void prepare_yolo_data()
//{
//    for(int i = 0; i < 56; i++) {
//        for(int j = 0; j < 56; j++) {
//            uint16_t color = image_data[j + i * 56];  // 你的存储顺序
//            
//            // 提取R、G、B并减去128（和你的代码一致）
//            in_data[(j + i * 56) * 3]     = (int8_t)((color & 0xF800) >> 9) - 128;  // R
//            in_data[(j + i * 56) * 3 + 1] = (int8_t)((color & 0x07E0) >> 3) - 128;  // G
//            in_data[(j + i * 56) * 3 + 2] = (int8_t)((color & 0x001F) << 3) - 128;  // B
//        }
//    }
//}

//// 定义sigmoid函数
//float sigmoid(float x)
//{
//    return 1.0f / (1.0f + expf(-x));
//}

//// 网络后处理函数（保持你的原样）
//void post_process()
//{
//    int grid_x, grid_y;
//    float x, y, w, h;
//    
//    for(int i = 0; i < 49; i++) {
//        for(int j = 0; j < 3; j++) {
//            int8_t conf = out_data[i * 18 + j * 6 + 4];
//            
//            if(conf > -9) {
//                grid_x = i % 7;
//                grid_y = (i - grid_x) / 7;
//                
//                x = ((float)out_data[i * 18 + j * 6] + 15) * 0.14218327403068542f;
//                y = ((float)out_data[i * 18 + j * 6 + 1] + 15) * 0.14218327403068542f;
//                w = ((float)out_data[i * 18 + j * 6 + 2] + 15) * 0.14218327403068542f;
//                h = ((float)out_data[i * 18 + j * 6 + 3] + 15) * 0.14218327403068542f;
//                
//                x = (sigmoid(x) + grid_x) * 8;
//                y = (sigmoid(y) + grid_y) * 8;
//                w = expf(w) * anchors[j][0];
//                h = expf(h) * anchors[j][1];
//                
//                x1 = (int)(x - w / 2);
//                y1 = (int)(y - h / 2);
//                x2 = (int)(x + w / 2);
//                y2 = (int)(y + h / 2);
//                
//                if(x1 < 0) x1 = 0;
//                if(y1 < 0) y1 = 0;
//                if(x2 > 55) x2 = 55;
//                if(y2 > 55) y2 = 55;
//                
//                // 绘制检测框
//                printf("Face detected: [%d,%d] to [%d,%d]\r\n", x1, y1, x2, y2);
//                // GUI_Rectangle(x1, y1, x2, y2, RED);
//            }
//        }
//    }
//}
