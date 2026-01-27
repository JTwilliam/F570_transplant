/*
 * bsp_adc.h
 *
 *  Created on: 2026��1��22��
 *      Author: huago
 */

#ifndef ADC_BSP_ADC_H_
#define ADC_BSP_ADC_H_

#include "hal_data.h"
#include "UART/uart.h"

extern uint16_t g_adc_ch1_data;
extern uint16_t g_adc_ch2_data;
extern uint16_t g_adc_ch3_data;
extern uint16_t g_adc_ch4_data;

extern volatile bool scan_all_complete_flag;


void ADC_Init(void);
void adc_callback(adc_callback_args_t *p_args);
void Read_Adc_Value(void);

#endif /* ADC_BSP_ADC_H_ */


/*
extern adc_voltage_t adc_volt ;

typedef struct
    {
        double ch0_voltage;  // CH0电压值
        double ch1_voltage;  // CH1电压值
    } adc_voltage_t;*/
