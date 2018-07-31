#include "includes.h"



//全局变量初始化
void GloVar_Init(void)
{
	uint16_t i = 0;
	//获取当前软件版本信息
	for(i=0;i<sizeof(ROM_FLASH_PARAM);i++)
	{
		*((uint8_t*)(&updatDataRam) + i)=*((uint8_t*)(UPDATAPARA_ADDR) + i);
	}
	//便携王发送信息初始化	
	
	
	
	//清空VCI CAN通讯参数区
	memset(&canVCIData,0,sizeof(VCICAN_Data));
	
	
	varOperaRun.isVCIchangeCANTrue = 1;
}
void LS595Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOD, GPIO_Pin_15);
	
	GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9);

}
void delay(uint16_t z)
{
	int i,j;
	for(i=z;i>0;i--)
		for(j=0;j<10;j++);
}
extern OS_EVENT * lockMux595;		//595 互斥锁
void LS595Deal(u16 data595)
{
	u8 i;
	u8 err;
	OSMutexPend(lockMux595,0,&err);	//互斥，防止其他任务打断
	
	for (i=0x00;i<16;i++)
	{
		if ((data595<<i)&0x8000)
			SDI_595 = 1;
		else 
			SDI_595 = 0;
		delay(2);
		HCP_595=1;
		delay(1);
		HCP_595=2;

	}
	delay(2);
	TCP_595=1;
	delay(2);
	TCP_595=0;
	delay(2);
	
	OSMutexPost(lockMux595);
	
}
//将数据进行封包
void PackageDeal(u8 * ptrDataToSend)
{
	
}
//校验CheckSum
uint8_t RecvPackAnalysis(u8 *ptrDataToDeal,u8 mode)
{
	uint16_t offset = 0;
	uint16_t datLen = 0;
	uint16_t sumDat = 0;
	uint16_t sumCheck = 0;
	uint16_t i,dat;
	while(1)
	{
		if(ptrDataToDeal[offset++] == 0x7E) //找到帧头
			break;
		if(offset > 8)
			return 1;
	}
	datLen   = ptrDataToDeal[offset+1];		//获取帧长度
	datLen   = (datLen << 8) + ptrDataToDeal[offset];		
	for(i=0;i<datLen;i+=2)
	{
		dat = ptrDataToDeal[offset+i+1];
		dat = (dat<<8)+ptrDataToDeal[offset+i];
		sumCheck += dat;				    //计算的CheckSum
	}
	if(mode == 1)//校验蓝牙接收的帧CheckSum
	{
		sumDat   = ptrDataToDeal[offset+datLen+1];//帧中的CheckSum
		sumDat   = (sumDat << 8) + ptrDataToDeal[offset+datLen];
		
		if(sumCheck != sumDat)					//比较CheckSum

			return 2;
	}else 		//生成蓝牙发送的帧 CheckSum 以及帧尾 0x7E
	{
		ptrDataToDeal[offset+datLen]   = (uint8_t)sumCheck        & 0x00FF;
		ptrDataToDeal[offset+datLen+1] = (uint8_t)(sumCheck >> 8) & 0x00FF;
		ptrDataToDeal[offset+datLen+2] = 0x7E;
	}
	
	return 0;
}









