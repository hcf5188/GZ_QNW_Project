#include "sys.h"
#include "bsp.h"
#include "includes.h"
extern OS_EVENT * CANRecvQ;				//CAN 接收消息队列的指针
/**********************************************************************************
功能说明：CAN1 FIFO0 接收中断
***********************************************************************************/
CanRxMsg *RxMessage1;
uint8_t err;
void CAN1_RX0_IRQHandler(void)
{   
	OSIntEnter();//系统进入中断服务程序
	RxMessage1 = Mem_malloc(sizeof(CanRxMsg));
	if(RxMessage1 != NULL)
	{
		CAN_Receive(CAN1, CAN_FIFO0,RxMessage1);//
		OSQPost(CANRecvQ,RxMessage1);
	}
	OSIntExit();  //中断服务结束，系统进行任务调度
}

/***********************************************************************************
功能说明：CAN2 FIFO1 接收中断
***********************************************************************************/
CanRxMsg *RxMessage2;

void CAN2_RX1_IRQHandler(void)
{
	OSIntEnter();//系统进入中断服务程序
	RxMessage2 = Mem_malloc(sizeof(CanRxMsg));
	if(RxMessage2 != NULL)
	{
		CAN_Receive(CAN2, CAN_FIFO1,RxMessage2);
		OSQPost(CANRecvQ,RxMessage2);
	}
	OSIntExit();  //中断服务结束，系统进行任务调度
}


/*************************************************************************************
功能说明：CAN1发送函数
*************************************************************************************/
void Can1_SendData(u32 canId,u32 ide,u8* pdat)
{
	CanTxMsg TxMessage;
	
	TxMessage.StdId = canId;   
	TxMessage.ExtId = canId;
	TxMessage.IDE 	= ide;
	TxMessage.RTR 	= CAN_RTR_DATA;
	TxMessage.DLC 	= 8;
	memcpy(TxMessage.Data,pdat,8);
	
	CAN_Transmit(CAN1,&TxMessage);
}
/***********************************************************************************
功能说明：CAN2发送函数
************************************************************************************/
void Can2_SendData(u32 canId,u32 ide,u8* pdat)
{
	CanTxMsg TxMessage;
	
	TxMessage.StdId = canId;   
	TxMessage.ExtId = canId;
	TxMessage.IDE 	= ide;
	TxMessage.RTR 	= CAN_RTR_DATA;
	TxMessage.DLC 	= 8;
	memcpy(TxMessage.Data,pdat,8);
	
	CAN_Transmit(CAN2,&TxMessage);
}


