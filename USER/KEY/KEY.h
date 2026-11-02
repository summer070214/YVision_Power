//
// Created by 28715 on 2026/10/5.
//

#ifndef YVISION_POWER_KEY_H
#define YVISION_POWER_KEY_H
#include <stdint.h>
#include "main.h"
//硬件定义
#define KEY_PIN        GPIO_PIN_9
#define KEY_PORT       GPIOB
#define KEY_PRESSED    GPIO_PIN_RESET      // 按下为低
#define KEY_RELEASED   GPIO_PIN_SET
#define KEY_READ()     HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN)
//按键参数
#define KEY_DEBOUNCE_MS       20     // 消抖时间
#define KEY_SHORT_MIN_MS      50     // 最短有效短按（滤掉极短毛刺）
#define KEY_SHORT_MAX_MS      500    // 超过这个时间不算短按
#define KEY_LONG_MS           1500   // 长按判定阈值
#define KEY_WAIT_LONG_MS      800    // 短按释放后，等待“再按下并长按”的窗口
#define KEY_SCAN_PERIOD_MS    5      // Key_Scan 的调用周期
#define KEY_ELAPSED_MAX_MS   50     // 单次最多按 50ms 计


typedef enum {
    KEY_EVT_NONE = 0,
    KEY_EVT_SHORT,          // 事件 1：单次短按
    KEY_EVT_MULTI_THEN_LONG // 事件 2：连续短按 1 次后再长按
} KeyEvent_t;

typedef enum {
    KS_IDLE = 0,            // 空闲，等待首次按下
    KS_DEBOUNCE_DOWN,       // 首次按下消抖
    KS_PRESSED,             // 首次按下确认，区分短按
    KS_WAIT_LONG,           // 短按释放后，等待长按按下
    KS_LONG_DEBOUNCE,       // 长按按下消抖
    KS_LONG_PRESSED         // 长按计时中
} KeyState_t;
void Key_timeinit(void);
void Key_Scan(uint32_t elapsed_ms);
void Key_event(void);
void event1_pdled(void);
void event2_vout(void);

extern  KeyEvent_t key_event;
extern  int BAT_STATE;

#endif //YVISION_POWER_KEY_H