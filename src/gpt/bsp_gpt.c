#include "bsp_gpt.h"
#include "UART/uart.h"


timer_info_t info_timer1;
timer_info_t info_timer3;
timer_info_t info_timer4;

uint32_t gpt4_period;
uint32_t duty_count;

uint32_t pwm_period = 0;
uint32_t pwm_freq = 0;
uint32_t pwm_duty = 0;
uint32_t pwm_high_level = 0;

void Gpt_Init(void)
{
    R_GPT_Open(&g_timer4_ctrl, &g_timer4_cfg);
    R_GPT_Open(&g_timer3_ctrl, &g_timer3_cfg);
    R_GPT_Open(&g_timer1_ctrl, &g_timer1_cfg);

    R_GPT_InfoGet(&g_timer4_ctrl, &info_timer4);
    gpt4_period = info_timer4.period_counts;

    R_GPT_Enable(&g_timer4_ctrl);

    R_GPT_Start(&g_timer4_ctrl);
    R_GPT_Start(&g_timer3_ctrl);
    R_GPT_Start(&g_timer1_ctrl);
}


//写入（a,b,c,d）(a,b,c,d在0-100内，表示pwm输出占空比0%-100%),pwm1.2.3.4分别输出a,b,c,d
void Gpt_Pwm_Setduty(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{

    if(a > 100)
    {
        a=100;
    }
    if(b > 100)
    {
        b=100;
    }
    if(c > 100)
    {
        c=100;
    }
    if(d > 100)
    {
        d=100;
    }

    R_GPT_InfoGet(&g_timer1_ctrl, &info_timer1);

    duty_count = (info_timer1.period_counts * a) / 100;
    R_GPT_DutyCycleSet(&g_timer1_ctrl, duty_count, GPT_IO_PIN_GTIOCA);

    duty_count = (info_timer1.period_counts * b) / 100;
    R_GPT_DutyCycleSet(&g_timer1_ctrl, duty_count, GPT_IO_PIN_GTIOCB);

    R_GPT_InfoGet(&g_timer3_ctrl, &info_timer3);

    duty_count = (info_timer3.period_counts * c) / 100;
    R_GPT_DutyCycleSet(&g_timer3_ctrl, duty_count, GPT_IO_PIN_GTIOCA);

    duty_count = (info_timer3.period_counts * d) / 100;
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

        R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_SECONDS);
    }
}





/*
void Gpt_Init(void)
{
    R_GPT_Open(&g_timer3_ctrl, &g_timer3_cfg);
    R_GPT_Open(&g_timer1_ctrl, &g_timer1_cfg);

//    R_GPT_Enable(&g_timer3_ctrl);

    R_GPT_Start(&g_timer3_ctrl);
    R_GPT_Start(&g_timer1_ctrl);
}

// gpt1/2pwm输出
void Gpt1_Pwm_Setduty(uint8_t duty)
{
    timer_info_t info;
    uint32_t duty_count;

    if(duty > 100)
    {
        duty=100;
    }

    R_GPT_InfoGet(&g_timer1_ctrl, &info);
    duty_count = (info.period_counts * duty) / 100;
    R_GPT_DutyCycleSet(&g_timer1_ctrl, duty_count, GPT_IO_PIN_GTIOCA);
}

void Gpt2_Pwm_Setduty(uint8_t duty)
{
    timer_info_t info;
    uint32_t duty_count;

    if(duty > 100)
    {
        duty=100;
    }

    R_GPT_InfoGet(&g_timer3_ctrl, &info);
    duty_count = (info.period_counts * duty) / 100;
    R_GPT_DutyCycleSet(&g_timer3_ctrl, duty_count, GPT_IO_PIN_GTIOCA);
}

// gpt2输入捕获
void gpt2_callback(timer_callback_args_t *p_args)
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
            t2 = p_args->capture + overflow_times * gpt2_period;
            pwm_period = t2 - t;
            overflow_times = 0;
            one_period_flag = 0;
        }
        break;
    case TIMER_EVENT_CAPTURE_B:
        if (1 == one_period_flag)
        {
            t1 = p_args->capture + overflow_times * gpt2_period;
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
*/
