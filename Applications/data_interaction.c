#include "data_interaction.h"
#include "usart.h"
#include "ahrs.h"
#include 	"mpu6500.h"

u8 out_date[OUT_DATA_LENGTH];
extern s16 system_time;//��λ��S
s16 Angle_X,Angle_Y,Angle_Z;

/*
 * ��������Data_Send
 * ����  ���������������ʹ��INF_GCS��Ӧ������ʾ�е���̬ģʽ1
 * ����  ����
 * ���  ����
 */  
void Data_Send(void)
{
	int i = 0;
	u8 out_date_sum = 0;//У��λ
	 
	Angle_X=(s16)(Angle.X * 100);
	Angle_Y=(s16)(Angle.Y * 100);
	Angle_Z=(s16)(Angle.Z * 100);
	
	out_date[0] = 0x55;//֡ͷ 
	out_date[1] = 0x01;//��ͷ
	
	out_date[2] = BYTEH(MPU6500_Gyro.X);
	out_date[3] = BYTEL(MPU6500_Gyro.X);
	out_date[4] = BYTEH(MPU6500_Gyro.Y);
	out_date[5] = BYTEL(MPU6500_Gyro.Y);
	out_date[6] = BYTEH(MPU6500_Gyro.Z);
	out_date[7] = BYTEL(MPU6500_Gyro.Z);
	
	out_date[8] = BYTEH(MPU6500_Acc.X);
	out_date[9] = BYTEL(MPU6500_Acc.X);
	out_date[10] = BYTEH(MPU6500_Acc.Y);
	out_date[11] = BYTEL(MPU6500_Acc.Y);
	out_date[12] = BYTEH(MPU6500_Acc.Z);
  out_date[13] = BYTEL(MPU6500_Acc.Z);

	out_date[14] = BYTEH(Angle_X);
	out_date[15] = BYTEL(Angle_X);
	out_date[16] = BYTEH(Angle_Y);
	out_date[17] = BYTEL(Angle_Y);
	out_date[18] = BYTEH(Angle_Z);
  out_date[19] = BYTEL(Angle_Z);
	
	out_date[20] = BYTEH(system_time);
	out_date[21] = BYTEL(system_time);
	
	for(i = 0;i < (OUT_DATA_LENGTH - 1);i++)//����ֽ�ΪУ���
	{
		out_date_sum += out_date[i];
  }
	out_date[OUT_DATA_LENGTH - 1] = out_date_sum;
	
	for(i = 0;i < OUT_DATA_LENGTH;i++)
	{
		USART_SendData(USART3, out_date[i]);
		while( USART_GetFlagStatus(USART3,USART_FLAG_TC)!= SET);	
	} 
}


