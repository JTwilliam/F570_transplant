/*
 * bsp_joystick.h
 *
 *  Created on: 2026年1月27日
 *      Author: huago
 */

#ifndef JOYSTICK_BSP_JOYSTICK_H_
#define JOYSTICK_BSP_JOYSTICK_H_

#include "hal_data.h"
#include <stdint.h>
#include <string.h>
#include <math.h>

/************************ 核心配置宏定义（手柄侧摇杆控制） ************************/
// 摇杆ADC通道映射（与硬件绑定）
#define ADC_CH_LEFT_X    ADC_CHANNEL_1   // 左X→偏航（YAW：-80~+80）
#define ADC_CH_LEFT_Y    ADC_CHANNEL_2   // 左Y→油门（THR：0~90，50悬停）
#define ADC_CH_RIGHT_X   ADC_CHANNEL_3   // 右X→横滚（ROLL：-80~+80）
#define ADC_CH_RIGHT_Y   ADC_CHANNEL_4   // 右Y→俯仰（PITCH：-80~+80）

// 核心控制参数（与飞控适配）
#define JOY_DEAD_ZONE    30      // 摇杆死区阈值（解决漂移）
#define JOY_MAX_VALUE    4095    // 12位ADC最大值
#define THR_MIN          0       // 最小油门（飞控识别下限）
#define THR_MAX          90      // 最大油门（飞控识别上限）
#define THR_HOVER        50      // 悬停基准油门
#define ATTITUDE_MAX     80      // 最大姿态角（飞控限幅）
#define SAFETY_THRESHOLD 20      // 安全阈值：油门<20时姿态指令无效
#define SAMPLE_PERIOD    20      // 采集周期（20ms，与飞控控制周期对齐）

/************************ 数据结构定义（飞控可识别） ************************/
// 摇杆控制指令结构体（输出给飞控的标准化指令）
typedef struct
{
    uint8_t  thr;    // 油门（0~90）
    int16_t  yaw;    // 偏航（-80~+80）
    int16_t  roll;   // 横滚（-80~+80）
    int16_t  pitch;  // 俯仰（-80~+80）
} Joystick_Cmd_t;

// 飞控通信帧结构体（带校验，保证传输可靠）
typedef struct
{
    uint8_t  header;  // 帧头（固定0xAA，飞控同步用）
    uint8_t  thr;     // 油门
    int16_t  yaw;     // 偏航
    int16_t  roll;    // 横滚
    int16_t  pitch;   // 俯仰
    uint8_t  crc8;    // CRC8校验位
    uint8_t  tail;    // 帧尾（固定0x55，飞控同步用）
} Cmd_Frame_t;

/************************ 手柄侧核心功能 ************************/


/**
 * @brief 摇杆零位校准（上电必执行，消除个体差异）
 */
void Joystick_Calibration(void);



/**
 * @brief 采集并处理摇杆数据，生成飞控可识别的控制指令
 * @return 标准化摇杆控制指令
 */
void Joystick_Get_Cmd(Joystick_Cmd_t *cmd);



/**
 * @brief 发送摇杆指令给飞控（增量发送+CRC校验）
 */
void Joystick_Send_Cmd(void);

void Joystick_To_PS2_Format(Joystick_Cmd_t *cmd, uint8_t ps2_data[4]);


#endif /* JOYSTICK_BSP_JOYSTICK_H_ */
