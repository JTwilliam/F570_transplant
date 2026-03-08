
#include "bsp_gpt.h"
#include "UART/uart.h"
#include "DSHOT/bsp_dshot.h"

uint32_t gpt4_period = 0;
volatile uint8_t g_esc_index = ESC_CMD_BUF_LEN;
uint16_t g_esc_cmd[4][ESC_CMD_BUF_LEN] = {0};

#define DSHOT_CAPTURE_BUF_LEN 16
volatile uint16_t dshot_capture_buf[DSHOT_CAPTURE_BUF_LEN] = {0};
volatile uint8_t dshot_capture_index = 0;
volatile uint8_t dshot_frame_ready = 0;

extern void decode_and_print_dshot(uint16_t packet);

// 启动GPT1/GPT3中断
fsp_err_t bsp_gpt_base_start_it(void)
{
    R_GPT_Start(DSHOT_GPT1_INSTANCE);
    return R_GPT_Start(DSHOT_GPT3_INSTANCE);
}

// 停止GPT1/GPT3中断
fsp_err_t bsp_gpt_base_stop_it(void)
{
    R_GPT_Stop(DSHOT_GPT1_INSTANCE);
    return R_GPT_Stop(DSHOT_GPT3_INSTANCE);
}


//-------------------- GPT0 DSHOT输入捕获回调（软件交替记录上升沿和下降沿） --------------------
// 回调中交替记录上升沿和下降沿，下降沿时计算高电平宽度
void gpt0_dshot_capture_callback(timer_callback_args_t *p_args)
{
    static uint32_t t_rise = 0;
    static uint32_t t_fall = 0;
    static uint8_t last_is_rise = 1; // 1:等待上升沿, 0:等待下降沿
    uint32_t gpt0_period = 0;
    uint32_t width = 0;

    if (p_args->event == TIMER_EVENT_CAPTURE_A)
    {
        if (last_is_rise)
        {
            t_rise = p_args->capture;
            last_is_rise = 0;
        }
        else
        {
            t_fall = p_args->capture;
            width = (t_fall >= t_rise) ? (t_fall - t_rise) : (gpt0_period - t_rise + t_fall);
            if (dshot_capture_index < DSHOT_CAPTURE_BUF_LEN)
            {
                dshot_capture_buf[dshot_capture_index++] = (uint16_t)width;
            }
            if (dshot_capture_index >= DSHOT_CAPTURE_BUF_LEN)
            {
                dshot_capture_index = 0;
                dshot_frame_ready = 1;
            }
            last_is_rise = 1;
        }
    }
}

//-------------------- DSHOT输入捕获解码与打印（主循环调用） --------------------
void dshot_capture_task(void)
{
    if (dshot_frame_ready)
    {
        uint16_t packet = 0;
        for (uint8_t i = 0; i < DSHOT_CAPTURE_BUF_LEN; i++)
        {
            if (dshot_capture_buf[i] > (DSHOT_GPT_PERIOD_VALUE * 9 / 16))
                packet |= (1 << (15 - i));
        }
        decode_and_print_dshot(packet);
        dshot_frame_ready = 0;
    }
}

