#include "sys.h"
#include "bsp.h"
#include "includes.h"
/************          Ƭѡ����           ************/
#define CSH GPIO_SetBits(GPIOA,GPIO_Pin_15)		//
#define CSL GPIO_ResetBits(GPIOA,GPIO_Pin_15)	//ѡ��ADS1243



/************         ��������            ************/
void ADS1243_Delay(uint16_t cnt);
u32 SPI_read_reg(u16 reg);
void SPI_write_reg(u16 reg,u16 data);
u16 SPI3_SendByte(u16 byte);
u32 ADS1243_ReadData(void);

void CheckReady(void);

void SPI3_ConfigInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	SPI_InitTypeDef  SPI_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	/******************************MISO**************************************/
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;//GPIO_Mode_IPU;//GPIO_Mode_IN_FLOATING;//GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*****************************MOSI AND SCK********************************/
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*****************************NSS******************************************/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	/*****************************DRDY****************************************/
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
//	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);	
 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode 		= SPI_Mode_Master;	//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize	= SPI_DataSize_8b;	//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL		= SPI_CPOL_Low;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ
	SPI_InitStructure.SPI_CPHA		= SPI_CPHA_2Edge;	//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS		= SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; //���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit			= SPI_FirstBit_MSB; //ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial 	= 7; 		//CRCֵ����Ķ���ʽ
	SPI_Init(SPI3, &SPI_InitStructure); 				//����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

	//Enable SPI3 
	SPI_Cmd(SPI3, ENABLE); 
}

void ADS1243_Delay(uint16_t cnt)
{
	uint16_t i,j;
	for(i=0;i<cnt;i++)
		for(j = 0;j < 100;j++);
}
//
u32 SPI_read_reg(u16 reg)
{
	u32 temp_data;
	CSL;
	SPI3_SendByte(ADS1243_RREG|(reg&0x0f));
	SPI3_SendByte(0);
	ADS1243_Delay(100);
	temp_data = SPI3_SendByte(0);
	
	CSH;
	return temp_data;
}
//�� ADS1243 �Ĵ���д������
void SPI_write_reg(u16 reg,u16 data)
{
	CSL;
	ADS1243_Delay(2);
	
	SPI3_SendByte(ADS1243_WREG|(reg&0x0f));	//д�Ĵ��� 0x50 + �Ĵ�����ַ
	SPI3_SendByte(0);						//дһ���Ĵ���
	SPI3_SendByte(data);					//��Ĵ���дֵ

	CSH;
	ADS1243_Delay(2);
}
//��������
u16 SPI3_SendByte(u16 byte)
{
	 while (!SPI_I2S_GetFlagStatus(SPI3,SPI_I2S_FLAG_TXE));
	 SPI_I2S_SendData(SPI3, byte);
	 while (!SPI_I2S_GetFlagStatus(SPI3,SPI_I2S_FLAG_RXNE));
	 return SPI_I2S_ReceiveData(SPI3);
}

void ADS1243_Init1(void)
{
	CSL;	SPI3_SendByte(ADS1243_RESET);	CSH;	//��λоƬ
	ADS1243_Delay(10);						
	
	CSL;	SPI3_SendByte(ADS1243_WAKEUP);	CSH;	//ͬ������
	ADS1243_Delay(10);
		
	CSL;	SPI3_SendByte(ADS1243_SELFCAL);	CSH;	//�Զ�У��
	ADS1243_Delay(10);	
	
	CSL;	SPI3_SendByte(ADS1243_SELFOCAL);CSH;
	ADS1243_Delay(10);	
	
	CSL;	SPI3_SendByte(ADS1243_SELFGCAL);CSH;
	ADS1243_Delay(10);
		
	CSL;	SPI3_SendByte(ADS1243_DSYNC);	CSH;	//ͬ������		
	
	ADS1243_Delay(10);	
	SPI_write_reg(ADS1243_ADDER_ACR,0x40);
	SPI_write_reg(ADS1243_ADDER_SETUP,0x00);
}

u32 ADS1243_ReadDataC(u8 channel)
{
	u8 i;
	u32 sum = 0;
	u32 r = 0;
	
	SPI_write_reg(ADS1243_ADDER_MUX,(channel << 4)+0x08);//���·���ÿ��ƼĴ���д ����ͨ��
	SPI_write_reg(ADS1243_ADDER_ACR,0x40);

	OSTimeDlyHMSM(0,0,0,80);	
	CSL;
	SPI3_SendByte(0x01);
	ADS1243_Delay(2);
	for (i=0;i<3;i++)
	{
		r = SPI3_SendByte(0);
		sum = (sum << 8) + r;
	}
	CSH;
	r = SPI_read_reg(2);
	return sum;
}

void CheckReady(void)
{
	static u8 drdy;
	do{
		drdy = SPI_read_reg(ADS1243_ADDER_ACR);
	}while(drdy&0x80);
}

typedef float  fp32;
fp32 cal_k_100[]	= {1.005 ,1.005 ,1.013 ,1.0125,1.012 ,1.0117,1.0114,1.0125,1.0122};
fp32 cal_k_1k[]		= {1.012 ,1.012 ,1.012 ,1.0125,1.0124,1.0126,1.0131,1.0132,1.0134};
fp32 cal_k_10k[]	= {1.014 ,1.0174,1.0194,1.023 ,1.026 ,1.0294,1.032 ,1.035 ,1.038};
fp32 cal_k_100k[]	= {1.0414,1.073 ,1.105 ,1.14  ,1.18  ,1.23  ,1.27  ,1.33  ,1.36};
fp32 cal_k_1m[]		= {1.44  ,1.45   };

u32 CAL_DATA(u32 data)
{
	static u32 C_D_temp_data;
	if (data<110)
	{
		return data;
	}
	else if (data>110&data<1000)
	{
		C_D_temp_data = data/cal_k_100[(data/100)-1];
		return C_D_temp_data;
	}
	else if (data<10000)
	{				
		C_D_temp_data = data/cal_k_1k[(data/1000)-1];
		return C_D_temp_data;		
	}
	else if (data<100000)
	{
		C_D_temp_data = data/cal_k_10k[(data/10000)-1];
		return C_D_temp_data;	
	}
	else if (data<1000000)
	{
		C_D_temp_data = data/cal_k_100k[(data/100000)-1];
		return C_D_temp_data;	
	}
	else if (data<10000000)
	{
		C_D_temp_data = data/cal_k_1m[(data/100000)-1];
		return C_D_temp_data;
	}
	else 
	{
		return data;
	}
}








