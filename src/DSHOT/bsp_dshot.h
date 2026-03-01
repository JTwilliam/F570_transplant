/*
 * bsp_dshot.h
 *
 *  Created on: 2026年1月28日
 *      Author: huago
 */

#ifndef __BSP_DSHOT_H__
#define __BSP_DSHOT_H__

#include <stdint.h>
#include "gpt/bsp_gpt.h" // 确保项目include路径包含src目录

// Dshot命令枚举
typedef enum
{
    DSHOT_CMD_MOTOR_STOP = 0,
    DSHOT_CMD_BEEP1 = 1,
    DSHOT_CMD_BEEP2 = 2,
    DSHOT_CMD_BEEP3 = 3,
    DSHOT_CMD_BEEP4 = 4,
    DSHOT_CMD_BEEP5 = 5,
    DSHOT_CMD_ESC_INFO = 6,
    DSHOT_CMD_SPIN_DIRECTION_1 = 7,
    DSHOT_CMD_SPIN_DIRECTION_2 = 8,
    DSHOT_CMD_3D_MODE_OFF = 9,
    DSHOT_CMD_3D_MODE_ON = 10,
    DSHOT_CMD_SETTINGS_REQUEST = 11,
    DSHOT_CMD_SAVE_SETTINGS = 12,
    DSHOT_CMD_SPIN_DIRECTION_NORMAL = 20,
    DSHOT_CMD_SPIN_DIRECTION_REVERSED = 21,
    DSHOT_CMD_LED0_ON = 22,
    DSHOT_CMD_LED1_ON = 23,
    DSHOT_CMD_LED2_ON = 24,
    DSHOT_CMD_LED3_ON = 25,
    DSHOT_CMD_LED0_OFF = 26,
    DSHOT_CMD_LED1_OFF = 27,
    DSHOT_CMD_LED2_OFF = 28,
    DSHOT_CMD_LED3_OFF = 29,
    DSHOT_CMD_MAX = 47
}DSHOT_Cmd_e;

// Dshot电机参数结构体（前置定义，无语法错误）
typedef struct
{
    uint8_t  Telemetry; // 遥测标志位（0/1）
    uint16_t  throttle;  // 油门值（0~2047）
}dshotMotorVal_t;

// 电机驱动接口（前置定义）
typedef struct
{
    void (*init)(void);
    void (*set_target)(dshotMotorVal_t m1, dshotMotorVal_t m2, dshotMotorVal_t m3, dshotMotorVal_t m4);
}MotorInterface_t;

// 函数前置声明（解决未声明错误）
void motor_init(void);
void pwmWriteDigital(dshotMotorVal_t m1, dshotMotorVal_t m2, dshotMotorVal_t m3, dshotMotorVal_t m4);
uint16_t DshotDecode(dshotMotorVal_t val);

// 全局接口实例
extern MotorInterface_t UserDshotMotor;

#endif /* __BSP_DSHOT_H__ */
