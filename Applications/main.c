/*********BlackHole1 For Study**************
 *	File					:					 main.c
 *	Description		: 	主程序，初始化寄存器
 *  IDE						:			  	 MDK5.14
 *	Version				:	BlackHole1_For_Exploer_V1.0
 *	Author				:	 		  		 INF
 *	Copyright			:	  	【INF】无穷创新
 *	Last Time			:			   2015.12.21								
 *	Support				:		 	 www.infcre.com
 *	QQ群					:   	    168590475
 *  State					:        	 	 open
 *			For infinite creativity
 */
 
#include "main.h"


/*
 * 函数名：System_Init
 * 描述  ：系统初始化函数
 * 输入  ：无
 * 输出  ：无
 */  
void System_Init(void)
{
	FLASH_Unlock();											//解锁flash,是否安全更多考虑
	EE_Init();													//初始化模拟eeprom设置	
	Param_Init();												//参数初始化
	LED_Init();													//LED状态灯初始化
	PWM_Out_Init(3999,35,0,0,0,0);			//PWM输出初始化，电机PWM频率500Hz
	USART3_Init(460800);								//串口初始化
	ADC_Battery_Init();									//电池电压ADC检测初始化

	while(!MPU6500_Init());							//等待MPU6500初始化成功
	MS5611_Init();											//MS5611初始化

	LED_StartShow();										//两个作用：一是延时，二是状态显示
	Systick_Init();											//开启1ms中断
	PWM_In_Init(0xffff,72	-	1);       	//开启输入捕获，定时精度为1us
}

/*
 * 函数名：main
 * 描述  ：主函数
 * 输入  ：无
 * 输出  ：无
 */  
int main(void)
{			
	System_Init();										//系统初始化		
	while(1)													//空循环
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

