/*
 * bsp_key.h
 *
 *  Created on: 2026年1月26日
 *      Author: huago
 */

#ifndef KEY_BSP_KEY_H_
#define KEY_BSP_KEY_H_

#include "hal_data.h"

#define KEY_OFF 0
#define KEY_ON 1

// 按键事件类型（核心检测目标）
typedef enum {
    KEY_EVENT_NONE,     // 无事件
    KEY_EVENT_CLICK,    // 单击
    KEY_EVENT_DOUBLE,   // 双击
    KEY_EVENT_LONGPRESS // 长按
} Key_Event_E;

// 时间阈值（可全局调整）
#define KEY_DEBOUNCE_MS        10      // 消抖时间
#define KEY_DOUBLE_INTERVAL_MS 300     // 双击间隔阈值（≤300ms为双击）
#define KEY_LONGPRESS_MS       800     // 长按阈值（≥800ms为长按）

#define KEY0 BSP_IO_PORT_00_PIN_00

#define KEY1 BSP_IO_PORT_07_PIN_00
#define KEY2 BSP_IO_PORT_07_PIN_01
#define KEY3 BSP_IO_PORT_07_PIN_02
#define KEY4 BSP_IO_PORT_07_PIN_03

#define KEY5 BSP_IO_PORT_07_PIN_04
#define KEY6 BSP_IO_PORT_07_PIN_05
#define KEY7 BSP_IO_PORT_07_PIN_06
#define KEY8 BSP_IO_PORT_07_PIN_07

#define KEY9 BSP_IO_PORT_08_PIN_05
#define KEY10 BSP_IO_PORT_08_PIN_06

void Key_Init(void);
Key_Event_E Key_CheckEvent(bsp_io_port_pin_t key_pin);

/*
// 4. 各按键功能处理函数声明（仅注释说明功能，无实际代码）
void Key_Select_Handler(Key_Event_E event);  // KEY1 - SELECT键
void Key_Start_Handler(Key_Event_E event);   // KEY2 - START键
void Key_L1_Handler(Key_Event_E event);      // KEY3 - L1键
void Key_L2_Handler(Key_Event_E event);      // KEY4 - L2键
void Key_R1_Handler(Key_Event_E event);      // KEY5 - R1键
void Key_R2_Handler(Key_Event_E event);      // KEY6 - R2键
void Key_X_Handler(Key_Event_E event);       // KEY7 - X键
void Key_Y_Handler(Key_Event_E event);       // KEY8 - Y键
void Key_A_Handler(Key_Event_E event);       // KEY9 - A键
void Key_B_Handler(Key_Event_E event);       // KEY10 - B键

// 手柄按键任务（主循环调用，扫描所有key1~10并触发对应功能函数）
void Key_HandleTask(void);
*/


#endif /* KEY_BSP_KEY_H_ */
