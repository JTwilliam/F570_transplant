/*
 * bsp_adc.c
 *
 *  Created on: 2026��1��22��
 *      Author: huago
 */
/*
 * bsp_adc.c
 *
 *  Created on: 2026年1月22日
 *      Author: huago
 */

#include "bsp_adc.h"
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// 瑞萨平台 ADC 数据
uint16_t g_adc_ch1_data = 0;
uint16_t g_adc_ch2_data = 0;
uint16_t g_adc_ch3_data = 0;
uint16_t g_adc_ch4_data = 0;

volatile bool scan_all_complete_flag = false;

// 机器人电压（兼容 STM32 版本）
float g_robotVOL = 12.0f;

// ADC 缓冲区，用于平均滤波（类似 STM32 DMA 缓冲区）
#define userconfig_ADC_BUF_LEN 10
static uint16_t g_Adc_Buf[userconfig_ADC_BUF_LEN][4] = {0};
static uint8_t g_adc_buf_index = 0;

void ADC_Userconfig_Init(void)
{
    fsp_err_t err;

    err = FSP_SUCCESS;
    err = R_ADC_Open(&g_adc0_ctrl, &g_adc0_cfg);
    err = R_ADC_ScanCfg(&g_adc0_ctrl, &g_adc0_channel_cfg);
    err = R_ADC_CallbackSet(&g_adc0_ctrl, adc_callback,  NULL, NULL);
    // 启动 ADC 连续扫描（类似 STM32 的 HAL_ADC_Start_DMA）
    // 注意：如果配置了连续模式，ADC 会自动重复扫描
    err = R_ADC_ScanStart(&g_adc0_ctrl);

    assert(FSP_SUCCESS == err);
}

void adc_callback(adc_callback_args_t *p_args)
{
    if (p_args->event == ADC_EVENT_SCAN_COMPLETE)
    {
        // ADC 扫描完成，在中断中直接读取数据（类似 STM32 DMA 自动搬运）
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_1, &g_adc_ch1_data);
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_2, &g_adc_ch2_data);
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_3, &g_adc_ch3_data);
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_4, &g_adc_ch4_data);

        // 存入缓冲区用于平均滤波（类似 STM32 DMA 多次采样）
        g_Adc_Buf[g_adc_buf_index][0] = g_adc_ch1_data;
        g_Adc_Buf[g_adc_buf_index][1] = g_adc_ch2_data;
        g_Adc_Buf[g_adc_buf_index][2] = g_adc_ch3_data;
        g_Adc_Buf[g_adc_buf_index][3] = g_adc_ch4_data;

        g_adc_buf_index++;
        if (g_adc_buf_index >= userconfig_ADC_BUF_LEN)
        {
            g_adc_buf_index = 0;
        }

        // 更新机器人电压（假设通道2用于电压检测，4倍分压）
        g_robotVOL = (float)g_adc_ch2_data / 4096.0f * 3.3f * 4.0f;

        scan_all_complete_flag = true;
    }
}

// 获取 ADC 通道值（平均滤波，兼容 STM32 版本）
// 应用层直接调用此函数获取数据，无需触发 ADC 采集
uint16_t USER_ADC_Get_AdcBufValue(uint8_t channel)
{
    uint32_t tmp;
    uint8_t i;
    tmp = 0;

    // 防止索引溢出
    if (channel > 3)
    {
        return 0;
    }

    for (i = 0; i < userconfig_ADC_BUF_LEN; i++)
    {
        tmp += g_Adc_Buf[i][channel];
    }

    return (uint16_t)(tmp / userconfig_ADC_BUF_LEN);
}

// ADC 接口实例（兼容 STM32 版本）
ADCInterface_t UserADC1 = {
    .init = ADC_Userconfig_Init,
    .getValue = USER_ADC_Get_AdcBufValue,
};



