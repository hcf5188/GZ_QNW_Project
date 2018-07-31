#include "sys.h"
#include "bsp.h"
#include "includes.h"
/************          片选引脚           ************/
#define CSH GPIO_SetBits(GPIOA,GPIO_Pin_15)		//
#define CSL GPIO_ResetBits(GPIOA,GPIO_Pin_15)	//选中ADS1243



/************         函数声明            ************/
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
 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode 		= SPI_Mode_Master;	//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize	= SPI_DataSize_8b;	//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL		= SPI_CPOL_Low;		//串行同步时钟的空闲状态为低电平
	SPI_InitStructure.SPI_CPHA		= SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS		= SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; //定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit			= SPI_FirstBit_MSB; //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial 	= 7; 		//CRC值计算的多项式
	SPI_Init(SPI3, &SPI_InitStructure); 				//根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

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
//向 ADS1243 寄存器写入数据
void SPI_write_reg(u16 reg,u16 data)
{
	CSL;
	ADS1243_Delay(2);
	
	SPI3_SendByte(ADS1243_WREG|(reg&0x0f));	//写寄存器 0x50 + 寄存器地址
	SPI3_SendByte(0);						//写一个寄存器
	SPI3_SendByte(data);					//向寄存器写值

	CSH;
	ADS1243_Delay(2);
}
//发送数据
u16 SPI3_SendByte(u16 byte)
{
	 while (!SPI_I2S_GetFlagStatus(SPI3,SPI_I2S_FLAG_TXE));
	 SPI_I2S_SendData(SPI3, byte);
	 while (!SPI_I2S_GetFlagStatus(SPI3,SPI_I2S_FLAG_RXNE));
	 return SPI_I2S_ReceiveData(SPI3);
}

void ADS1243_Init1(void)
{
	CSL;	SPI3_SendByte(ADS1243_RESET);	CSH;	//复位芯片
	ADS1243_Delay(10);						
	
	CSL;	SPI3_SendByte(ADS1243_WAKEUP);	CSH;	//同步唤醒
	ADS1243_Delay(10);
		
	CSL;	SPI3_SendByte(ADS1243_SELFCAL);	CSH;	//自动校正
	ADS1243_Delay(10);	
	
	CSL;	SPI3_SendByte(ADS1243_SELFOCAL);CSH;
	ADS1243_Delay(10);	
	
	CSL;	SPI3_SendByte(ADS1243_SELFGCAL);CSH;
	ADS1243_Delay(10);
		
	CSL;	SPI3_SendByte(ADS1243_DSYNC);	CSH;	//同步命令		
	
	ADS1243_Delay(10);	
	SPI_write_reg(ADS1243_ADDER_ACR,0x40);
	SPI_write_reg(ADS1243_ADDER_SETUP,0x00);
}

u32 ADS1243_ReadDataC(u8 channel)
{
	u8 i;
	u32 sum = 0;
	u32 r = 0;
	
	SPI_write_reg(ADS1243_ADDER_MUX,(channel << 4)+0x08);//向多路复用控制寄存器写 测试通道
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








