/*
 * bsp_key.c
 *
 *  Created on: 2026年1月26日
 *      Author: huago
 */

#include "bsp_key.h"
#include "UART/uart.h"

void Key_Init(void)
{
    R_IOPORT_Open(&g_ioport_ctrl, g_ioport.p_cfg);
}


typedef struct {
    bsp_io_port_pin_t pin;                // 按键引脚（KEY1~10）
    uint32_t press_start_ms;              // 按下起始时间（ms）
    uint32_t release_ms;                  // 松开时间（ms）
    uint8_t  click_cnt;                   // 点击次数（双击检测）
    uint8_t  longpress_flag;              // 长按触发标志（避免重复）
    uint8_t  last_state;                  // 上一次物理状态（按下/松开）
} Key_State_S;

// 2. key1~10状态数组（与PCB按键一一对应）
static Key_State_S g_key_state[] = {
    {KEY1, 0, 0, 0, 0, KEY_OFF},  // SELECT
    {KEY2, 0, 0, 0, 0, KEY_OFF},  // START
    {KEY3, 0, 0, 0, 0, KEY_OFF},  // L1
    {KEY4, 0, 0, 0, 0, KEY_OFF},  // L2
    {KEY5, 0, 0, 0, 0, KEY_OFF},  // R1
    {KEY6, 0, 0, 0, 0, KEY_OFF},  // R2
    {KEY7, 0, 0, 0, 0, KEY_OFF},  // X
    {KEY8, 0, 0, 0, 0, KEY_OFF},  // Y
    {KEY9, 0, 0, 0, 0, KEY_OFF},  // A
    {KEY10, 0, 0, 0, 0, KEY_OFF}  // B
};

// 3. 系统毫秒级计时函数（需适配瑞萨定时器，示例框架）
static uint32_t Key_GetSysMs(void)
{
    // 替换为实际的瑞萨定时器毫秒计数（如GPT定时器）
    // 示例：return g_timer_ms; // 全局毫秒计数器
    static uint32_t ms = 0;
    ms++;
    return ms;
}

// 4. 基础消抖扫描函数（获取按键物理状态）
static uint8_t Key_Scan(bsp_io_port_pin_t key_pin)
{
    bsp_io_level_t level;
    uint8_t state = KEY_OFF;

    // 第一次读取电平
    R_IOPORT_PinRead(&g_ioport_ctrl, key_pin, &level);
    // 消抖延时
    R_BSP_SoftwareDelay(KEY_DEBOUNCE_MS, BSP_DELAY_UNITS_MILLISECONDS);
    // 第二次读取电平（确认稳定状态）
    R_IOPORT_PinRead(&g_ioport_ctrl, key_pin, &level);

    // 按下=低电平，松开=高电平（适配PCB硬件设计）
    if (BSP_IO_LEVEL_LOW == level) {
        state = KEY_ON;
    }
    return state;
}


