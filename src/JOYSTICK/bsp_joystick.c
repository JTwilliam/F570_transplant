/*
 * bsp_joystick.c
 *
 *  Created on: 2026年1月27日
 *      Author: huago
 */


#include "bsp_joystick.h"
#include "UART/uart.h"
#include "ADC/bsp_adc.h"

/************************ 静态全局变量 ************************/
uint16_t joy_calib_center[4] = {2048, 2048, 2048, 2048}; // 仅用于初始化滤波缓冲区
uint16_t g_adc_filter_buf[4][5] = {0};                   // ADC滑动滤波缓冲区
uint8_t g_filter_idx = 0;                                // 滤波缓冲区索引
static Joystick_ADC_Raw_t g_last_valid_adc = {2048, 2048, 2048, 2048};
/************************ 静态工具函数 ************************/
/**
 * @brief ADC滑动平均滤波（降噪，保留原始值范围）
 * @param axis 摇杆轴索引（0-3）
 * @param new_val 新采集的ADC值
 * @return 滤波后的ADC值（0-4095）
 */
uint16_t ADC_Sliding_Filter(uint8_t axis, uint16_t new_val)
{
    g_adc_filter_buf[axis][g_filter_idx] = new_val;
    g_filter_idx = (uint8_t)((g_filter_idx + 1) % 5);

    uint32_t sum = 0;
    for (uint8_t i = 0; i < 5; i++)
    {
        sum += g_adc_filter_buf[axis][i];
    }
    return (uint16_t)(sum / 5); // 滤波后仍为0-4095
}

void ADC_Wait_ScanDone(void)
{
    adc_status_t adc_status;
    do
    {
        R_ADC_StatusGet(&g_adc0_ctrl, &adc_status);
    } while (adc_status.state);
}

/************************ 对外接口函数实现 ************************/
void Joystick_Calibration(void) {
    uint16_t adc_val, sum, max_val, min_val;
    uart_printf(UART_PORT_4,"Release!\r\n");
    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);

    for (uint8_t axis = 0; axis < 4; axis++)
    {
        sum = 0;
        max_val = 0;
        min_val = JOY_MAX_VALUE;

        R_ADC_ScanStart(&g_adc0_ctrl);
        ADC_Wait_ScanDone();

        // 多次采集初始化滤波缓冲区（仅降噪，不修改原始值范围）
        for (uint8_t i = 0; i < 10; i++)
        {
            R_ADC_Read(&g_adc0_ctrl, axis, &adc_val);
            sum += adc_val;
            max_val = (adc_val > max_val) ? adc_val : max_val;
            min_val = (adc_val < min_val) ? adc_val : min_val;
            R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
        }

        if ((max_val - min_val) > 20)
        {
            axis--;
            continue;
        }

        joy_calib_center[axis] = sum / 10;
        // 初始化滤波缓冲区为校准值（降噪）
        for (uint8_t i = 0; i < 5; i++)
        {
            g_adc_filter_buf[axis][i] = joy_calib_center[axis];
        }
    }

    uart_printf(UART_PORT_4, "Calibration Done: YAW=%d THR=%d ROLL=%d PITCH=%d\r\n",
           joy_calib_center[0], joy_calib_center[1], joy_calib_center[2], joy_calib_center[3]);
}

/**
 * @brief 采集并获取4轴滤波后的ADC原始值（核心修改：仅读原始值，无阈值/范围处理）
 */
void Joystick_Get_ADC_Raw(Joystick_ADC_Raw_t *raw)
{
    uint16_t adc_val, filtered_adc;
    fsp_err_t err; // 新增：返回值存储

    // 左X轴（偏航）：带返回值校验
    err = R_ADC_Read(&g_adc0_ctrl, ADC_CH_LEFT_X, &adc_val);
    if (FSP_SUCCESS != err) {
        uart_printf(UART_PORT_4, "ADC Read YAW Error: %d\r\n", err);
        filtered_adc = ADC_Sliding_Filter(0, g_last_valid_adc.adc_yaw); // 用上次有效值
    }
    else
    {
        filtered_adc = ADC_Sliding_Filter(0, adc_val);
    }
    raw->adc_yaw = filtered_adc;

    // 左Y轴（油门）：同理
    err = R_ADC_Read(&g_adc0_ctrl, ADC_CH_RIGHT_Y, &adc_val);
    if (FSP_SUCCESS != err)
    {
        uart_printf(UART_PORT_4, "ADC Read THR Error: %d\r\n", err);
        filtered_adc = ADC_Sliding_Filter(1, g_last_valid_adc.adc_thr);
    }
    else
    {
        filtered_adc = ADC_Sliding_Filter(1, adc_val);
    }
    raw->adc_thr = filtered_adc;

    // 右X轴、右Y轴
    err = R_ADC_Read(&g_adc0_ctrl, ADC_CH_LEFT_Y, &adc_val);
    if (FSP_SUCCESS != err)
    {
        uart_printf(UART_PORT_4, "ADC Read THR Error: %d\r\n", err);
        filtered_adc = ADC_Sliding_Filter(1, g_last_valid_adc.adc_roll);
    }

    else
    {
        filtered_adc = ADC_Sliding_Filter(1, adc_val);
    }
    raw->adc_roll = filtered_adc;


    //右Y轴
    err = R_ADC_Read(&g_adc0_ctrl, ADC_CH_RIGHT_X, &adc_val);
    if (FSP_SUCCESS != err) {
        uart_printf(UART_PORT_4, "ADC Read THR Error: %d\r\n", err);
        filtered_adc = ADC_Sliding_Filter(1, g_last_valid_adc.adc_pitch);
    }

    else
    {
        filtered_adc = ADC_Sliding_Filter(1, adc_val);
    }
    raw->adc_pitch = filtered_adc;

    // 更新全局有效缓存
    memcpy(&g_last_valid_adc, raw, sizeof(Joystick_ADC_Raw_t));
}

/**
 * @brief 核心修改：ADC原始值（0-4095）直接映射到PS2（0-255）
 * 映射公式：ps2_val = (adc_val * 255) / 4095 （线性缩放）
 */
void Joystick_To_PS2_Format(Joystick_ADC_Raw_t *raw, uint8_t ps2_data[4])
{
    // 左X轴（偏航）：0-4095 → 0-255
    ps2_data[0] = (uint8_t)((uint32_t)raw->adc_yaw * 255 / 4095);
    // 左Y轴（油门）：0-4095 → 0-255
    ps2_data[1] = (uint8_t)((uint32_t)raw->adc_thr * 255 / 4095);
    // 右X轴（横滚）：0-4095 → 0-255
    ps2_data[2] = (uint8_t)((uint32_t)raw->adc_roll * 255 / 4095);
    // 右Y轴（俯仰）：0-4095 → 0-255
    ps2_data[3] = (uint8_t)((uint32_t)raw->adc_pitch * 255 / 4095);

    // 安全限幅（防止计算溢出，确保0-255）
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wtype-limits"
    for (uint8_t i = 0; i < 4; i++)
    {
        ps2_data[i] = ps2_data[i] > 255 ? 255 : (ps2_data[i] < 0 ? 0 : ps2_data[i]);
    }
    #pragma GCC diagnostic pop
}

// 兼容原有函数（若无需保留可删除）
void Joystick_Get_Cmd(Joystick_Cmd_t *cmd)
{
    cmd->thr = 0;
    cmd->yaw = 0;
    cmd->roll = 0;
    cmd->pitch = 0;
}

void Joystick_Send_Cmd(void)
{
    // 若无需保留发送逻辑，可留空或删除
}
