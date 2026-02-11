/*
 * bsp_gpt.h
 *
 *  Created on: 2026年1月21日
 *      Author: huago
 */

#ifndef __BSP_GPT_H__
#define __BSP_GPT_H__

#include "hal_data.h"       // FSP核心配置（含错误码/类型）
#include "r_timer_api.h"    // Timer/GPT核心类型
#include "r_gpt.h"          // GPT专属API

// ************************ 核心配置 ************************
#define DSHOT_GPT1_INSTANCE    &g_timer1_ctrl // FSP生成的GPT1控制块
#define DSHOT_GPT3_INSTANCE    &g_timer3_ctrl // FSP生成的GPT3控制块
#define DSHOT_GPT_PERIOD_VALUE  3999U          // Dshot300周期（120MHz/300000 -1）
#define ESC_CMD_BUF_LEN        16U            // Dshot缓冲区长度
// ********************************************************

// Dshot位定义（75%=BIT1，37.5%=BIT0）
#define ESC_BIT_1               (DSHOT_GPT_PERIOD_VALUE * 3 / 4)
#define ESC_BIT_0               (DSHOT_GPT_PERIOD_VALUE * 3 / 8)

// 全局变量声明
extern uint32_t gpt4_period;
extern volatile uint8_t g_esc_index;
extern uint16_t g_esc_cmd[4][ESC_CMD_BUF_LEN];

// 函数声明
void dshot_gpt_update_callback(timer_callback_args_t *p_args);
fsp_err_t bsp_gpt_init(void);
fsp_err_t pwm_start(timer_ctrl_t *p_ctrl, gpt_io_pin_t pin);
fsp_err_t pwm_stop(timer_ctrl_t *p_ctrl, gpt_io_pin_t pin);
fsp_err_t bsp_gpt_set_duty(gpt_io_pin_t channel, uint16_t duty, uint8_t gpt_instance);
fsp_err_t bsp_gpt_base_start_it(void);
fsp_err_t bsp_gpt_base_stop_it(void);

#endif /* __BSP_GPT_H__ */


/*
#ifndef __BSP_GPT_H__
#define __BSP_GPT_H__

#include "hal_data.h"
#include "r_timer_api.h"
#include "r_gpt.h"
#include "fsp_errors.h"

// ************************ 核心配置 ************************
#define DSHOT_GPT1_INSTANCE    &g_timer1_ctrl // FSP生成的GPT1控制块
#define DSHOT_GPT3_INSTANCE    &g_timer3_ctrl // FSP生成的GPT3控制块
#define DSHOT_GPT_CLOCK_FREQ   120000000UL    // PCLK=120MHz（与FSP一致）
#define DSHOT_PWM_PERIOD_US    3.333f         // Dshot300位周期
#define ESC_CMD_BUF_LEN        20U            // Dshot缓冲区长度
// ********************************************************

// 计算GPT周期值（120MHz → 3.333us对应4000个计数，直接用此值避免结构体访问）
#define DSHOT_GPT_PERIOD_VALUE  3999U          // 120MHz/300000 - 1 = 3999
// Dshot位定义（75%=BIT1，37.5%=BIT0）
#define ESC_BIT_1               (DSHOT_GPT_PERIOD_VALUE * 3 / 4)
#define ESC_BIT_0               (DSHOT_GPT_PERIOD_VALUE * 3 / 8)

// 全局变量声明
extern uint32_t gpt4_period;
extern volatile uint8_t g_esc_index;
extern uint16_t g_esc_cmd[4][ESC_CMD_BUF_LEN];

// 函数声明
void dshot_gpt_update_callback(timer_callback_args_t *p_args);
fsp_err_t bsp_gpt_init(void);
fsp_err_t bsp_gpt_pwm_start(gpt_io_pin_t channel);
fsp_err_t bsp_gpt_pwm_stop(gpt_io_pin_t channel);
fsp_err_t bsp_gpt_set_duty(gpt_io_pin_t channel, uint16_t duty);
fsp_err_t bsp_gpt_base_start_it(void);
fsp_err_t bsp_gpt_base_stop_it(void);

void Gpt_Init(void);
void Gpt_Pwm_Setduty(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void Get_Gpt_Pwm(void);


//void Gpt1_Pwm_Setduty(uint8_t duty);
//void Gpt2_Pwm_Setduty(uint8_t duty);
#endif  GPT_BSP_GPT_H_
*/
