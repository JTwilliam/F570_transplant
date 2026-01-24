#include <UART/uart.h>
#include "stdio.h"
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <assert.h>

/* 初始化指定串口（完全保留，无修改） */
void uart_init(uart_port_t port)
{
    fsp_err_t err = FSP_SUCCESS;

    switch (port)
    {
    case UART_PORT_4:
        err = R_SCI_UART_Open(&g_uart4_ctrl, &g_uart4_cfg);
        assert(FSP_SUCCESS == err);
        break;
    case UART_PORT_5:
           err = R_SCI_UART_Open(&g_uart5_ctrl, &g_uart5_cfg);
           assert(FSP_SUCCESS == err);
           break;
    default:
        break;
    }
}

/* 串口独立发送完成标志（保留之前的修复） */
static volatile bool uart_send_complete_flag_arr[2] = {false};
volatile bool uart_send_complete_flag = false;

/* 串口回调（保留之前的修复） */
void g_uart4_callback(uart_callback_args_t *p_args)
{
    switch (p_args->event)
    {
    case UART_EVENT_RX_CHAR:
        break;
    case UART_EVENT_TX_COMPLETE:
        uart_send_complete_flag = true;
        uart_send_complete_flag_arr[0] = true;
        break;
    default:
        break;
    }
}
void g_uart5_callback(uart_callback_args_t *p_args)
{
    switch (p_args->event)
    {
    case UART_EVENT_RX_CHAR:
        break;
    case UART_EVENT_TX_COMPLETE:
        uart_send_complete_flag = true;
        uart_send_complete_flag_arr[1] = true;
        break;
    default:
        break;
    }
}

// 修复点1：修改m_pow_n的第二个参数为int（匹配实际使用场景，根除警告）
// 简单的整数次幂（m^n），n 为非负
static unsigned long m_pow_n(unsigned long m, int n)
{
    // 增加合法性检查：n为负时返回1（避免错误计算）
    if (n < 0) return 1;

    unsigned long ret = 1;
    for (int i = 0; i < n; i++)
    {
        ret *= m;
    }
    return ret;
}

