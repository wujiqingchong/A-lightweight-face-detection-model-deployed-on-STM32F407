/**
  ******************************************************************************
  * @file    yoloface50k_data_params.h
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-01-24T14:35:00+0800
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#ifndef YOLOFACE50K_DATA_PARAMS_H
#define YOLOFACE50K_DATA_PARAMS_H

#include "ai_platform.h"

/*
#define AI_YOLOFACE50K_DATA_WEIGHTS_PARAMS \
  (AI_HANDLE_PTR(&ai_yoloface50k_data_weights_params[1]))
*/

#define AI_YOLOFACE50K_DATA_CONFIG               (NULL)


#define AI_YOLOFACE50K_DATA_ACTIVATIONS_SIZES \
  { 30172, }
#define AI_YOLOFACE50K_DATA_ACTIVATIONS_SIZE     (30172)
#define AI_YOLOFACE50K_DATA_ACTIVATIONS_COUNT    (1)
#define AI_YOLOFACE50K_DATA_ACTIVATION_1_SIZE    (30172)



#define AI_YOLOFACE50K_DATA_WEIGHTS_SIZES \
  { 11304, }
#define AI_YOLOFACE50K_DATA_WEIGHTS_SIZE         (11304)
#define AI_YOLOFACE50K_DATA_WEIGHTS_COUNT        (1)
#define AI_YOLOFACE50K_DATA_WEIGHT_1_SIZE        (11304)



#define AI_YOLOFACE50K_DATA_ACTIVATIONS_TABLE_GET() \
  (&g_yoloface50k_activations_table[1])

extern ai_handle g_yoloface50k_activations_table[1 + 2];



#define AI_YOLOFACE50K_DATA_WEIGHTS_TABLE_GET() \
  (&g_yoloface50k_weights_table[1])

extern ai_handle g_yoloface50k_weights_table[1 + 2];


#endif    /* YOLOFACE50K_DATA_PARAMS_H */
