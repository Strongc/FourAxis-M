#include "ms5611.h"
#include "ms5611_spi.h"
#include "delay.h"
#include "math.h"

s32 ref_pressure;//�ο���ѹ
s32 ref_altitude;//�ο�����

MS5611_DATA MS5611_Date = {0};

void MS5611_Init(void)
{
	SPI2_Init();
	Delay_Ms(5);
	
	MS5611_CS(0);  //ʹ��MS5611��SPI����
  SPI2_Read_Write_Byte(MS5611_RESET);
	Delay_Ms(5);
	MS5611_CS(1);  //ʧ��MS5611��SPI����

	MS5611_PROM_READ();
	
	MS5611_Altitude_Offset(50);
}


/*******************************************************************************
	��PROM��ȡ����У׼����
	�������ƣ�MS5611_PROM_READ
	���������void
	���������void
*******************************************************************************/
void MS5611_PROM_READ(void)
{
  MS5611_Date.C1 = SPI2_Read_Write_2Bytes(MS5611_PROM_COEFF_1);
  MS5611_Date.C2 = SPI2_Read_Write_2Bytes(MS5611_PROM_COEFF_2);
  MS5611_Date.C3 = SPI2_Read_Write_2Bytes(MS5611_PROM_COEFF_3);
  MS5611_Date.C4 = SPI2_Read_Write_2Bytes(MS5611_PROM_COEFF_4);
  MS5611_Date.C5 = SPI2_Read_Write_2Bytes(MS5611_PROM_COEFF_5);
  MS5611_Date.C6 = SPI2_Read_Write_2Bytes(MS5611_PROM_COEFF_6); 
  //SPI2_Read_Write_2Bytes(MS5611_PROM_CRC); 
}

/*******************************************************************************
	ͨ��MS5611ģ��õ���ѹֵ
	�������ƣ�MS5611_getPressure
	�����������ѹ�ֱ���ϵ��
	���������void
*******************************************************************************/

void MS5611_PressureADC_Conversion(void)
{
	MS5611_CS(0);  //ʹ��MS5611��SPI���� 
  SPI2_Read_Write_Byte(MS5611_D1_OSR_4096);
	MS5611_CS(1);  //ʧ��MS5611��SPI����	
}

void MS5611_Pressure_Calculate(void)
{
  MS5611_Date.D1 = MS5611_SPI_Read_ADC();//�����ѹֵ
  MS5611_Date.P = (MS5611_Date.D1 * MS5611_Date.SENS / 2097152 - MS5611_Date.OFF) / 32768;
}


void MS5611_TemperatureADC_Conversion(void)
{
	MS5611_CS(0);  //ʹ��MS5611��SPI���� 
  SPI2_Read_Write_Byte(MS5611_D2_OSR_4096);
	MS5611_CS(1);  //ʧ��MS5611��SPI����	
}

/*******************************************************************************
	ͨ��MS5611ģ��õ��¶�ֵ
	�������ƣ�MS5611_getTemperature
	����������¶ȷֱ���ϵ��
	���������void
*******************************************************************************/

void MS5611_Temperature_Calculate(void)    
{
  MS5611_Date.D2 = MS5611_SPI_Read_ADC();	
  MS5611_Date.dT = MS5611_Date.D2 - (((u32)MS5611_Date.C5) << 8);
  MS5611_Date.TEMP = 2000 + MS5611_Date.dT * MS5611_Date.C6 / 8388608;
	MS5611_Temp_Compensation();
	//MS5611_SecondOrder_Temp_Compensation();
}