// 6. 通用按键事件检测函数（核心：检测任意按键的单击/双击/长按）
Key_Event_E Key_CheckEvent(bsp_io_port_pin_t key_pin)
{
    Key_Event_E event = KEY_EVENT_NONE;
    uint8_t curr_state;
    uint32_t curr_ms = Key_GetSysMs();
    uint8_t i;

    // 找到对应按键的状态结构体
    for (i = 0; i < 10; i++) {
        if (g_key_state[i].pin == key_pin) {
            break;
        }
    }
    if (i >= 10) return KEY_EVENT_NONE;  // 无效按键

    // 步骤1：获取当前按键物理状态
    curr_state = Key_Scan(key_pin);

    // 步骤2：状态机处理（区分按下/持续按下/松开）
    if (curr_state == KEY_ON && g_key_state[i].last_state == KEY_OFF)
    {
        // 按键按下：记录起始时间，重置标志
        g_key_state[i].press_start_ms = curr_ms;
        g_key_state[i].longpress_flag = 0;
    }
    else if (curr_state == KEY_ON && g_key_state[i].last_state == KEY_ON)
    {
        // 按键持续按下：检测长按
        if ((curr_ms - g_key_state[i].press_start_ms >= KEY_LONGPRESS_MS)
            && (g_key_state[i].longpress_flag == 0))
        {
            event = KEY_EVENT_LONGPRESS;
            uart_printf(UART_PORT_4, "long");
            g_key_state[i].longpress_flag = 1;  // 标记长按已触发
            g_key_state[i].click_cnt = 0;       // 长按后取消双击检测
        }
    }
    else if (curr_state == KEY_OFF && g_key_state[i].last_state == KEY_ON)
    {
        // 按键松开：记录松开时间，累计点击次数
        g_key_state[i].release_ms = curr_ms;
        if (g_key_state[i].longpress_flag == 0)
        {  // 未触发长按才检测单击/双击
            g_key_state[i].click_cnt++;
        }
    }
    else if (curr_state == KEY_OFF && g_key_state[i].last_state == KEY_ON)
    {
        // 按键松开后：检测单击/双击（超过双击间隔则判定为单击）
        if (g_key_state[i].click_cnt == 1)
        {
            if (curr_ms - g_key_state[i].release_ms > KEY_DOUBLE_INTERVAL_MS)
            {
                event = KEY_EVENT_CLICK;
                uart_printf(UART_PORT_4, "single");
                g_key_state[i].click_cnt = 0;
            }
        }
        else if (g_key_state[i].click_cnt == 2)
        {
            // 两次点击间隔≤阈值：判定为双击
            if (curr_ms - g_key_state[i].release_ms <= KEY_DOUBLE_INTERVAL_MS)
            {
                event = KEY_EVENT_DOUBLE;
                uart_printf(UART_PORT_4, "double");
                g_key_state[i].click_cnt = 0;
            }
        }
    }

    // 更新上一次状态
    g_key_state[i].last_state = curr_state;
    return event;
}

/*
// 7. 各按键功能处理函数（仅注释说明功能，无实际代码）
void Key_Select_Handler(Key_Event_E event)
{
    switch (event) {
        case KEY_EVENT_LONGPRESS:
            // 非飞行状态：长按进入无效高度模式
            // 飞行状态：无特殊动作（同非飞行）
            break;
        default:
            break;
    }
}

void Key_Start_Handler(Key_Event_E event)
{
    switch (event) {
        case KEY_EVENT_CLICK:
        case KEY_EVENT_DOUBLE:
            // 非飞行状态：单击/双击停止飞行
            // 飞行状态：同非飞行
            break;
        case KEY_EVENT_LONGPRESS:
            // 非飞行状态：长按启动飞行
            // 飞行状态：同非飞行
            break;
        default:
            break;
    }
}

void Key_L1_Handler(Key_Event_E event)
{
    switch (event) {
        case KEY_EVENT_LONGPRESS:
            // 非飞行状态：长按切换无头模式
            // 飞行状态：无特殊动作
            break;
        default:
            break;
    }
}

void Key_L2_Handler(Key_Event_E event)
{
    // 预留：L2键无明确映射功能，可扩展
}

void Key_R1_Handler(Key_Event_E event)
{
    // 预留：R1键无明确映射功能，可扩展
}

void Key_R2_Handler(Key_Event_E event)
{
    switch (event) {
        case KEY_EVENT_LONGPRESS:
            // 非飞行状态：长按系统复位
            // 飞行状态：无特殊动作
            break;
        default:
            break;
    }
}

void Key_X_Handler(Key_Event_E event)
{
    // 预留：X键可映射为“上下左右键/1/2/3/4键”相关（调整偏航/电机测试）
    switch (event) {
        case KEY_EVENT_CLICK:
        case KEY_EVENT_DOUBLE:
            // 非飞行状态：单击/双击调整偏航角度 / 对应电机测试（启停/参数）
            // 飞行状态：单击/双击设置LC307目标位置
            break;
        default:
            break;
    }
}

void Key_Y_Handler(Key_Event_E event)
{
    // 预留：Y键可映射为“上下左右键/1/2/3/4键”相关（调整偏航/电机测试）
    switch (event) {
        case KEY_EVENT_CLICK:
        case KEY_EVENT_DOUBLE:
            // 非飞行状态：单击/双击调整偏航角度 / 对应电机测试（启停/参数）
            // 飞行状态：单击/双击设置LC307目标位置
            break;
        default:
            break;
    }
}

void Key_A_Handler(Key_Event_E event)
{
    // 预留：A键可映射为“L摇杆”相关
    switch (event) {
        case KEY_EVENT_LONGPRESS:
            // 非飞行状态：长按保存偏航角度参数
            // 飞行状态：长按关闭避障
            break;
        case KEY_EVENT_CLICK:
        case KEY_EVENT_DOUBLE:
            // 飞行状态：单击/双击切换避障模式
            break;
        default:
            break;
    }
}

void Key_B_Handler(Key_Event_E event)
{
    // 预留：B键可映射为“R摇杆”相关
    switch (event) {
        case KEY_EVENT_LONGPRESS:
            // 非飞行状态：长按进入电机测试模式
            break;
        default:
            break;
    }
}

// 8. 手柄按键总任务（主循环调用，扫描所有key1~10）
void Key_HandleTask(void)
{
    Key_Event_E event;
    // 遍历key1~10，检测事件并触发对应功能函数
    event = Key_CheckEvent(KEY1);
    if (event != KEY_EVENT_NONE) Key_Select_Handler(event);

    event = Key_CheckEvent(KEY2);
    if (event != KEY_EVENT_NONE) Key_Start_Handler(event);

    event = Key_CheckEvent(KEY3);
    if (event != KEY_EVENT_NONE) Key_L1_Handler(event);

    event = Key_CheckEvent(KEY4);
    if (event != KEY_EVENT_NONE) Key_L2_Handler(event);

    event = Key_CheckEvent(KEY5);
    if (event != KEY_EVENT_NONE) Key_R1_Handler(event);

    event = Key_CheckEvent(KEY6);
    if (event != KEY_EVENT_NONE) Key_R2_Handler(event);

    event = Key_CheckEvent(KEY7);
    if (event != KEY_EVENT_NONE) Key_X_Handler(event);

    event = Key_CheckEvent(KEY8);
    if (event != KEY_EVENT_NONE) Key_Y_Handler(event);

    event = Key_CheckEvent(KEY9);
    if (event != KEY_EVENT_NONE) Key_A_Handler(event);

    event = Key_CheckEvent(KEY10);
    if (event != KEY_EVENT_NONE) Key_B_Handler(event);
}

*/




