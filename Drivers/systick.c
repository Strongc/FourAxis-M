#include "systick.h"

static __IO u32 TimingDelay = 0;

/*
 * ��������SysTick_Init
 * ����  ������ϵͳ�δ�ʱ��
 * ����  ����
 * ���  ����
 */
void Systick_Init(void)
{
	//SystemCoreClock / 1000    1ms�ж�һ��
	//SystemCoreClock / 500	    2ms�ж�һ��
	 
	if (SysTick_Config(SystemCoreClock / 1000))
	{ 
		while (1);
	}		
	//SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;//�رյδ�ʱ��  	
	//SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk; //ʹ�ܵδ�ʱ��  
}