//MS5611�����¶Ȳ���
void MS5611_SecondOrder_Temp_Compensation(void)    
{
	double temp;
	
	MS5611_Date.OFF = (u32)MS5611_Date.C2 * 65536 + ((u32)MS5611_Date.C4 * MS5611_Date.dT) / 128;
  MS5611_Date.SENS = (u32)MS5611_Date.C1 * 32768 + ((u32)MS5611_Date.C3 * MS5611_Date.dT) / 256;
	
	if(MS5611_Date.TEMP < 2000)//���MS5611�¶�С��20���϶�
	{
		MS5611_Date.TEMP2 = (MS5611_Date.dT * MS5611_Date.dT) / 2147483648U;
		temp = (MS5611_Date.TEMP - 2000)^2;
		MS5611_Date.OFF2 = 2.5 * temp;//OFF2 = 5 * (TEMP �C 2000)^2 / 2
		MS5611_Date.SENS2 = 1.25 * temp;//SENS2 = 5 * (TEMP �C 2000)^2/ 2
		
		if(MS5611_Date.TEMP < -1500)//���MS5611�¶�С��-15���϶�
		{
			temp = (MS5611_Date.TEMP + 1500)^2;
			MS5611_Date.OFF2 += 7 * temp;
			MS5611_Date.SENS2 += 5.5 * temp;
		}
	}
	else//���MS5611�¶ȴ���20���϶�
	{
		MS5611_Date.TEMP2 = 0;
		MS5611_Date.OFF2 = 0;
		MS5611_Date.SENS2 = 0;
	}
 
	MS5611_Date.TEMP = MS5611_Date.TEMP - MS5611_Date.TEMP2;
	MS5611_Date.OFF -= MS5611_Date.OFF2;
	MS5611_Date.SENS -= MS5611_Date.SENS2;
}

void MS5611_Temp_Compensation(void)    
{
	MS5611_Date.OFF = (u32)MS5611_Date.C2 * 65536 + ((u32)MS5611_Date.C4 * MS5611_Date.dT) / 128;
  MS5611_Date.SENS = (u32)MS5611_Date.C1 * 32768 + ((u32)MS5611_Date.C3 * MS5611_Date.dT) / 256;
}

/*******************************************************************************
	���ݴ���ѹ������θ߶� 
	�������ƣ�get_altitude
	���������void
	������������θ߶�
*******************************************************************************/


float altitude = 0; 
vs32 height;//��λ:cm
vs32 height_avg;
vs32 Height_Buf[MS5611_FILTER_LENGTH] = {0};

float MS5611_Altitude_Calculate(void)                             
{      
	static u8 filter_cnt	=	0;
	u8 cnt;
	s32	temp;
	
  altitude = 44330 * (1 - pow((MS5611_Date.P) / 101325.0, 0.190295));
  height = (altitude - ref_altitude) * 100;
	//height = 10 * (ref_pressure - MS5611_Date.P);//��λ����	
	
	temp = 0;	
	Height_Buf[filter_cnt] = height;
	filter_cnt++;
	
	for(cnt	=	0;cnt <	MS5611_FILTER_LENGTH;cnt++)
	{
		temp += Height_Buf[cnt];
	}
	height_avg = temp / MS5611_FILTER_LENGTH;
	
	if(filter_cnt	==	MS5611_FILTER_LENGTH)	filter_cnt = 0;
	height = height_avg;
	
	return (altitude);
}

//void	MS5611_Flitter()
//{
//	static u8 filter_cnt	=	0;
//	u8 cnt;
//	s32	temp;
//	
//	temp = 0;
//	
//	Acc_Buf[filter_cnt].X = MPU6500_Acc.X;

//	
//	filter_cnt++;
//	
//	for(cnt	=	0;cnt <	FILTER_LENGTH;cnt++)
//	{
//		Temp.X += Acc_Buf[cnt].X;

//	}
//	Acc_Avg.X = Temp.X / FILTER_LENGTH;
//	
//	if(filter_cnt	==	FILTER_LENGTH)	filter_cnt = 0;
//}


void MS5611_Altitude_Offset(u16 cnt)                             
{
	static double temp;
	u16 i = 0;
	
  for(i = cnt; i > 0; i--)
	{
		MS5611_TemperatureADC_Conversion();
		Delay_Ms(12);
		MS5611_Temperature_Calculate();
		MS5611_PressureADC_Conversion();
		Delay_Ms(12);
		MS5611_Pressure_Calculate();
		
		temp += MS5611_Date.P;
	}	
		
  ref_pressure = temp / cnt;
	ref_altitude = 44330 * (1 - pow((ref_pressure) / 101325.0, 0.190295));
}


