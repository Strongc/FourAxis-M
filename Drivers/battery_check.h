#ifndef __BATTERY_CHECK_H
#define __BATTERY_CHECK_H

#include "stm32f10x.h"

#define BATTERY_FILTER_LENGTH 10			//��ػ����˲�����

void ADC_Battery_Init(void);
u16 ADC_ReadValue(u8 ch);  
float Battery_Voltage_ReadValue(void);

#endif


