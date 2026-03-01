/*
 * bsp_dshot.c
 *
 *  Created on: 2026年1月28日
 *      Author: huago
 */
#include "bsp_dshot.h"
#include "gpt/bsp_gpt.h"

// 全局电机接口实例
MotorInterface_t UserDshotMotor = {
    .init = motor_init,
    .set_target = pwmWriteDigital
};


 //CRC校验计算

uint16_t DshotDecode(dshotMotorVal_t val)
{
    // 1. 油门值限幅（0~2047）
    uint16_t throttle = val.throttle > 2047 ? 2047 : val.throttle;
    // 2. 拼接12位packet（11位油门+1位遥测）
    uint16_t packet = (throttle << 1) | (val.Telemetry & 0x01);
    // 3. 计算4位CRC
    uint8_t crc = (packet ^ (packet >> 4) ^ (packet >> 8)) & 0x0F;
    // 4. 拼接16位最终数据包
    return (packet << 4) | crc;
}

/*uint16_t DshotDecode(dshotMotorVal_t val)
{
    uint16_t packet = (val.throttle << 1) | (val.Telemetry ? 1 : 0);
    uint8_t crc = (packet ^ (packet >> 4) ^ (packet >> 8)) & 0x0F;
    return (packet << 4) | crc;
}*/


//编码并发送Dshot指令

void pwmWriteDigital(dshotMotorVal_t m1, dshotMotorVal_t m2, dshotMotorVal_t m3, dshotMotorVal_t m4)
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

    // 转换为PWM占空比（高位先行）
    for(uint8_t i = 0; i < 16; i++)
    {
        g_esc_cmd[0][i] = ((m1.throttle >> (15 - i)) & 0x01) ? ESC_BIT_1 : ESC_BIT_0;
        g_esc_cmd[1][i] = ((m2.throttle >> (15 - i)) & 0x01) ? ESC_BIT_1 : ESC_BIT_0;
        g_esc_cmd[2][i] = ((m3.throttle >> (15 - i)) & 0x01) ? ESC_BIT_1 : ESC_BIT_0;
        g_esc_cmd[3][i] = ((m4.throttle >> (15 - i)) & 0x01) ? ESC_BIT_1 : ESC_BIT_0;
    }

    // 中断模式发送
    while(g_esc_index != ESC_CMD_BUF_LEN);
    g_esc_index = 0;
    bsp_gpt_base_start_it();
}

//电机驱动初始化
void motor_init(void)
{
    bsp_gpt_init();

    // 启动4路PWM输出
    pwm_start(DSHOT_GPT1_INSTANCE, GPT_IO_PIN_GTIOCA); // M1(GPT1-A)
    pwm_start(DSHOT_GPT1_INSTANCE, GPT_IO_PIN_GTIOCB); // M2(GPT1-B)
    pwm_start(DSHOT_GPT3_INSTANCE, GPT_IO_PIN_GTIOCA); // M3(GPT3-A)
    pwm_start(DSHOT_GPT3_INSTANCE, GPT_IO_PIN_GTIOCB); // M4(GPT3-B)
}
