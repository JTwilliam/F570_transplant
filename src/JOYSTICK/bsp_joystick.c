/*
 * bsp_joystick.c
 *
 *  Created on: 2026年1月27日
 *      Author: huago
 */

#include "bsp_joystick.h"
#include "UART/uart.h"
#include "ADC/bsp_adc.h"

/************************ 静态全局变量（仅内部使用，不对外暴露） ************************/
static uint16_t joy_calib_center[4] = {2048, 2048, 2048, 2048}; // 摇杆校准中心值
static uint16_t g_adc_filter_buf[4][5] = {0};                   // ADC滑动滤波缓冲区
static uint8_t g_filter_idx = 0;                                // 滤波缓冲区索引

/************************ 静态工具函数（仅内部使用） ************************/
/**
 * @brief ADC滑动平均滤波（降噪，提升摇杆控制稳定性）
 * @param axis 摇杆轴索引（0-3）
 * @param new_val 新采集的ADC值
 * @return 滤波后的ADC值
 */
static uint16_t ADC_Sliding_Filter(uint8_t axis, uint16_t new_val)
{
    g_adc_filter_buf[axis][g_filter_idx] = new_val;
    g_filter_idx = (uint8_t)((g_filter_idx + 1) % 5);

    uint32_t sum = 0;
    for (uint8_t i = 0; i < 5; i++)
    {
        sum += g_adc_filter_buf[axis][i];
    }
    return (uint16_t)(sum / 5);
}

/**
 * @brief 摇杆ADC值处理（死区+归一化+限幅）
 * @param axis 摇杆轴索引（0-3）
 * @param adc_val 滤波后的ADC值
 * @return 标准化的控制量
 */
static int16_t Joystick_Process(uint8_t axis, uint16_t adc_val)
{
    int16_t diff = (int16_t)adc_val - (int16_t)joy_calib_center[axis];

    // 死区处理：中心微小波动视为0
    if (abs(diff) < JOY_DEAD_ZONE)
    {
        return (axis == 1) ? THR_HOVER : 0; // 油门轴默认悬停，其他轴默认0
    }

    int16_t cmd;
    if (axis == 1)
    { // 油门通道（左Y轴）
        cmd = THR_HOVER + (diff * (THR_MAX - THR_HOVER)) / (JOY_MAX_VALUE - joy_calib_center[axis]);
        cmd = (cmd < THR_MIN) ? THR_MIN : (cmd > THR_MAX) ? THR_MAX : cmd;
    }
    else
    { // 姿态通道（偏航/横滚/俯仰）
        int32_t temp = (int32_t)diff * 100;
        temp /= (int32_t)(JOY_MAX_VALUE - joy_calib_center[axis]);
        cmd = (int16_t)temp;
        cmd = (cmd < -ATTITUDE_MAX) ? -ATTITUDE_MAX : (cmd > ATTITUDE_MAX) ? ATTITUDE_MAX : cmd;
    }
    return cmd;
}

/**
 * @brief CRC8校验计算（保证指令传输可靠）
 * @param data 待校验数据
 * @param len 数据长度
 * @return CRC8校验值
 */

/*
static uint8_t CRC8_Calculate(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0xFF;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
        }
    }
    return crc;
}
*/

/************************ 对外接口函数实现 ************************/
void Joystick_Calibration(void) {
    uint16_t adc_val, sum, max_val, min_val;
    uart_printf(UART_PORT_4,"Release!\r\n");
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);

    for (uint8_t axis = 0; axis < 4; axis++)
    {
        sum = 0;
        max_val = 0;
        min_val = JOY_MAX_VALUE;

        // 多次采集，计算平均值+检测波动
        for (uint8_t i = 0; i < 10; i++)
        {
            R_ADC_Read(&g_adc0_ctrl, axis, &adc_val);
            sum += adc_val;
            max_val = (adc_val > max_val) ? adc_val : max_val;
            min_val = (adc_val < min_val) ? adc_val : min_val;
            R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
        }

        // 波动过大，重新校准当前轴
        if ((max_val - min_val) > 20)
        {
            axis--;
            continue;
        }

        joy_calib_center[axis] = sum / 10;
        // 初始化滤波缓冲区
        for (uint8_t i = 0; i < 5; i++)
        {
            g_adc_filter_buf[axis][i] = joy_calib_center[axis];
        }
    }

    printf("Calibration Done: YAW=%d THR=%d ROLL=%d PITCH=%d\r\n",
           joy_calib_center[0], joy_calib_center[1], joy_calib_center[2], joy_calib_center[3]);
}



