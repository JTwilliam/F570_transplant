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
#include "stdbool.h"
#include "stdint.h"

uint16_t g_adc_ch0_data = 0;
uint16_t g_adc_ch1_data = 0;
volatile bool scan_all_complete_flag = false;

/*volatile bool scan_all_complete_flag = false;
//adc_voltage_t adc_volt = {0.0, 0.0};

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
    double a0 , a1;

    R_ADC_ScanStart(&g_adc0_ctrl);

    while(!scan_all_complete_flag)
    {
        ;
    }

    scan_all_complete_flag = false;

    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_1, &g_adc_ch0_data);
    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_2, &g_adc_ch1_data);


    a0 = (double)g_adc_ch0_data * 3.3 / 4095.0;
    a1 = (double)g_adc_ch1_data * 3.3 / 4095.0;

        // 打印电压值
    uart_printf(UART_PORT_4, "CH1: %.2f V, CH2: %.2f V\r\n", a0, a1);

}*/


void ADC_Init(void)
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
}

    /*// 1. 定义临时变量接收ADC数据
    uint16_t temp0, temp1;
    while(scan_all_complete_flag)
    {
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_1, &temp0);
        R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_2, &temp1);

        // 3. 再赋值给volatile全局变量
        g_adc_ch0_data = temp0;
        g_adc_ch1_data = temp1;

        adc_volt.ch0_voltage = (double)g_adc_ch0_data * 3.3 / 4095.0;
        adc_volt.ch1_voltage = (double)g_adc_ch1_data * 3.3 / 4095.0;

        // 打印电压值
        uart_printf(UART_PORT_4, "CH1: %.2f V, CH2: %.2f V\r\n", adc_volt.ch0_voltage, adc_volt.ch1_voltage);
        scan_all_complete_flag = false;
    }*/






/*

void Read_Adc_Value(void)
{


    // 等待一轮扫描完成
    while(!scan_all_complete_flag)
    {
        ;
    }
    scan_all_complete_flag = false;

    uint16_t temp0, temp1;
    // 读取双通道原始数据
    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_1, &temp0);
    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_2, &temp1);

    // 保存到全局变量
    g_adc_ch0_data = temp0;
    g_adc_ch1_data = temp1;

    // 转换为实际电压值
    adc_volt.ch0_voltage = temp0 * 3.3 / 4095.0;
    adc_volt.ch1_voltage = temp1 * 3.3 / 4095.0;
    uart_printf(UART_PORT_4, "CH1: %.2f V, CH2: %.2f V\r\n", adc_volt.ch0_voltage, adc_volt.ch1_voltage);
    // 返回整个结构体
    return;
}
*/


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