/*
#include "bsp_adc.h"
#include "stdbool.h"
#include "stdint.h"

uint16_t g_adc_ch1_data = 0;
uint16_t g_adc_ch2_data = 0;
uint16_t g_adc_ch3_data = 0;
uint16_t g_adc_ch4_data = 0;

volatile bool scan_all_complete_flag = false;

void ADC_Init(void)
{
    fsp_err_t err;

    err = R_ADC_Open(&g_adc0_ctrl, &g_adc0_cfg);
    err = R_ADC_ScanCfg(&g_adc0_ctrl, &g_adc0_channel_cfg);

    assert(FSP_SUCCESS == err);
}

void adc_callback(adc_callback_args_t *p_args)
{
    if (p_args->event == ADC_EVENT_SCAN_COMPLETE)
    {
        // 设置扫描完成标志
        scan_all_complete_flag = true;
    }
}


void Read_Adc_Value(void)
{
    double v1 , v2 , v3 , v4;

    R_ADC_ScanStart(&g_adc0_ctrl);

    while(!scan_all_complete_flag)
    {
        ;
    }

    scan_all_complete_flag = false;

    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_1, &g_adc_ch1_data);
    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_2, &g_adc_ch2_data);
    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_3, &g_adc_ch3_data);
    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_4, &g_adc_ch4_data);

    v1 = (double)g_adc_ch1_data ;//* 3.3 / 4095.0;
    v2 = (double)g_adc_ch2_data ;//* 3.3 / 4095.0;
    v3 = (double)g_adc_ch3_data ;//* 3.3 / 4095.0;
    v4 = (double)g_adc_ch4_data ;//* 3.3 / 4095.0;

        // 打印电压值
    uart_printf(UART_PORT_4, "CH1: %.2f V, CH2: %.2f V,CH3: %.2f V, CH4: %.2f V\r\n", v1, v2,v3,v4);

}

void ADC_Userconfig_Init(void)
{
    fsp_err_t err;

    err = FSP_SUCCESS;
    err = R_ADC_Open(&g_adc0_ctrl, &g_adc0_cfg);
    err = R_ADC_ScanCfg(&g_adc0_ctrl, &g_adc0_channel_cfg);

    // 启动 ADC 连续扫描（类似 STM32 的 HAL_ADC_Start_DMA）
    // 注意：如果配置了连续模式，ADC 会自动重复扫描
    err = R_ADC_ScanStart(&g_adc0_ctrl);

    assert(FSP_SUCCESS == err);
}

*/


/*void Read_JoyStick_Adc_Value(uint16_t *x_axis, uint16_t *y_axis)
{
    while(!scan_all_complete_flag)
    {
        ;
    }

    scan_all_complete_flag = false;

    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_3, x_axis); // 假设X轴连接到ADC_CHANNEL_3
    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_4, y_axis); // 假设Y轴连接到ADC_CHANNEL_4

    // 可以根据需要打印或处理X轴和Y轴的值
    uart_printf(UART_PORT_4, "Joystick X: %d, Y: %d\r\n", *x_axis, *y_axis);
}*/


/*void ADC_Init(void)
{
    fsp_err_t err;

    err = R_ADC_Open(&g_adc0_ctrl, &g_adc0_cfg);
    err = R_ADC_ScanCfg(&g_adc0_ctrl, &g_adc0_channel_cfg);
//    err = R_ADC_CallbackSet(&g_adc0_ctrl, adc_callback,  NULL, NULL);
    err = R_ADC_ScanStart(&g_adc0_ctrl);

    assert(FSP_SUCCESS == err);
}



void adc_callback(adc_callback_args_t *p_args)
{
//    uart_printf(UART_PORT_4, "666");
    if (p_args->event == ADC_EVENT_SCAN_COMPLETE)
    {
        // 设置扫描完成标志
        scan_all_complete_flag = true;
    }
}

void Read_Adc_Value(void)
{
    double a0 =0;
    double a1=0;
    uart_printf(UART_PORT_4,"1");
        while(!scan_all_complete_flag)
        {
            uart_printf(UART_PORT_4,"2");
        }
        uart_printf(UART_PORT_4,"3");
        scan_all_complete_flag = false;

        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_1, &g_adc_ch0_data);
        uart_printf(UART_PORT_4,"4");

        a0 = (float)(g_adc_ch0_data * 3.3 / 4095.0);
        uart_printf(UART_PORT_4,"5");

        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_2, &g_adc_ch1_data);
        uart_printf(UART_PORT_4,"6");

        a1 = (float)(g_adc_ch1_data * 3.3 / 4095.0);
        uart_printf(UART_PORT_4, "CH1: %.2f V, CH2: %.2f V\r\n", a0, a1);
        uart_printf(UART_PORT_4,"7");

            // 打印电压值

        uart_printf(UART_PORT_4,"8");
}*/


/*void Read_Adc_Value(void)
{

    adc_voltage_t adc_volt = {0.0, 0.0};

        while(!scan_all_complete_flag)
        {
            ;
        }
        scan_all_complete_flag = false;

        uint16_t temp0, temp1;
        // 先读到临时变量，再赋值给volatile全局变量

        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_0, &temp0);
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_1, &temp1);
        g_adc_ch0_data = temp0;
        g_adc_ch1_data = temp1;

        adc_volt.ch0_voltage = (double)g_adc_ch0_data * 3.3 / 4095.0;
        adc_volt.ch1_voltage = (double)g_adc_ch1_data * 3.3 / 4095.0;

}*/