void Joystick_Get_Cmd(Joystick_Cmd_t *cmd)
{
    // 初始化指令
    cmd->thr = THR_MIN;
    cmd->yaw = 0;
    cmd->roll = 0;
    cmd->pitch = 0;

    uint16_t adc_val, filtered_adc;

    // 左X轴（偏航）采集+处理
    R_ADC_Read(&g_adc0_ctrl, ADC_CH_LEFT_X, &adc_val);
    filtered_adc = ADC_Sliding_Filter(0, adc_val);
    cmd->yaw = Joystick_Process(0, filtered_adc);

    // 左Y轴（油门）采集+处理
    R_ADC_Read(&g_adc0_ctrl, ADC_CH_LEFT_Y, &adc_val);
    filtered_adc = ADC_Sliding_Filter(1, adc_val);
    cmd->thr = (uint8_t)Joystick_Process(1, filtered_adc);

    // 右X轴（横滚）采集+处理
    R_ADC_Read(&g_adc0_ctrl, ADC_CH_RIGHT_X, &adc_val);
    filtered_adc = ADC_Sliding_Filter(2, adc_val);
    cmd->roll = Joystick_Process(2, filtered_adc);

    // 右Y轴（俯仰）采集+处理
    R_ADC_Read(&g_adc0_ctrl, ADC_CH_RIGHT_Y, &adc_val);
    filtered_adc = ADC_Sliding_Filter(3, adc_val);
    cmd->pitch = Joystick_Process(3, filtered_adc);

    // 安全机制
    if (cmd->thr < SAFETY_THRESHOLD)
    {
        cmd->yaw = 0;
        cmd->roll = 0;
        cmd->pitch = 0;
    }
}


// 转换为PS2协议格式的8位模拟量
void Joystick_To_PS2_Format(Joystick_Cmd_t *cmd, uint8_t ps2_data[4])
{
    // 左X轴（偏航）：-80~+80 → 0~255，中心127
    int32_t temp = 127 + ((int32_t)cmd->yaw * 127) / 80;
    ps2_data[0] = (uint8_t)temp; // 显式转换，消除-Wconversion

    // 左Y轴（油门）：0~90 → 0（上最大）~255（下最大），中心127
    temp = 255 - ((int32_t)cmd->thr * 255) / 90;
    ps2_data[1] = (uint8_t)temp;

    // 右X轴（横滚）：-80~+80 → 0~255，中心127
    temp = 127 + ((int32_t)cmd->roll * 127) / 80;
    ps2_data[2] = (uint8_t)temp;

    // 右Y轴（俯仰）：-80~+80 → 0（上最大）~255（下最大），中心127
    temp = 255 - ((int32_t)cmd->pitch * 255) / 80;
    ps2_data[3] = (uint8_t)temp;

    // 可选：保留限幅并抑制警告（若需兼容输入范围变化）
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wtype-limits"
    for (uint8_t i = 0; i < 4; i++)
    {
        ps2_data[i] = ps2_data[i] > 255 ? 255 : (ps2_data[i] < 0 ? 0 : ps2_data[i]);
    }
    #pragma GCC diagnostic pop
}



/*
 *
 *
static Joystick_Cmd_t g_last_cmd = {THR_MIN, 0, 0, 0};          // 历史指令（增量发送用）
void Joystick_Send_Cmd(void)
{
    Joystick_Cmd_t curr_cmd = Joystick_Get_Cmd();

    // 增量发送：仅指令变化时发送，减少飞控接收冗余
    if (abs(curr_cmd.yaw - g_last_cmd.yaw) <= 1 &&
        abs(curr_cmd.roll - g_last_cmd.roll) <= 1 &&
        abs(curr_cmd.pitch - g_last_cmd.pitch) <= 1 &&
        curr_cmd.thr == g_last_cmd.thr)
    {
        return;
    }

    // 打包飞控可识别的帧数据
    Cmd_Frame_t frame =
    {
        .header = 0xAA,
        .thr = curr_cmd.thr,
        .yaw = curr_cmd.yaw,
        .roll = curr_cmd.roll,
        .pitch = curr_cmd.pitch,
        .tail = 0x55
    };
    frame.crc8 = CRC8_Calculate((uint8_t*)&frame, sizeof(Cmd_Frame_t) - 1);

    // 串口发送给飞控（瑞萨SCI接口）
    R_SCI_UART_Write(&g_sci1_ctrl, (uint8_t*)&frame, sizeof(Cmd_Frame_t));
    // 更新历史指令
    g_last_cmd = curr_cmd;
}

*********************** 主函数（独立工程用，集成时可删除/替换） ***********************
void main(void) {
    // 初始化：仅保留ADC+串口（飞控对接必需）
    HAL_Init();
    R_ADC_Open(&g_adc0_ctrl, &g_adc0_cfg);
    R_SCI_UART_Open(&g_sci1_ctrl, &g_sci1_cfg);

    // 上电校准摇杆
    Joystick_Calibration();

    // 主循环：20ms采集+发送指令
    while (1) {
        Joystick_Get_Cmd();
        Joystick_Send_Cmd();
        HAL_Delay(SAMPLE_PERIOD);
    }
}
*/
