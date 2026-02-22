#include "yoloface.h"
#include <stdio.h>
#include "ai_platform.h"
#include "yoloface50k.h"
#include "yoloface50k_data.h"
#include <string.h>

/* Global handle to reference an instantiated C-model */
static ai_handle YOLOFACE50K = AI_HANDLE_NULL;

/* Global c-array to handle the activations buffer */
AI_ALIGNED(32)
static ai_u8 activations[AI_YOLOFACE50K_DATA_ACTIVATIONS_SIZE];
AI_ALIGNED(32)
static ai_u8 in_data[AI_YOLOFACE50K_IN_1_SIZE_BYTES];
AI_ALIGNED(32)
static ai_u8 out_data[AI_YOLOFACE50K_OUT_1_SIZE_BYTES];

/* Array of pointer to manage the model's input/output tensors */
static ai_buffer *ai_input;
static ai_buffer *ai_output;


///* 
// * Bootstrap code 加载模型权重，准备运行时环境
// */
//int aiInit(void) {
//  ai_error err;
//  ai_bool result;
//  ai_network_params params;

//  /* 1 - Create an instance of the model 创建神经网络实例 */
//  err = ai_yoloface50k_create(&YOLOFACE50K, AI_YOLOFACE50K_DATA_CONFIG);
//  if (err.type != AI_ERROR_NONE) {
//    printf("E: AI ai_yoloface50k_create error - type=%d code=%d\r\n", err.type, err.code);
//    return -1;
//  }
//	
////	  /* 2 - Initialize the instance 初始化神经网络参数*/
////   params = AI_NETWORK_PARAMS_INIT(
////      AI_YOLOFACE50K_DATA_WEIGHTS(ai_yoloface50k_data_weights_get()),
////      AI_YOLOFACE50K_DATA_ACTIVATIONS(activations)
////  );

//	

//  /* 2 - 获取网络参数配置 */
//  if (!ai_yoloface50k_data_params_get(&params)) {
//    printf("E: Failed to get data params\n");
//    return -1;
//  }


//  /* 4 - Initialize the instance 初始化神经网络参数 */
//  result = ai_yoloface50k_init(YOLOFACE50K, &params);
//	
//  if (!result) {
//    err = ai_yoloface50k_get_error(YOLOFACE50K);
//    printf("E: AI ai_yoloface50k_init error - type=%d code=%d\r\n", err.type, err.code);
//    return -1;
//  }

//  printf("AI initialized successfully\n");
//  return 0;
//}

///* 
// * Run inference code 推理函数,执行神经网络前向传播
// */
//int aiRun(const void *in_data, void *out_data)
//{
//  ai_i32 n_batch;
//  ai_error err;
//	
//	/*用于管理模型输入/输出张量的指针数组*/
//static ai_buffer *ai_input;
//static ai_buffer *ai_output;
////  ai_buffer ai_input[AI_YOLOFACE50K_IN_NUM];
////  ai_buffer ai_output[AI_YOLOFACE50K_OUT_NUM];


//  /* 2 - Update IO handlers with the data payload 
////  绑定数据*/
////  ai_input[0].n_batches = 1;
////  ai_input[0].data = AI_HANDLE_PTR(in_data);
////  ai_output[0].n_batches = 1;
////  ai_output[0].data = AI_HANDLE_PTR(out_data);

//ai_input = ai_yoloface50k_inputs_get(YOLOFACE50K, NULL);
//  ai_output = ai_yoloface50k_outputs_get(YOLOFACE50K, NULL);
//	ai_input[0].data = AI_HANDLE_PTR(in_data);
//  ai_output[0].data = AI_HANDLE_PTR(out_data);

//  /* 3 - Perform the inference 执行推理 */
//  n_batch = ai_yoloface50k_run(YOLOFACE50K, &ai_input[0], &ai_output[0]);
//  if (n_batch != 1) {
//    err = ai_yoloface50k_get_error(YOLOFACE50K);
//    printf("E: AI ai_yoloface50k_run error - type=%d code=%d\r\n", err.type, err.code);
//    return -1;
//  }
//  // 检查输出数据
//  printf("\n=== Output Data Analysis ===\n");
//  int8_t* out_ptr = (int8_t*)out_data;
//  
//  // 1. 检查是否全为0
//  int zero_count = 0;
//  for(int i = 0; i < 50; i++) {
//    if(out_ptr[i] == 0) zero_count++;
//  }
//  printf("First 50 output values: %d zeros, %d non-zeros\n", 
//         zero_count, 50-zero_count);
//  
//  // 2. 打印前几个值
//  printf("First 20 output values:\n");
//  for(int i = 0; i < 20; i++) {
//    printf("  out[%d] = %d\n", i, out_ptr[i]);
//  }
//  
//  // 3. 检查confidences（位置4, 10, 16, ...）
//  printf("\nConfidence values (should be around -128 to 127):\n");
//  for(int i = 0; i < 3; i++) {
//    int idx = i * 6 + 4;  // 每个检测框的第5个值是confidence
//    printf("  Box %d confidence: %d\n", i, out_ptr[idx]);
//  }
//	
//	
//  return 0;
//}


///* 
// * 初始化函数：补充权重/激活缓冲区的size赋值
// */
//int aiInit(void) {
//    ai_error err;

