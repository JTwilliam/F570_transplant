/*
 * Test_LQZ.c
 *
 *  Created on: 2026��1��22��
 *      Author: huago
 */

#include "Test_LQZ.h"
#include "gpt/bsp_gpt.h"
#include "UART/uart.h"
#include "stdio.h"
#include "inttypes.h"
#include "ADC/bsp_adc.h"
#include "KEY/bsp_key.h"
#include "LED/bsp_led.h"
#include "JOYSTICK/bsp_joystick.h"
#include "DSHOT/bsp_dshot.h"



//void motor_test(dshotMotorVal_t m1, dshotMotorVal_t m2, dshotMotorVal_t m3, dshotMotorVal_t m4)经过dshot编码后要发送的的数据用串口打印出来
void motor_test(dshotMotorVal_t m1, dshotMotorVal_t m2, dshotMotorVal_t m3, dshotMotorVal_t m4)
{
    // 油门值限幅（0~2047）
    m1.throttle = (m1.throttle > 2047) ? 2047 : m1.throttle;
    m2.throttle = (m2.throttle > 2047) ? 2047 : m2.throttle;
    m3.throttle = (m3.throttle > 2047) ? 2047 : m3.throttle;
    m4.throttle = (m4.throttle > 2047) ? 2047 : m4.throttle;

    // 编码Dshot数据包
    m1.throttle = (uint16_t) DshotDecode(m1);
    m2.throttle = (uint16_t) DshotDecode(m2);
    m3.throttle = (uint16_t) DshotDecode(m3);
    m4.throttle = (uint16_t) DshotDecode(m4);

    // 打印编码后的数据包
    uart_printf(UART_PORT_4, "Motor 1: %4X\n", m1.throttle);
    uart_printf(UART_PORT_4, "Motor 2: %4X\n", m2.throttle);
    uart_printf(UART_PORT_4, "Motor 3: %4X\n", m3.throttle);
    uart_printf(UART_PORT_4, "Motor 4: %4X\n", m4.throttle);
}






/*
void Joystick_Test_RealTime(uint32_t print_interval_ms)
{

    //R_ADC_ScanStart(&g_adc0_ctrl);
    // 1. 先执行摇杆校准（确保零位准确）
    Joystick_Calibration();

    Joystick_Cmd_t cmd;
    uint8_t ps2_data[4] = {0};
    uint16_t adc_raw[4] = {0};    // 原始ADC值（0:YAW 1:THR 2:ROLL 3:PITCH）
    uint16_t adc_filtered[4] = {0}; // 滤波后ADC值

    // 打印表头（方便识别列含义）

    uart_printf(UART_PORT_4, "YAW| THR| ROLL| PITCH\r\n");

    while(1)
    {

        R_ADC_ScanStart(&g_adc0_ctrl); // 启动ADC扫描，采集所有通道的最新值
        ADC_Wait_ScanDone();

        // 2. 逐个轴读取原始ADC值 + 滤波后值
        // 左X轴（YAW）
        R_ADC_Read(&g_adc0_ctrl, ADC_CH_LEFT_X, &adc_raw[0]);
        adc_filtered[0] = ADC_Sliding_Filter(0, adc_raw[0]);
        // 左Y轴（THR）
        R_ADC_Read(&g_adc0_ctrl, ADC_CH_LEFT_Y, &adc_raw[1]);
        adc_filtered[1] = ADC_Sliding_Filter(1, adc_raw[1]);
        // 右X轴（ROLL）
        R_ADC_Read(&g_adc0_ctrl, ADC_CH_RIGHT_X, &adc_raw[2]);
        adc_filtered[2] = ADC_Sliding_Filter(2, adc_raw[2]);
        // 右Y轴（PITCH）
        R_ADC_Read(&g_adc0_ctrl, ADC_CH_RIGHT_Y, &adc_raw[3]);
        adc_filtered[3] = ADC_Sliding_Filter(3, adc_raw[3]);

        // 3. 获取标准化指令 + 转换为PS2格式
        Joystick_Get_Cmd(&cmd);
        Joystick_To_PS2_Format(&cmd, ps2_data);

        // 4. 串口打印实时数据（格式对齐，方便查看）
        uart_printf(UART_PORT_4,
                    "%d/%d/%d/%d        | %d/%d/%d/%d        | %d/%d/%d/%d        | %d/%d/%d/%d\r\n",
                    // YAW：原始ADC/滤波ADC/指令值/PS2值
                    adc_raw[0], adc_filtered[0], cmd.yaw, ps2_data[0],
                    // THR：原始ADC/滤波ADC/指令值/PS2值
                    adc_raw[1], adc_filtered[1], cmd.thr, ps2_data[1],
                    // ROLL：原始ADC/滤波ADC/指令值/PS2值
                    adc_raw[2], adc_filtered[2], cmd.roll, ps2_data[2],
                    // PITCH：原始ADC/滤波ADC/指令值/PS2值
                    adc_raw[3], adc_filtered[3], cmd.pitch, ps2_data[3]);

        // 5. 打印间隔（避免刷屏，建议20~50ms）
        R_BSP_SoftwareDelay(print_interval_ms, BSP_DELAY_UNITS_MILLISECONDS);
    }
}
*/

/*

void Test_Gpt_Pwm(uint8_t A, uint8_t B, uint8_t C, uint8_t D)
{
    Gpt_Init();
    uart_init(UART_PORT_4);

    Gpt_Pwm_Setduty(A, B, C, D);

    Get_Gpt_Pwm();
}


void Test_Gpt_Pwm(void)
{
    Gpt_Init();
    uart_init(UART_PORT_4);

    Gpt1_Pwm_Setduty(60);

    while (1)
    {
        if (pwm_period > 0)
        {
            pwm_freq = gpt2_info.clock_frequency / pwm_period;
            pwm_duty = pwm_high_level * 100 / pwm_period;
        }
        else
        {
            pwm_freq = 0;
            pwm_duty = 0;
        }

        uart_printf(UART_PORT_4, "High = %d,Period = %dFreq = %dHZ , Duty = %d%%\r\n", pwm_high_level, pwm_period, pwm_freq, pwm_duty);

        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
    }
}
*/

void Test_Adc(void)
{
    uart_init(UART_PORT_4);
    uart_printf(UART_PORT_4, "UART\n");
    ADC_Init();
    uart_printf(UART_PORT_4, "ADC\n");

    while(1)
    {
        Read_Adc_Value();
        R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
    }

}

void Test_Key(bsp_io_port_pin_t key_pin)
{
    Key_CheckEvent(key_pin);


/*    uart_init(UART_PORT_4);
    Key_Init();

    while(1)
    {
        if(Key_Scan(key) == KEY_ON )
        {
            LED_TOGGLE;
        }
        R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
    }*/
}