void CAN_SetBaud(CAN_TypeDef * CAN_BASE,uint32_t baud);
void CAN1SetFilter(uint32_t canId,uint32_t canIde,uint32_t mask);
void CAN2SetFilter(uint32_t canId,uint32_t canIde,uint32_t mask);
/*********************************************************************************
功能说明：CAN1模式初始化配置
          CAN1的过滤器为0~13，过滤器关联到 FIFO0
*********************************************************************************/
void CAN1Config(uint32_t canId,uint32_t canIde,uint32_t mask,uint32_t baud)
{

	GPIO_InitTypeDef GPIO_InitStructure; 
	NVIC_InitTypeDef NVIC_InitStructure;
	
	static uint32_t canIdOld    = 0;
	static uint32_t canIdeOld   = 0;
	static uint32_t maskOld     = 0;
	static uint32_t baudOld = CANBAUD_100K;
	//不用重复设置
	if(canIdOld == canId && canIdeOld == canIde && maskOld == mask &&baudOld == baud)
		return;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);//使能PORTD时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//使能CAN1时钟        

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;             
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
	GPIO_Init(GPIOD, &GPIO_InitStructure);			//初始化IO    CAN1 TX

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);       	//初始化IO       CAN1 RX
	GPIO_PinRemapConfig(GPIO_Remap2_CAN1 , ENABLE); //CAN1完全重映射到PD0,PD1

	CAN_SetBaud(CAN1,baud);				//CAN 波特率设置

	CAN1SetFilter(canId,canIde,mask);	//CAN滤波器设置
	
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	// 主优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			// 次优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);//FIFO0消息挂号中断允许.                                                    
}  
/**********************************************************************************
功能说明：CAN2模式初始化配置
          CAN2的过滤器配置为14~28，过滤器关联到FIFO 1
**********************************************************************************/
void CAN2Config(uint32_t canId,uint32_t canIde,uint32_t mask,uint32_t baud)
{
	static uint32_t canIdOld    = 0;
	static uint32_t canIdeOld   = 0;
	static uint32_t maskOld     = 0;
	static uint32_t baudOld 	= 0;
	GPIO_InitTypeDef GPIO_InitStructure; 
	NVIC_InitTypeDef  NVIC_InitStructure;
	
	if(canIdOld == canId && canIdeOld == canIde && maskOld == mask &&baudOld == baud)
		return;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能PORTB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能CAN 管脚复用 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//使能CAN1时
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);//使能CAN2时
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;             
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//复用推挽
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//初始化IO    CAN2 TX

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//初始化IO       CAN2 RX
		
	
	
	CAN_SetBaud(CAN2,baud);
	CAN2SetFilter(canId,canIde,mask);

	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;	// 主优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			// 次优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	CAN_ITConfig(CAN2,CAN_IT_FMP1,ENABLE);
} 
void CAN_SetBaud(CAN_TypeDef * CAN_BASE,uint32_t baud)
{
	CAN_InitTypeDef         CAN_InitStructure;
	
	CAN_DeInit(CAN_BASE); //复位CAN寄存器
	CAN_StructInit(&CAN_InitStructure);  //填充CAN参数结构体成员为缺省值
	
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//正常模式 CAN_Mode_LoopBack CAN_Mode_Normal
	//设置 波特率
	//CAN 波特率 = RCC_APB1Periph_CAN / Prescaler /(sjw + bs1 +bs2)
	//CAN 波特率 = 36000000/8/(1+4+4)=500kbps
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_9tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
	switch(baud)
	{
		case 100000:CAN_InitStructure.CAN_Prescaler		= 20;break;
		case 250000:CAN_InitStructure.CAN_Prescaler		= 8;break;
		case 500000:CAN_InitStructure.CAN_Prescaler		= 4;break;
		case 1000000:CAN_InitStructure.CAN_Prescaler	= 2;break;
		default:break;
	}
	CAN_Init(CAN_BASE, &CAN_InitStructure);
}
//设置CAN1滤波器
void CAN1SetFilter(uint32_t canId,uint32_t canIde,uint32_t mask)
{
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber=0;          //过滤器0
	CAN_FilterInitStructure.CAN_FilterMode  = CAN_FilterMode_IdMask;  //标识符屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; //过滤器位宽32位 
		
	if(canIde == CAN_ID_STD)//标准帧
	{
		CAN_FilterInitStructure.CAN_FilterIdHigh   = (((u32)canId<<21)&0xffff0000)>>16;
		CAN_FilterInitStructure.CAN_FilterIdLow    = (((u32)canId<<21)|CAN_ID_STD)&0xffff;

	}else					//扩展帧
	{
		CAN_FilterInitStructure.CAN_FilterIdHigh   = (((u32)canId<<3)&0xffff0000)>>16;
		CAN_FilterInitStructure.CAN_FilterIdLow    = (((u32)canId<<3)|CAN_ID_EXT)&0xffff;
	}
	
	mask <<= 3;//留出 IDE RTR 0 位
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh  = (mask >> 16)&0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow   = (mask&0xFFFF)|0x02;		//只接收数据帧
	
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;	//过滤器0关联到FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation  = ENABLE; 				//激活过滤器0
	CAN_FilterInit(&CAN_FilterInitStructure);							//滤波器初始化 CAN1
}

//设置CAN2滤波器
void CAN2SetFilter(uint32_t canId,uint32_t canIde,uint32_t mask)
{
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 14;		//过滤器14
	CAN_FilterInitStructure.CAN_FilterMode  = CAN_FilterMode_IdMask; 
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; //32位 
	
	if(canIde == CAN_ID_STD)//标准帧
	{
		CAN_FilterInitStructure.CAN_FilterIdHigh   = (((u32)canId<<21)&0xffff0000)>>16;
		CAN_FilterInitStructure.CAN_FilterIdLow    = (((u32)canId<<21)|CAN_ID_STD)&0xffff;

	}else					//扩展帧
	{
		CAN_FilterInitStructure.CAN_FilterIdHigh   = (((u32)canId<<3)&0xffff0000)>>16;
		CAN_FilterInitStructure.CAN_FilterIdLow    = (((u32)canId<<3)|CAN_ID_EXT)&0xffff;
	}
	
	mask <<= 3;//留出 IDE RTR 0 位
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh  = (mask >> 16)&0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow   = (mask&0xFFFF)|0x02;		//只接收数据帧
	
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO1;	//过滤器0关联到FIFO1
	CAN_FilterInitStructure.CAN_FilterActivation    =ENABLE;			//激活过滤器0
	CAN_FilterInit(&CAN_FilterInitStructure);							//滤波器初始化 CAN2
}





















