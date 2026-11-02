//
// Created by 28715 on 2026/11/2.
//

#include "Joystick.h"
#include <string.h>

/* DMA 接收缓存 */
static uint16_t adc_dma_buffer[JOYSTICK_CH_NUM];

/* 摇杆数据实例 */
static Joystick_Data_t joy_data;

/* 摇杆中点校准值 (12位ADC中点为2048) */
#define JOYSTICK_CENTER     2048
/* 摇杆死区 (防止摇杆在中间时微小抖动产生控制信号) */
#define JOYSTICK_DEADZONE   50
/* 摇杆最大量程 */
#define JOYSTICK_MAX_VAL    4095

/* 内部函数：将 0~4095 的 ADC 值映射到 -1000 ~ 1000 并处理死区 */
static int16_t MapJoystickValue(uint16_t raw)
{
    int32_t val = (int32_t)raw - JOYSTICK_CENTER;

    /* 死区处理 */
    if (val > -JOYSTICK_DEADZONE && val < JOYSTICK_DEADZONE)
    {
        return 0;
    }

    /* 线性映射 */
    if (val > 0)
    {
        // 正向：(val * 1000) / (4095 - 2048)
        return (int16_t)((val * 1000) / (JOYSTICK_MAX_VAL - JOYSTICK_CENTER));
    }
    else
    {
        // 负向：(val * 1000) / 2048
        return (int16_t)((val * 1000) / JOYSTICK_CENTER);
    }
}

/* 初始化并启动 DMA 循环采集 */
void Joystick_Init(void)
{
    memset(&joy_data, 0, sizeof(joy_data));
    memset(adc_dma_buffer, 0, sizeof(adc_dma_buffer));

    /* 启动 ADC DMA 循环采集
     * 注意：CubeMX 中需将 ADC1 的 Scan 模式开启，Continuous 开启，
     * DMA 设置为 Circular (循环模式)，数据宽度为 Half Word (16-bit)
     */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, JOYSTICK_CH_NUM);
}

/* 更新摇杆数据 */
void Joystick_Update(void)
{
    /* 由于开启了 DMA Circular 模式，adc_dma_buffer 中的数据总是在后台自动刷新，
     * 因此这里直接读取即可，完全非阻塞。
     */
    joy_data.raw_x1 = adc_dma_buffer[0];
    joy_data.raw_y1 = adc_dma_buffer[1];
    joy_data.raw_x2 = adc_dma_buffer[2];
    joy_data.raw_y2 = adc_dma_buffer[3];

    /* 转换为 -1000 ~ 1000 的控制量 */
    joy_data.x1 = MapJoystickValue(joy_data.raw_x1);
    joy_data.y1 = MapJoystickValue(joy_data.raw_y1);
    joy_data.x2 = MapJoystickValue(joy_data.raw_x2);
    joy_data.y2 = MapJoystickValue(joy_data.raw_y2);
}

/* 获取最新数据指针 */
Joystick_Data_t* Joystick_GetData(void)
{
    return &joy_data;
}