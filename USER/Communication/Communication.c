#include "Communication.h"
#include "stm32f4xx_hal_i2c.h"
#include <string.h>

/* DMA 接收缓存 */
static uint8_t rx_buffer[2];

/* 当前读取的命令，回调中会使用 */
static volatile uint8_t current_command;

/* 数据结构 */
static BQ28Z610_Data_t bq_data;

/* 状态标志 */
static volatile uint8_t dma_ok = 0;
static volatile BQ28Z610_Status_t bq_status = BQ28Z610_OK;

/* 轮询命令表 */
static const uint8_t bq_cmd_table[] =
{
    BQ28Z610_CMD_TEMPERATURE,
    BQ28Z610_CMD_VOLTAGE,
    BQ28Z610_CMD_CURRENT,
    BQ28Z610_CMD_REMAINING_CAPACITY,
    BQ28Z610_CMD_FULL_CHARGE_CAP,
    BQ28Z610_CMD_AVERAGE_CURRENT,
    BQ28Z610_CMD_CYCLE_COUNT,
    BQ28Z610_CMD_RSOC,
    BQ28Z610_CMD_SOH
};

#define BQ_CMD_COUNT    (sizeof(bq_cmd_table) / sizeof(bq_cmd_table[0]))

static uint8_t bq_cmd_index = 0;

/* 初始化 */
void BQ28Z610_Init(void)
{
    memset(&bq_data, 0, sizeof(bq_data));
    dma_ok = 0;
    bq_status = BQ28Z610_OK;
    current_command = 0;
    bq_cmd_index = 0;
}

/* DMA 异步读取 16 位寄存器 */
BQ28Z610_Status_t BQ28Z610_ReadWord_DMA(uint8_t command)
{
    if (bq_status == BQ28Z610_BUSY)
    {
        return BQ28Z610_BUSY;
    }

    dma_ok = 0;
    current_command = command;
    bq_status = BQ28Z610_BUSY;

    HAL_StatusTypeDef ret;
    ret = HAL_I2C_Mem_Read_DMA(
        &hi2c1,
        BQ28Z610_I2C_ADDR,
        command,
        I2C_MEMADD_SIZE_8BIT,
        rx_buffer,
        2
    );

    if (ret != HAL_OK)
    {
        bq_status = BQ28Z610_ERROR;
        return BQ28Z610_ERROR;
    }

    return BQ28Z610_OK;
}

/* DMA 接收完成回调，由 HAL_I2C_MemRxCpltCallback 转发调用 */
void BQ28Z610_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    uint16_t value;

    if (hi2c != &hi2c1)
    {
        return;
    }

    /* BQ28Z610 数据低字节在前 */
    value = (uint16_t)rx_buffer[0]
          | ((uint16_t)rx_buffer[1] << 8);

    switch (current_command)
    {
        case BQ28Z610_CMD_TEMPERATURE:
            bq_data.temperature = value;
            break;

        case BQ28Z610_CMD_VOLTAGE:
            bq_data.voltage = value;
            break;

        case BQ28Z610_CMD_CURRENT:
            bq_data.current = (int16_t)value;
            break;

        case BQ28Z610_CMD_AVERAGE_CURRENT:
            bq_data.average_current = (int16_t)value;
            break;

        case BQ28Z610_CMD_REMAINING_CAPACITY:
            bq_data.remaining_capacity = value;
            break;

        case BQ28Z610_CMD_FULL_CHARGE_CAP:
            bq_data.full_charge_capacity = value;
            break;

        case BQ28Z610_CMD_CYCLE_COUNT:
            bq_data.cycle_count = value;
            break;

        case BQ28Z610_CMD_RSOC:
            bq_data.rsoc = value;
            break;

        case BQ28Z610_CMD_SOH:
            bq_data.soh = value;
            break;

        default:
            break;
    }

    bq_status = BQ28Z610_OK;
    dma_ok = 1;
}

/* I2C 错误回调，由 HAL_I2C_ErrorCallback 转发调用 */
void BQ28Z610_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c != &hi2c1)
    {
        return;
    }

    bq_status = BQ28Z610_ERROR;
    dma_ok = 1;
}

/*
 * 非阻塞状态机：
 * 主循环周期调用，每次只启动一条命令；
 * DMA 完成后由回调置 dma_ok，下一次调用推进到下一命令。
 */
void BQ28Z610_GetData(void)
{
    /* 正在传输，直接返回 */
    if (bq_status == BQ28Z610_BUSY)
    {
        return;
    }

    /* 错误恢复：重试当前命令，不推进索引 */
    if (bq_status == BQ28Z610_ERROR)
    {
        dma_ok = 0;
        bq_status = BQ28Z610_OK;

        (void)BQ28Z610_ReadWord_DMA(bq_cmd_table[bq_cmd_index]);
        return;
    }

    /* 上一条命令完成，推进到下一条 */
    if (dma_ok)
    {
        dma_ok = 0;
        bq_cmd_index++;

        if (bq_cmd_index >= BQ_CMD_COUNT)
        {
            bq_cmd_index = 0;
        }
    }

    /* 启动当前命令 */
    (void)BQ28Z610_ReadWord_DMA(bq_cmd_table[bq_cmd_index]);
}

/* DMA 是否完成 */
uint8_t BQ28Z610_IsReady(void)
{
    return dma_ok;
}

/* 获取读取状态 */
BQ28Z610_Status_t BQ28Z610_GetStatus(void)
{
    return bq_status;
}

/* 清除完成标志 */
void BQ28Z610_ClearReady(void)
{
    dma_ok = 0;
}