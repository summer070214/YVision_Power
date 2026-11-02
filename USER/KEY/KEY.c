//
// Created by 28715 on 2026/10/5.
//
#include "main.h"
#include "KEY.h"
#include <stdint.h>
KeyEvent_t key_event = KEY_EVT_NONE;
int BAT_STATE = 0;
/* ===== 状态机内部状态 ===== */
static KeyState_t state  = KS_IDLE;
static uint32_t   ms_cnt = 0;
static uint32_t last_scan;

void Key_timeinit(void) {
    state     = KS_IDLE;
    ms_cnt    = 0;
    key_event = KEY_EVT_NONE;
    BAT_STATE = 0;
    last_scan = HAL_GetTick();
}

void Key_Scan(uint32_t elapsed_ms)
{

    GPIO_PinState level = KEY_READ();

    switch (state) {

    case KS_IDLE:
        ms_cnt = 0;
        if (level == KEY_PRESSED) {
            state = KS_DEBOUNCE_DOWN;
            ms_cnt = 0;
        }
        break;

    case KS_DEBOUNCE_DOWN:
        if (level == KEY_PRESSED) {
            ms_cnt += elapsed_ms;
            if (ms_cnt >= KEY_DEBOUNCE_MS) {
                state = KS_PRESSED;
                ms_cnt = 0;
            }
        }
        else {
            state = KS_IDLE;                 // 毛刺
        }
        break;

    case KS_PRESSED:
        ms_cnt += elapsed_ms;

        if (level == KEY_RELEASED) {
            if (ms_cnt >= KEY_SHORT_MIN_MS && ms_cnt < KEY_SHORT_MAX_MS) {
                // 一次短按完成，完成电量显示，进入等待长按窗口

                state = KS_WAIT_LONG;
                ms_cnt = 0;
            } else {
                state = KS_IDLE;             // 太短/太长，无效
            }
        }
            else if (ms_cnt >= KEY_SHORT_MAX_MS) {
            // 首次按下就超过短按上限：这里选择忽略
            // 若希望首次直接长按也发事件1，把下面改成 g_key_event=KEY_EVT_SHORT;
            state = KS_IDLE;
        }
        break;

    case KS_WAIT_LONG:
        ms_cnt += elapsed_ms;

        if (level == KEY_PRESSED) {          // 窗口内又按下 → 准备识别长按
            state = KS_LONG_DEBOUNCE;
            ms_cnt = 0;
        } else if (ms_cnt >= KEY_WAIT_LONG_MS) {
            // 窗口内没有按下 → 不是长按 → 事件1
            key_event = KEY_EVT_SHORT;
            state = KS_IDLE;
        }
        break;

    case KS_LONG_DEBOUNCE:
        if (level == KEY_PRESSED) {
            ms_cnt += elapsed_ms;
            if (ms_cnt >= KEY_DEBOUNCE_MS) {
                state = KS_LONG_PRESSED;
                ms_cnt = 0;
            }
        } else {
            // 消抖期间释放，视为毛刺，回到等待窗口（窗口重新计时）
            state = KS_WAIT_LONG;
            ms_cnt = 0;
        }
        break;

    case KS_LONG_PRESSED:
        if (level == KEY_PRESSED) {
            ms_cnt += elapsed_ms;
            if (ms_cnt >= KEY_LONG_MS) {
                key_event = KEY_EVT_MULTI_THEN_LONG;   // 事件2
                state = KS_IDLE;
            }
        } else {
            // 按下了，但没到长按时间就松开 → 不是长按 → 事件1
            key_event = KEY_EVT_SHORT;
            state = KS_IDLE;
        }
        break;
    }
}
void Key_event(void) {
    uint32_t now     = HAL_GetTick();
    uint32_t elapsed = now - last_scan;
    if (elapsed > KEY_ELAPSED_MAX_MS) {
        elapsed = KEY_ELAPSED_MAX_MS;
    }
    if (elapsed >= KEY_SCAN_PERIOD_MS) {
        last_scan = now;
        Key_Scan(elapsed);
    }

    if (key_event != KEY_EVT_NONE) {
        KeyEvent_t evt = key_event;
        key_event = KEY_EVT_NONE;

        switch (evt) {
            case KEY_EVT_SHORT:
                event1_pdled();
                break;
            case KEY_EVT_MULTI_THEN_LONG:
                event2_vout();
                break;
            default:
                break;
        }
    }
}
void event1_pdled(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);//EN:IP2366
    // key_event = KEY_EVT_NONE;
}
void event2_vout(void) {
   if (BAT_STATE==0) {
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);//EN:MP2229GQ-Z
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);//EN:KEYLED
       BAT_STATE=1;
   }
   else if (BAT_STATE==1) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);//DISEN:MP2229GQ-Z
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);//DISEN:KEYLED
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);//DISEN:IP2366

        BAT_STATE=0;
    }
    key_event = KEY_EVT_NONE;

}
