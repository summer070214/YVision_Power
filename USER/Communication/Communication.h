//
// Created by 28715 on 2026/10/5.
//

#ifndef YVISION_POWER_COMUNICATION_H
#define YVISION_POWER_COMUNICATION_H
#include <stdint.h>
#include "stm32f4xx_hal.h"

/* I2C_HandleTypeDef hi2c1; 在 main.c 中定义 */
extern I2C_HandleTypeDef hi2c1;

/* BQ28Z610 I2C 7-bit 地址为 0x55
 * STM32 HAL 需要左移 1 位
 */
#define BQ28Z610_I2C_ADDR        (0x55 << 1)

/* Standard Data Commands */
#define BQ28Z610_CMD_TEMPERATURE         0x06
#define BQ28Z610_CMD_VOLTAGE             0x08
#define BQ28Z610_CMD_CURRENT             0x0C
#define BQ28Z610_CMD_REMAINING_CAPACITY  0x10
#define BQ28Z610_CMD_FULL_CHARGE_CAP     0x12
#define BQ28Z610_CMD_AVERAGE_CURRENT     0x14
#define BQ28Z610_CMD_CYCLE_COUNT         0x2A
#define BQ28Z610_CMD_RSOC                0x2C
#define BQ28Z610_CMD_SOH                 0x2E

typedef struct
{
    uint16_t temperature;          // 温度，单位：0.1 K（开尔文）
    uint16_t voltage;              // 电池电压，单位：mV（毫伏）

    int16_t current;                // 当前电流，单位：mA（毫安）
    int16_t average_current;        // 平均电流，单位：mA（毫安）

    uint16_t remaining_capacity;     // 剩余容量，单位：mAh（毫安时）
    uint16_t full_charge_capacity;   // 满充容量，单位：mAh（毫安时）

    uint16_t cycle_count;            // 电池循环次数，单位：次
    uint16_t rsoc;                   // 剩余电量百分比，单位：%

    uint16_t soh;                    // 电池健康度，单位：%
} BQ28Z610_Data_t;

typedef enum
{
    BQ28Z610_OK = 0,
    BQ28Z610_BUSY,
    BQ28Z610_ERROR
} BQ28Z610_Status_t;

/* 初始化 */
void BQ28Z610_Init(void);

/* DMA 异步读取一个 16 位寄存器 */
BQ28Z610_Status_t BQ28Z610_ReadWord_DMA(uint8_t command);

/* DMA 接收完成回调
 * 需要在 HAL_I2C_MemRxCpltCallback 中调用
 */
void BQ28Z610_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);

/* I2C 错误回调 */
void BQ28Z610_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);

/* 非阻塞状态机推进，主循环中周期调用 */
void BQ28Z610_GetData(void);

/* DMA 是否读取完成 */
uint8_t BQ28Z610_IsReady(void);

/* 获取最近一次读取状态 */
BQ28Z610_Status_t BQ28Z610_GetStatus(void);

/* 清除完成标志 */
void BQ28Z610_ClearReady(void);
#endif //YVISION_POWER_COMUNICATION_H