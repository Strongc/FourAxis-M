/*********BlackHole1 For Study**************
 *	File					:					 main.c
 *	Description		: 	�����򣬳�ʼ���Ĵ���
 *  IDE						:			  	 MDK5.14
 *	Version				:	BlackHole1_For_Exploer_V1.0
 *	Author				:	 		  		 INF
 *	Copyright			:	  	��INF�������
 *	Last Time			:			   2015.12.21								
 *	Support				:		 	 www.infcre.com
 *	QQȺ					:   	    168590475
 *  State					:        	 	 open
 *			For infinite creativity
 */
 
#include "main.h"


/*
 * ��������System_Init
 * ����  ��ϵͳ��ʼ������
 * ����  ����
 * ���  ����
 */  
void System_Init(void)
{
	FLASH_Unlock();											//����flash,�Ƿ�ȫ���࿼��
	EE_Init();													//��ʼ��ģ��eeprom����	
	Param_Init();												//������ʼ��
	LED_Init();													//LED״̬�Ƴ�ʼ��
	PWM_Out_Init(3999,35,0,0,0,0);			//PWM�����ʼ�������PWMƵ��500Hz
	USART3_Init(460800);								//���ڳ�ʼ��
	ADC_Battery_Init();									//��ص�ѹADC����ʼ��

	while(!MPU6500_Init());							//�ȴ�MPU6500��ʼ���ɹ�
	MS5611_Init();											//MS5611��ʼ��

	LED_StartShow();										//�������ã�һ����ʱ������״̬��ʾ
	Systick_Init();											//����1ms�ж�
	PWM_In_Init(0xffff,72	-	1);       	//�������벶�񣬶�ʱ����Ϊ1us
}

/*
 * ��������main
 * ����  ��������
 * ����  ����
 * ���  ����
 */  
int main(void)
{			
	System_Init();										//ϵͳ��ʼ��		
	while(1)													//��ѭ��
	{			
		
  }	 			  
}
		
		






#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
		/* User can add his own implementation to report the file name and line number,
			 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

		/* Infinite loop */
		while (1)
		{
		}
}

#endif

