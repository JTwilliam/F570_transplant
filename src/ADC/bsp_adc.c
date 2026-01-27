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
    double v1 , v2;

    R_ADC_ScanStart(&g_adc0_ctrl);

    while(!scan_all_complete_flag)
    {
        ;
    }

    scan_all_complete_flag = false;

    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_1, &g_adc_ch1_data);
    R_ADC_Read(&g_adc0_ctrl, ADC_CHANNEL_2, &g_adc_ch2_data);


    v1 = (double)g_adc_ch1_data * 3.3 / 4095.0;
    v2 = (double)g_adc_ch2_data * 3.3 / 4095.0;

        // 打印电压值
    uart_printf(UART_PORT_4, "CH1: %.2f V, CH2: %.2f V\r\n", v1, v2);

}


void Read_JoyStick_Adc_Value(uint16_t *x_axis, uint16_t *y_axis)
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
}


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
