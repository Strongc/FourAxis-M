#include "ms5611_spi.h"
#include "ms5611.h"

/*
 * ��������SPI2_Init
 * ����  ��SPI2��ʼ��
 * ����  ����
 * ���  ����
 */ 
void SPI2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB,ENABLE );//PORTBʱ��ʹ�� 
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2 , ENABLE );//SPI2ʱ��ʹ�� 	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //�MPU9250Ƭѡ�ź�
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//��ʼ��ָ��IO
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15����������� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
  
	MS5611_CS(1);  //MPU9250Ƭѡȡ��
	GPIO_SetBits(GPIOB,GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);  //PB13/14/15����
		
//	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE);  //��λSPI2
//	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE); //ֹͣ��λSPI2
	SPI_Cmd(SPI2,DISABLE);  //SPI2ʧ��
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//SPI����
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//ʱ�����յ�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//���ݲ����ڵ�1��ʱ����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź����������
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ16��36M/128=9M��
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	SPI_Init(SPI2, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
	
	SPI_Cmd(SPI2,ENABLE);//ʹ��SPI2
	SPI2_Read_Write_Byte(0xff);//��������	 

}


/*
 * ��������MS5611_Write_Reg
 * ����  ��SPIд��Ĵ���
 * ����  ��reg:ָ���ļĴ�����ַ��value��д���ֵ
 * ���  ��status������״ֵ̬
 */ 
u8 MS5611_Write_Reg(uint8_t reg,uint8_t value)
{
	uint8_t status;
	MS5611_CS(0);  //ʹ��SPI����
	status=SPI2_Read_Write_Byte(reg); //����д����+�Ĵ�����
	SPI2_Read_Write_Byte(value);//д��Ĵ���ֵ
	MS5611_CS(1);  //��ֹMPU9250
	return(status);//����״ֵ̬
}


/*
 * ��������MS5611_Read_Reg
 * ����  ��SPI��ȡ�Ĵ���
 * ����  ��reg:ָ���ļĴ�����ַ
 * ���  ��reg_val��reg�Ĵ�����ַ��Ӧ��ֵ
 */ 
u8 MS5611_Read_Reg(uint8_t reg)
{
	uint8_t reg_val;
	MS5611_CS(0);  //ʹ��SPI����
	SPI2_Read_Write_Byte(reg|0x80); //���Ͷ�����+�Ĵ�����
	reg_val=SPI2_Read_Write_Byte(0xff);//��ȡ�Ĵ���ֵ
	MS5611_CS(1);  //��ֹMPU9250
	return(reg_val);
}

/*
 * ��������SPI2_Read_Write_Byte
 * ����  ����дһ���ֽ�
 * ����  ��TxData:Ҫд����ֽ�
 * ���  ����ȡ�����ֽ�
 */
u8 SPI2_Read_Write_Byte(uint8_t TxData)
{		
	uint16_t retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
		{
			retry++;
			if(retry > 2000) return 0;
		}			  
	SPI_I2S_SendData(SPI2, TxData); //ͨ������SPIx����һ������
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
	{
		retry++;
		if(retry > 2000) return 0;
	}	  						    
	return SPI_I2S_ReceiveData(SPI2); //����ͨ��SPIx������յ�����					    
}

/*
 * ��������SPI2_Read_Write_2Bytes
 * ����  ����д�����ֽ�
 * ����  ��addr:Ҫд���16bit��
 * ���  ����ȡ����16λ����
 */
u16  SPI2_Read_Write_2Bytes(unsigned char addr)
{
  unsigned char byteH,byteL;
  unsigned int return_value;
	
  MS5611_CS(0);  //ʹ��SPI���� 
  SPI2_Read_Write_Byte(addr);
  byteH = SPI2_Read_Write_Byte(0);
  byteL = SPI2_Read_Write_Byte(0);
  MS5611_CS(1);  //��ֹMPU9250
  return_value = ((unsigned int)byteH<<8) | (byteL);
	
  return(return_value);    
}

/*
 * ��������MS5611_SPI_Read_ADC
 * ����  ����ȡADC���
 * ����  ����
 * ���  ������24λ����ѹ/�¶�
 */
long MS5611_SPI_Read_ADC(void)
{
  unsigned char byteH,byteM,byteL;
  long return_value;
  
  MS5611_CS(0);  //ʹ��SPI���� 
  SPI2_Read_Write_Byte(MS5611_ADC_READ);
  byteH = SPI2_Read_Write_Byte(0);
  byteM = SPI2_Read_Write_Byte(0);
  byteL = SPI2_Read_Write_Byte(0);
  MS5611_CS(1);  //ʹ��SPI���� 
  return_value = (((long)byteH) << 16) | (((long)byteM) << 8) | (byteL);
  return(return_value);
}

