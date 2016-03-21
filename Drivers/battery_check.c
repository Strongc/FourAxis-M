#include "battery_check.h"
#include "delay.h"

u16 adc_value;															//ADC����ֵ
u16 adc_avg;																//ADC����ƽ��ֵ
float battery_voltage;											//��ص�ѹ
u16 Adc_Value[BATTERY_FILTER_LENGTH] = {0};	//�洢Adc��ֵ����

/*
 * ��������ADC_Battery_Init
 * ����  ����ص�ѹADC����ʼ������
 * ����  ����
 * ���  ����
 */ 		 
void  ADC_Battery_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_ADC1,ENABLE);	  		//ʹ��ADC1ͨ��ʱ��
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   																						//����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䳬��14M���ѹ�ɼ���׼ȷ

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);  																															//��λADC1

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;															//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;																		//ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;															//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;							//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;													//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;																					//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);																							//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   

	ADC_Cmd(ADC1, ENABLE);																													//ʹ��ADC1
	
	ADC_ResetCalibration(ADC1);																											//ʹ�ܸ�λУ׼  
	while(ADC_GetResetCalibrationStatus(ADC1));																			//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 																										//����ADУ׼
	while(ADC_GetCalibrationStatus(ADC1));	 																				//�ȴ�У׼����
}				  

/*
 * ��������ADC_ReadValue
 * ����  ��ADCԭʼ���ݶ�ȡ
 * ����  ��ADCͨ��
 * ���  ����
 */
u16 ADC_ReadValue(u8 ch)   
{																																									//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );							//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			      
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);																					//ʹ��ָ����ADC1�����ת����������		 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));																	//�ȴ�ת������
	return ADC_GetConversionValue(ADC1);																						//�������һ��ADC1�������ת�����
}

/*
 * ��������Battery_Voltage_ReadValue
 * ����  ���˲���ĵ�ص�ѹ����������ʱ����Ϊ31us
 * ����  ��ADCͨ��
 * ���  ����
 */
float Battery_Voltage_ReadValue(void) 
{
	static u8 filter_cnt	=	0;
	u8 cnt = 0;
	u32 temp = 0;
	
	adc_value = ADC_ReadValue(ADC_Channel_9);
	Adc_Value[filter_cnt] = adc_value;
	
	filter_cnt++;
	
	for(cnt	=	0;cnt <	BATTERY_FILTER_LENGTH;cnt++)
	{
		temp += Adc_Value[cnt];
	}
	adc_avg = temp / BATTERY_FILTER_LENGTH;
	
	battery_voltage = ((float)adc_avg * 6.6f) / 4096.0f;
	if(filter_cnt	==	BATTERY_FILTER_LENGTH)	filter_cnt = 0;
	return battery_voltage;
}

