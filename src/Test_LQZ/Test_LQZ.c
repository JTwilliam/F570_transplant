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

uint32_t pwm_period = 0;
uint32_t pwm_freq = 0;
uint32_t pwm_duty = 0;
uint32_t pwm_high_level = 0;

void Test_Gpt_Pwm(void)
{
    Gpt_Init();
    uart_init(UART_PORT_4);

     Gpt1_Pwm_Setduty(0);

    while (1)
    {
        for(uint8_t duty = 0; duty <= 100; duty += 5)
        {
            Gpt1_Pwm_Setduty(duty);
            R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
        }
        for(uint8_t duty = 100; duty > 0; duty -= 5)
        {
            Gpt1_Pwm_Setduty(duty);
            R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
        }

        // gpt2���벶��(gpt1���pwm��gpt2�������ź�)
//         if (pwm_period > 0)
//         {
//             pwm_freq = gpt2_info.clock_frequency / pwm_period;
//             pwm_duty = pwm_high_level * 100 / pwm_period;
//         }
//         else
//         {
//             pwm_freq = 0;
//             pwm_duty = 0;
//         }


        // uart_printf(UART_PORT_4, "High = %d,Period = %dFreq = %dHZ , Duty = %d%%\r\n", pwm_high_level, pwm_period, pwm_freq, pwm_duty);

        // R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
    }
}
