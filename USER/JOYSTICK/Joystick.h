//
// Created by 28715 on 2026/11/2.
//

#ifndef YVISION_POWER_JOYSTICK_H
#define YVISION_POWER_JOYSTICK_H
#include <stdint.h>
#include "stm32f4xx_hal.h"

/* ADC_HandleTypeDef hadc1; 在 adc.c 中定义 */
extern ADC_HandleTypeDef hadc1;

/* 摇杆通道数量 */
#define JOYSTICK_CH_NUM     4

/* 摇杆数据结构 */
typedef struct
{
    /* 原始 ADC 值 (0 ~ 4095) */
    uint16_t raw_x1;    // 左摇杆 X (PA0)
    uint16_t raw_y1;    // 左摇杆 Y (PA1)
    uint16_t raw_x2;    // 右摇杆 X (PA2)
    uint16_t raw_y2;    // 右摇杆 Y (PA3)

    /* 映射后的控制量 (-1000 ~ 1000，带有死区) */
    int16_t x1;         // 左摇杆 X
    int16_t y1;         // 左摇杆 Y
    int16_t x2;         // 右摇杆 X
    int16_t y2;         // 右摇杆 Y
} Joystick_Data_t;

/* 初始化 ADC DMA 循环采集 */
void Joystick_Init(void);

/* 从 DMA 缓冲区更新并处理摇杆数据，主循环调用 */
void Joystick_Update(void);

/* 获取最新数据指针 */
Joystick_Data_t* Joystick_GetData(void);
#endif //YVISION_POWER_JOYSTICK_H