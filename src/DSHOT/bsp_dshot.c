/*
 * bsp_dshot.c
 *
 *  Created on: 2026年1月28日
 *      Author: huago
 */
#include "bsp_dshot.h"
#include "gpt/bsp_gpt.h"
#include "DSHOT/bsp_dshot.h"
#include "UART/uart.h"

// 全局电机接口实例
MotorInterface_t UserDshotMotor = {
    .init = motor_init,
    .set_target = pwmWriteDigital
};


 //CRC校验计算
uint16_t DshotEncode(dshotMotorVal_t val)
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


//编码并发送Dshot指令
void pwmWriteDigital(dshotMotorVal_t m1, dshotMotorVal_t m2, dshotMotorVal_t m3, dshotMotorVal_t m4)
{
    // 油门值限幅（0~2047）
    m1.throttle = (m1.throttle > 2047) ? 2047 : m1.throttle;
    m2.throttle = (m2.throttle > 2047) ? 2047 : m2.throttle;
    m3.throttle = (m3.throttle > 2047) ? 2047 : m3.throttle;
    m4.throttle = (m4.throttle > 2047) ? 2047 : m4.throttle;
    // 编码Dshot数据包
    m1.throttle = (uint16_t) DshotEncode(m1);
    m2.throttle = (uint16_t) DshotEncode(m2);
    m3.throttle = (uint16_t) DshotEncode(m3);
    m4.throttle = (uint16_t) DshotEncode(m4);

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

// 解码 Dshot 数据包
// 返回 0 表示校验失败，1 表示成功，结果通过 out_val 输出
uint8_t DshotPacketDecode(uint16_t packet, dshotMotorVal_t *out_val)
{
    uint8_t crc = packet & 0x0F;
    uint16_t data = (packet >> 4) & 0x0FFF;
    uint8_t calc_crc = (data ^ (data >> 4) ^ (data >> 8)) & 0x0F;
    if (crc != calc_crc)
        return 0; // 校验失败

    out_val->throttle = (data >> 1) & 0x07FF; // 11位油门
    out_val->Telemetry = data & 0x01;         // 1位 Telemetry
    return 1; // 校验成功
}

// 1. 设置目标油门值并编码发送
void set_and_send_dshot(uint16_t throttle, uint8_t telemetry)
{
    dshotMotorVal_t val = { .Telemetry = telemetry, .throttle = throttle };
    // 这里只发送一个电机，其他可按需扩展
    pwmWriteDigital(val, val, val, val);
}

// 2. 解码并串口打印
void decode_and_print_dshot(uint16_t packet)
{
    dshotMotorVal_t val;
    if (DshotPacketDecode(packet, &val))
    {
        uart_printf(UART_PORT_4, "Throttle: %d, Telemetry: %d\r\n", val.throttle, val.Telemetry);
    }
    else
    {
        uart_printf(UART_PORT_4, "error!\r\n");
    }
}