/*static uint32_t key_prev_state = KEY_OFF;
 *
 * uint32_t Key_Scan(bsp_io_port_pin_t key)
{
    bsp_io_level_t curr_level;
    uint32_t curr_state = KEY_OFF;

    // 1. 读取当前GPIO电平
    R_IOPORT_PinRead(&g_ioport_ctrl, key, &curr_level);

    // 2. 消抖处理：先延时20ms，等待触点抖动结束
    R_BSP_SoftwareDelay(KEY_DEBOUNCE_MS, BSP_DELAY_UNITS_MILLISECONDS);
    // 再次读取电平，确认稳定状态
    R_IOPORT_PinRead(&g_ioport_ctrl, key, &curr_level);

    // 3. 判断按键稳定状态（假设你的按键是“按下低电平，松开高电平”）
    if(BSP_IO_LEVEL_LOW == curr_level)
    {
        curr_state = KEY_ON;  // 消抖后确认按键按下
    }

    else
    {
        curr_state = KEY_OFF; // 消抖后确认按键未按下/松开
    }

    // 4. 仅在状态变化时打印（减少串口占用，避免卡顿）
    if(curr_state != key_prev_state)
    {
        uart_printf(UART_PORT_4, curr_state == KEY_ON ? " 1 " : " 0 ");
        key_prev_state = curr_state; // 更新上一次状态
    }

    return curr_state;
}*/

/*
// 【可选】推荐：主循环调用示例（非阻塞）
void Key_Task(void)
{
    // 扫描A键（示例引脚，替换为你的实际按键引脚）
    uint32_t a_key_state = Key_Scan(BSP_IO_PORT_02_PIN_06);
    if(KEY_ON == a_key_state)
    {
        // 执行A键按下的逻辑（如发送确认指令）
    } else {
        // 按键未按下，执行其他逻辑
    }
}
*/


/*
uint32_t Key_Scan(bsp_io_port_pin_t key)
{
    bsp_io_level_t state;
    R_IOPORT_PinRead(&g_ioport_ctrl, key, &state);

    if(BSP_IO_LEVEL_HIGH == state)
    {
        return KEY_OFF;
    }

    else
    {
        do
        {
            R_IOPORT_PinRead(&g_ioport_ctrl, key, &state);
        }while(BSP_IO_LEVEL_LOW == state);
    }
    return KEY_ON;
}
*/