//    /* 1 - 创建模型实例 */
//    err = ai_yoloface50k_create(&YOLOFACE50K, NULL);
//    if (err.type != AI_ERROR_NONE) {
//        printf("E: yoloface50k create error - %d:%d\r\n", err.type, err.code);
//        return -1;
//    }

//    /* 2 - 初始化参数：补充size字段（关键修复） */
//    ai_network_params params = {0};
//    // 权重缓冲区：data+size（缺一不可）
//    params.params.data = ai_yoloface50k_data_weights_get();
//    params.params.size = AI_YOLOFACE50K_DATA_WEIGHTS_SIZE; // 权重字节数
//    // 激活缓冲区：data+size（缺一不可）
//    params.activations.data = AI_HANDLE_PTR(activations);
//    params.activations.size = AI_YOLOFACE50K_DATA_ACTIVATIONS_SIZE; // 激活字节数

//    /* 3 - 初始化模型 */
//    if (!ai_yoloface50k_init(YOLOFACE50K, &params)) {
//        err = ai_yoloface50k_get_error(YOLOFACE50K);
//        printf("E: yoloface50k init error - %d:%d\r\n", err.type, err.code);
//        ai_yoloface50k_destroy(YOLOFACE50K);
//        YOLOFACE50K = AI_HANDLE_NULL;
//        return -1;
//    }

//    printf("I: yoloface50k init OK\r\n");
//    return 0;
//}

///* 
// * 推理函数：修复size（字节数）+补充format字段
// */
//int aiRun(const void *in_data, void *out_data)
//{
//    ai_i32 n_batch;
//    ai_error err;

//    /* 1 - 初始化IO缓冲区 */
//    ai_buffer ai_input[AI_YOLOFACE50K_IN_NUM] = {0};
//    ai_buffer ai_output[AI_YOLOFACE50K_OUT_NUM] = {0};

//    /* 2 - 绑定输入数据（核心修复：size用字节数+补充format） */
//    ai_input[0].data = AI_HANDLE_PTR(in_data);
//    ai_input[0].size = AI_YOLOFACE50K_IN_1_SIZE_BYTES; // 输入字节数（9409）
//    ai_input[0].format = AI_YOLOFACE50K_IN_1_FORMAT;   // 输入格式（8651840）

//    /* 3 - 绑定输出数据 */
//    ai_output[0].data = AI_HANDLE_PTR(out_data);
//    ai_output[0].size = AI_YOLOFACE50K_OUT_1_SIZE_BYTES; // 输出字节数（882）
//    ai_output[0].format = AI_YOLOFACE50K_OUT_1_FORMAT;   // 输出格式（补充）

//    /* 4 - 执行推理 */
//    n_batch = ai_yoloface50k_run(YOLOFACE50K, ai_input, ai_output);
//    if (n_batch != 1) {
//        err = ai_yoloface50k_get_error(YOLOFACE50K);
//        printf("E: yoloface50k run error - %d:%d\r\n", err.type, err.code);
//        return -1;
//    }
//    
//    return 0;
//}

///* 
// * 销毁函数
// */
//void aiDeinit(void) {
//    if (YOLOFACE50K != AI_HANDLE_NULL) {
//        ai_yoloface50k_destroy(YOLOFACE50K);
//        YOLOFACE50K = AI_HANDLE_NULL;
//        printf("I: yoloface50k deinit OK\r\n");
//    }
//}


/* 
 * Bootstrap
 */
int aiInit(void) {
  ai_error err;
  
  /* Create and initialize the c-model */
  const ai_handle acts[] = { activations };
  err = ai_yoloface50k_create_and_init(&YOLOFACE50K, acts, NULL);
  if (err.type != AI_ERROR_NONE) { 
	printf("E: yoloface50k create error - %d:%d\r\n", err.type, err.code);
        return -1;};

  /* Reteive pointers to the model's input/output tensors */
  ai_input = ai_yoloface50k_inputs_get(YOLOFACE50K, NULL);
  ai_output = ai_yoloface50k_outputs_get(YOLOFACE50K, NULL);

  return 0;
}

/* 
 * Run inference
 */
int aiRun(const void *in_data, void *out_data) {
  ai_i32 n_batch;
  ai_error err;
  
  /* 1 - Update IO handlers with the data payload */
  ai_input[0].data = AI_HANDLE_PTR(in_data);
  ai_output[0].data = AI_HANDLE_PTR(out_data);

  /* 2 - Perform the inference */
  n_batch = ai_yoloface50k_run(YOLOFACE50K, &ai_input[0], &ai_output[0]);
  if (n_batch != 1) {
      err = ai_yoloface50k_get_error(YOLOFACE50K);
      printf("W: run warning - %d:%d (n_batch=%d)\r\n", err.type, err.code, n_batch);
  };
  
  return 0;
}

void aiDeinit(void) {
    if (YOLOFACE50K != AI_HANDLE_NULL) {
        ai_yoloface50k_destroy(YOLOFACE50K);
        YOLOFACE50K = AI_HANDLE_NULL;
        ai_input = NULL;
        ai_output = NULL;
    }
    printf("I: yoloface deinit OK\r\n");
}