// DSHOT发送端定时中断回调（飞控端）
void dshot_gpt_update_callback(timer_callback_args_t *p_args)
{
    if(p_args->event == TIMER_EVENT_CYCLE_END)
    {
        bsp_gpt_set_duty(GPT_IO_PIN_GTIOCA, g_esc_cmd[0][g_esc_index], 1); // M1(GPT1-A)
        bsp_gpt_set_duty(GPT_IO_PIN_GTIOCB, g_esc_cmd[1][g_esc_index], 1); // M2(GPT1-B)
        bsp_gpt_set_duty(GPT_IO_PIN_GTIOCA, g_esc_cmd[2][g_esc_index], 3); // M3(GPT3-A)
        bsp_gpt_set_duty(GPT_IO_PIN_GTIOCB, g_esc_cmd[3][g_esc_index], 3); // M4(GPT3-B)
        g_esc_index++;
        if(g_esc_index == ESC_CMD_BUF_LEN)
        {
            bsp_gpt_base_stop_it();
            bsp_gpt_set_duty(GPT_IO_PIN_GTIOCA, 0 , 1);
            bsp_gpt_set_duty(GPT_IO_PIN_GTIOCB, 0 , 1);
            bsp_gpt_set_duty(GPT_IO_PIN_GTIOCA, 0 , 3);
            bsp_gpt_set_duty(GPT_IO_PIN_GTIOCB, 0 , 3);
            g_esc_index = ESC_CMD_BUF_LEN;
        }
    }
}
fsp_err_t bsp_gpt_set_duty(gpt_io_pin_t channel, uint16_t duty, uint8_t gpt_instance)
{
    uint16_t duty_count = (uint16_t)(((uint32_t)DSHOT_GPT_PERIOD_VALUE * duty) / DSHOT_GPT_PERIOD_VALUE);
    if(duty_count > DSHOT_GPT_PERIOD_VALUE) duty_count = DSHOT_GPT_PERIOD_VALUE;

    if(gpt_instance == 1) // GPT1（M1/M2）
    {
        return R_GPT_DutyCycleSet(DSHOT_GPT1_INSTANCE, channel, duty_count);
    }
    else if(gpt_instance == 3) // GPT3（M3/M4）
    {
        return R_GPT_DutyCycleSet(DSHOT_GPT3_INSTANCE, channel, duty_count);
    }
    return FSP_ERR_INVALID_DATA;
}
// GPT1/GPT3初始化（适配Dshot300）
fsp_err_t bsp_gpt_init(void)
{
    fsp_err_t err = FSP_SUCCESS;
    err = R_GPT_Open(DSHOT_GPT1_INSTANCE, &g_timer1_cfg);
    if(err != FSP_SUCCESS) return err;
    err = R_GPT_PeriodSet(DSHOT_GPT1_INSTANCE, DSHOT_GPT_PERIOD_VALUE);
    if(err != FSP_SUCCESS) return err;

    err = R_GPT_Open(DSHOT_GPT3_INSTANCE, &g_timer3_cfg);
    if(err != FSP_SUCCESS) return err;
    err = R_GPT_PeriodSet(DSHOT_GPT3_INSTANCE, DSHOT_GPT_PERIOD_VALUE);
    if(err != FSP_SUCCESS) return err;

    err = R_GPT_Open(&g_timer4_ctrl, &g_timer4_cfg);
    if(err != FSP_SUCCESS) return err;
    err = R_GPT_CallbackSet(&g_timer4_ctrl, dshot_gpt_update_callback, NULL, NULL);
    if(err != FSP_SUCCESS) return err;
    gpt4_period = DSHOT_GPT_PERIOD_VALUE;

    err = R_GPT_Open(&g_timer0_ctrl, &g_timer0_cfg);
    if(err != FSP_SUCCESS) return err;
    err = R_GPT_CallbackSet(&g_timer0_ctrl, gpt0_dshot_capture_callback, NULL, NULL);
    if(err != FSP_SUCCESS) return err;

    R_GPT_Enable(DSHOT_GPT1_INSTANCE);
    R_GPT_Enable(DSHOT_GPT3_INSTANCE);
    R_GPT_Enable(&g_timer4_ctrl);
    return err;
}


// 启动 PWM 输出
fsp_err_t pwm_start(timer_ctrl_t *p_ctrl, gpt_io_pin_t pin)
{
    fsp_err_t err = R_GPT_OutputEnable(p_ctrl, pin);
    if (FSP_SUCCESS == err)
    {
        err = R_GPT_Start(p_ctrl);
    }
    return err;
}


/*void gpt4_callback(timer_callback_args_t *p_args)
{
    static uint32_t t;
        static uint32_t t1;
        static uint32_t t2;

        static uint32_t overflow_times;
        static uint32_t one_period_flag = 0;

        switch (p_args->event)
        {

        case TIMER_EVENT_CAPTURE_A:
            if (0 == one_period_flag)
            {
                t = p_args->capture;
                overflow_times = 0;
                one_period_flag++;
            }
            else if (1 == one_period_flag)
            {
                t2 = p_args->capture + overflow_times * gpt4_period;
                pwm_period = t2 - t;
                overflow_times = 0;
                one_period_flag = 0;
            }
            break;
        case TIMER_EVENT_CAPTURE_B:
            if (1 == one_period_flag)
            {
                t1 = p_args->capture + overflow_times * gpt4_period;
                pwm_high_level = t1 - t;
            }
            break;
        case TIMER_EVENT_CYCLE_END:
            overflow_times++;
            break;

        default:
            break;
        }
}*/



/*void Get_Gpt_Pwm(void)
{
    while (1)
    {
        if (pwm_period > 0)
        {
            pwm_freq = info_timer4.clock_frequency / pwm_period;
            pwm_duty = pwm_high_level * 100 / pwm_period;
        }
        else
        {
            pwm_freq = 0;
            pwm_duty = 0;
        }

        uart_printf(UART_PORT_4, "High = %d,Period = %d , Freq = %dHZ , Duty = %d%%\r\n", pwm_high_level, pwm_period, pwm_freq, pwm_duty);

        pwm_period = 0;
        pwm_freq = 0;
        pwm_duty = 0;
        pwm_high_level = 0;

        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_SECONDS);
    }
}*/

