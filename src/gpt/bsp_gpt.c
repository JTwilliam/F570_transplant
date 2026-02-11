#include "bsp_gpt.h"

// 全局变量定义（仅保留必要变量，删除所有未定义的pwm_xxx）
uint32_t gpt4_period = 0;
volatile uint8_t g_esc_index = ESC_CMD_BUF_LEN;
uint16_t g_esc_cmd[4][ESC_CMD_BUF_LEN] = {0};

/*
 * @brief GPT周期中断回调（Dshot位发送）
 */
void dshot_gpt_update_callback(timer_callback_args_t *p_args)
{
    if(p_args->event == TIMER_EVENT_CYCLE_END)
    {
        // 更新4路电机占空比
        bsp_gpt_set_duty(GPT_IO_PIN_GTIOCA, g_esc_cmd[0][g_esc_index], 1); // M1(GPT1-A)
        bsp_gpt_set_duty(GPT_IO_PIN_GTIOCB, g_esc_cmd[1][g_esc_index], 1); // M2(GPT1-B)
        bsp_gpt_set_duty(GPT_IO_PIN_GTIOCA, g_esc_cmd[2][g_esc_index], 3); // M3(GPT3-A)
        bsp_gpt_set_duty(GPT_IO_PIN_GTIOCB, g_esc_cmd[3][g_esc_index], 3); // M4(GPT3-B)

        g_esc_index++;

        // 帧发送完成，停止中断并清零
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

/*
 * @brief GPT1/GPT3初始化（适配Dshot300）
 */
fsp_err_t bsp_gpt_init(void)
{
    fsp_err_t err = FSP_SUCCESS;

    // 初始化GPT1
    err = R_GPT_Open(DSHOT_GPT1_INSTANCE, &g_timer1_cfg);
    if(err != FSP_SUCCESS) return err;
    err = R_GPT_PeriodSet(DSHOT_GPT1_INSTANCE, DSHOT_GPT_PERIOD_VALUE);
    if(err != FSP_SUCCESS) return err;
    err = R_GPT_CallbackSet(DSHOT_GPT1_INSTANCE, dshot_gpt_update_callback, NULL, NULL);
    if(err != FSP_SUCCESS) return err;

    // 初始化GPT3
    err = R_GPT_Open(DSHOT_GPT3_INSTANCE, &g_timer3_cfg);
    if(err != FSP_SUCCESS) return err;
    err = R_GPT_PeriodSet(DSHOT_GPT3_INSTANCE, DSHOT_GPT_PERIOD_VALUE);
    if(err != FSP_SUCCESS) return err;
    err = R_GPT_CallbackSet(DSHOT_GPT3_INSTANCE, dshot_gpt_update_callback, NULL, NULL);
    if(err != FSP_SUCCESS) return err;

    // 初始化GPT4（仅打开，无错误成员访问）
    err = R_GPT_Open(&g_timer4_ctrl, &g_timer4_cfg);
    if(err != FSP_SUCCESS) return err;
    gpt4_period = DSHOT_GPT_PERIOD_VALUE; // 直接赋值预定义周期

    // 启用GPT
    R_GPT_Enable(DSHOT_GPT1_INSTANCE);
    R_GPT_Enable(DSHOT_GPT3_INSTANCE);
    R_GPT_Enable(&g_timer4_ctrl);

    return err;
}


// 启动 PWM 输出
fsp_err_t pwm_start(timer_ctrl_t *p_ctrl, gpt_io_pin_t pin)
{
    fsp_err_t err = R_GPT_OutputEnable(p_ctrl, pin);  // 使能引脚输出
    if (FSP_SUCCESS == err)
    {
        err = R_GPT_Start(p_ctrl);  // 启动定时器（PWM 开始输出）
    }
    return err;
}

// 停止 PWM 输出
fsp_err_t pwm_stop(timer_ctrl_t *p_ctrl, gpt_io_pin_t pin)
{
    fsp_err_t err = R_GPT_Stop(p_ctrl);  // 停止定时器（PWM 停止输出）
    if (FSP_SUCCESS == err)
    {
        err = R_GPT_OutputDisable(p_ctrl, pin);  // 禁用引脚输出
    }
    return err;
}


/*
 * @brief 设置PWM占空比（无未定义符号，duty_count提前声明）
 */
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

/*
 *@brief 启动GPT中断
 */
fsp_err_t bsp_gpt_base_start_it(void)
{
    R_GPT_Start(DSHOT_GPT1_INSTANCE);
    return R_GPT_Start(DSHOT_GPT3_INSTANCE);
}

/*
  @brief 停止GPT中断
 */
fsp_err_t bsp_gpt_base_stop_it(void)
{
    R_GPT_Stop(DSHOT_GPT1_INSTANCE);
    return R_GPT_Stop(DSHOT_GPT3_INSTANCE);
}


/*fsp_err_t bsp_gpt_pwm_start(gpt_io_pin_t channel)
{
    if(channel == GPT_IO_PIN_GTIOCA || channel == GPT_IO_PIN_GTIOCB)
    {
        return R_GPT_PWM_Start(DSHOT_GPT1_INSTANCE, channel);
    }
    else
    {
        return R_GPT_PWM_Start(DSHOT_GPT3_INSTANCE, channel);
    }
}



fsp_err_t bsp_gpt_pwm_stop(gpt_io_pin_t channel)
{
    if(channel == GPT_IO_PIN_GTIOCA || channel == GPT_IO_PIN_GTIOCB)
    {
        return R_GPT_PWM_Stop(DSHOT_GPT1_INSTANCE, channel);
    }
    else
    {
        return R_GPT_PWM_Stop(DSHOT_GPT3_INSTANCE, channel);
    }
}*/


/*void Gpt_Init(void)
{
    R_GPT_Open(&g_timer4_ctrl, &g_timer4_cfg);
    R_GPT_Open(&g_timer3_ctrl, &g_timer3_cfg);
    R_GPT_Open(&g_timer1_ctrl, &g_timer1_cfg);

    R_GPT_InfoGet(&g_timer4_ctrl, &g_timer4);
 //   gpt4_period = info_timer4.period_counts;

    R_GPT_Enable(&g_timer1_ctrl);
    R_GPT_Enable(&g_timer3_ctrl);
    R_GPT_Enable(&g_timer4_ctrl);

    R_GPT_Start(&g_timer4_ctrl);
    R_GPT_Start(&g_timer3_ctrl);
    R_GPT_Start(&g_timer1_ctrl);
}


//写入（a,b,c,d）(a,b,c,d在0-100内，表示pwm输出占空比0%-100%),pwm1.2.3.4分别输出a,b,c,d
void Gpt_Pwm_Setduty(uint8_t gpt1_pwm, uint8_t gpt2_pwm, uint8_t gpt3_pwm, uint8_t gpt4_pwm)
{
    pwm_period = 0;
    pwm_freq = 0;
    pwm_duty = 0;
    pwm_high_level = 0;

    if(gpt1_pwm > 100)
    {
        gpt1_pwm=100;
    }
    if(gpt2_pwm > 100)
    {
        gpt2_pwm=100;
    }
    if(gpt3_pwm > 100)
    {
        gpt3_pwm=100;
    }
    if(gpt4_pwm > 100)
    {
        gpt4_pwm=100;
    }

    R_GPT_InfoGet(&g_timer1_ctrl, &g_timer1);

    duty_count = (g_timer1.period_counts * gpt1_pwm) / 100;
    R_GPT_DutyCycleSet(&g_timer1_ctrl, duty_count, GPT_IO_PIN_GTIOCA);

    duty_count = (g_timer1.period_counts * gpt2_pwm) / 100;
    R_GPT_DutyCycleSet(&g_timer1_ctrl, duty_count, GPT_IO_PIN_GTIOCB);

    R_GPT_InfoGet(&g_timer3_ctrl, &g_timer3);

    duty_count = (g_timer3.period_counts * gpt3_pwm) / 100;
    R_GPT_DutyCycleSet(&g_timer3_ctrl, duty_count, GPT_IO_PIN_GTIOCA);

    duty_count = (g_timer3.period_counts * gpt4_pwm) / 100;
    R_GPT_DutyCycleSet(&g_timer3_ctrl, duty_count, GPT_IO_PIN_GTIOCB);
}


void gpt4_callback(timer_callback_args_t *p_args)
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
}



void Get_Gpt_Pwm(void)
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