// 格式化到缓冲区（仅修复警告相关的m_pow_n调用，其余逻辑不变）
static int format_to_buffer(char *outbuf, size_t maxlen, const char *fmt, va_list args)
{
    if (outbuf == NULL || fmt == NULL || maxlen == 0)
        return -1;

    size_t outpos = 0;
    const char *pStr = fmt;

    while (*pStr != '\0' && outpos + 1 < maxlen)
    {
        if (*pStr != '%')
        {
            if (*pStr == '\\' && *(pStr + 1) != '\0')
            {
                char next = *(pStr + 1);
                char putc = 0;
                if (next == 'r')
                    putc = '\r';
                else if (next == 'n')
                    putc = '\n';
                else if (next == 't')
                    putc = '\t';
                else if (next == '\\')
                    putc = '\\';

                if (putc != 0)
                {
                    if (outpos + 1 < maxlen)
                        outbuf[outpos++] = putc;
                    pStr += 2;
                    continue;
                }
            }

            outbuf[outpos++] = *pStr++;
            continue;
        }

        pStr++;
        if (*pStr == '\0')
            break;

        if (*pStr == '%')
        {
            outbuf[outpos++] = '%';
            pStr++;
            continue;
        }

        int precision = -1;
        if (*pStr == '.')
        {
            const char *q = pStr + 1;
            int prec_val = 0;
            int digits = 0;
            while (*q && isdigit((unsigned char)*q))
            {
                prec_val = prec_val * 10 + (*q - '0');
                q++;
                digits++;
            }
            if (digits > 0)
            {
                precision = prec_val;
                pStr = q;
            }
        }

        int ArgIntVal = 0;
        unsigned long ArgHexVal = 0;
        char *ArgStrVal = NULL;
        double ArgFloVal = 0.0;
        unsigned long val_seg = 0;
        unsigned long val_temp = 0;
        int cnt = 0;

        switch (*pStr)
        {
        case 'c':
            ArgIntVal = va_arg(args, int);
            if (outpos + 1 < maxlen)
                outbuf[outpos++] = (char)ArgIntVal;
            pStr++;
            break;
        case 'd':
            ArgIntVal = va_arg(args, int);
            if (ArgIntVal < 0)
            {
                if (outpos + 1 < maxlen)
                    outbuf[outpos++] = '-';
                val_temp = (unsigned long)(-(int64_t)ArgIntVal);
            }
            else
            {
                val_temp = (unsigned long)ArgIntVal;
            }

            if (val_temp)
            {
                unsigned long t = val_temp;
                while (t)
                {
                    cnt++;
                    t /= 10;
                }
            }
            else
                cnt = 1;

            // 修复点2：调用m_pow_n时参数类型匹配（无警告）
            while (cnt && outpos + 1 < maxlen)
            {
                val_seg = val_temp / m_pow_n(10, cnt - 1);
                val_temp %= m_pow_n(10, cnt - 1);
                outbuf[outpos++] = (char)val_seg + '0';
                cnt--;
            }
            pStr++;
            break;
        case 'o':
            ArgIntVal = va_arg(args, int);
            if (ArgIntVal < 0)
            {
                if (outpos + 1 < maxlen)
                    outbuf[outpos++] = '-';
                val_temp = (unsigned long)(-(int64_t)ArgIntVal);
            }
            else
                val_temp = (unsigned long)ArgIntVal;

            if (val_temp)
            {
                unsigned long t = val_temp;
                while (t)
                {
                    cnt++;
                    t /= 8;
                }
            }
            else
                cnt = 1;

            while (cnt && outpos + 1 < maxlen)
            {
                val_seg = val_temp / m_pow_n(8, cnt - 1);
                val_temp %= m_pow_n(8, cnt - 1);
                outbuf[outpos++] = (char)val_seg + '0';
                cnt--;
            }
            pStr++;
            break;
        case 'x':
            ArgHexVal = va_arg(args, unsigned long);
            if (ArgHexVal)
            {
                unsigned long t = ArgHexVal;
                while (t)
                {
                    cnt++;
                    t /= 16;
                }
            }
            else
                cnt = 1;

            while (cnt && outpos + 1 < maxlen)
            {
                val_seg = ArgHexVal / m_pow_n(16, cnt - 1);
                ArgHexVal %= m_pow_n(16, cnt - 1);
                if (val_seg <= 9)
                    outbuf[outpos++] = (char)val_seg + '0';
                else
                    outbuf[outpos++] = (char)(val_seg - 10) + 'A';
                cnt--;
            }
            pStr++;
            break;
        case 'b':
            ArgIntVal = va_arg(args, int);
            val_temp = (unsigned long)ArgIntVal;
            if (val_temp)
            {
                unsigned long t = val_temp;
                while (t)
                {
                    cnt++;
                    t /= 2;
                }
            }
            else
                cnt = 1;

            while (cnt && outpos + 1 < maxlen)
            {
                val_seg = val_temp / m_pow_n(2, cnt - 1);
                val_temp %= m_pow_n(2, cnt - 1);
                outbuf[outpos++] = (char)val_seg + '0';
                cnt--;
            }
            pStr++;
            break;
        case 's':
            ArgStrVal = va_arg(args, char *);
            if (ArgStrVal == NULL)
                ArgStrVal = "(null)";
            while (*ArgStrVal && outpos + 1 < maxlen)
            {
                outbuf[outpos++] = *ArgStrVal++;
            }
            pStr++;
            break;
        case 'f':
            ArgFloVal = va_arg(args, double);
            if (ArgFloVal < 0)
            {
                if (outpos + 1 < maxlen)
                    outbuf[outpos++] = '-';
                ArgFloVal = -ArgFloVal;
            }
            val_seg = (unsigned long)ArgFloVal;
            val_temp = val_seg;

            if (val_seg)
            {
                unsigned long t = val_seg;
                while (t)
                {
                    cnt++;
                    t /= 10;
                }
            }
            else
                cnt = 1;

            while (cnt && outpos + 1 < maxlen)
            {
                val_seg = val_temp / m_pow_n(10, cnt - 1);
                val_temp %= m_pow_n(10, cnt - 1);
                outbuf[outpos++] = (char)val_seg + '0';
                cnt--;
            }

            if (outpos + 1 < maxlen)
                outbuf[outpos++] = '.';

            if (precision < 0)
                precision = 6;
            ArgFloVal = ArgFloVal - (unsigned long)(ArgFloVal);
            {
                unsigned long mult = 1;
                for (int ii = 0; ii < precision; ii++)
                    mult *= 10u;
                ArgFloVal *= (double)mult;
                val_temp = (unsigned long)ArgFloVal;
                cnt = precision;
            }
            while (cnt && outpos + 1 < maxlen)
            {
                val_seg = val_temp / m_pow_n(10, cnt - 1);
                val_temp %= m_pow_n(10, cnt - 1);
                outbuf[outpos++] = (char)val_seg + '0';
                cnt--;
            }
            pStr++;
            break;
        default:
            if (outpos + 1 < maxlen)
                outbuf[outpos++] = ' ';
            pStr++;
            break;
        }
    }

    outbuf[outpos] = '\0';
    return (int)outpos;
}

// uart_printf（保留之前的修复，无警告相关修改）
void uart_printf(uart_port_t uart_num, const char *fmt, ...)
{
    if (fmt == NULL)
        return;

    char buf[512];
    va_list args;
    va_start(args, fmt);
    int len = format_to_buffer(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len <= 0)
        return;

    if (len >= (int)sizeof(buf))
        len = (int)sizeof(buf) - 1;

    volatile bool *p_flag = NULL;
    switch (uart_num)
    {
    case UART_PORT_4:
        p_flag = &uart_send_complete_flag_arr[0];
        break;
    case UART_PORT_5:
        p_flag = &uart_send_complete_flag_arr[1];
        break;
    default:
        return;
    }
    *p_flag = false;

    switch (uart_num)
    {
    case UART_PORT_4:
    {
        fsp_err_t err = R_SCI_UART_Write(&g_uart4_ctrl, (uint8_t *)buf, (uint32_t)len);
        if (err == FSP_SUCCESS)
        {
            unsigned int wait_cnt = 0;
            const unsigned int wait_max = 1000000u;
            while ((*p_flag == false) && (wait_cnt++ < wait_max))
            {
                __NOP();
            }
            *p_flag = false;
        }
        else
        {
            for (int i = 0; i < len; i++)
            {
                *p_flag = false;
                (void)R_SCI_UART_Write(&g_uart4_ctrl, (uint8_t *)&buf[i], 1u);
                unsigned int wait_cnt = 0;
                const unsigned int wait_max = 100000u;
                while ((*p_flag == false) && (wait_cnt++ < wait_max))
                {
                    __NOP();
                }
                *p_flag = false;
            }
        }
    }
    break;
    default:
        break;
    }
}
