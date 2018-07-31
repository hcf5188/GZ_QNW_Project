#include "sys.h"
#include "bsp.h"
#include "includes.h"
extern OS_EVENT * CANRecvQ;				//CAN ������Ϣ���е�ָ��
/**********************************************************************************
����˵����CAN1 FIFO0 �����ж�
***********************************************************************************/
CanRxMsg *RxMessage1;
uint8_t err;
void CAN1_RX0_IRQHandler(void)
{   
	OSIntEnter();//ϵͳ�����жϷ������
	RxMessage1 = Mem_malloc(sizeof(CanRxMsg));
	if(RxMessage1 != NULL)
	{
		CAN_Receive(CAN1, CAN_FIFO0,RxMessage1);//
		OSQPost(CANRecvQ,RxMessage1);
	}
	OSIntExit();  //�жϷ��������ϵͳ�����������
}

/***********************************************************************************
����˵����CAN2 FIFO1 �����ж�
***********************************************************************************/
CanRxMsg *RxMessage2;

void CAN2_RX1_IRQHandler(void)
{
	OSIntEnter();//ϵͳ�����жϷ������
	RxMessage2 = Mem_malloc(sizeof(CanRxMsg));
	if(RxMessage2 != NULL)
	{
		CAN_Receive(CAN2, CAN_FIFO1,RxMessage2);
		OSQPost(CANRecvQ,RxMessage2);
	}
	OSIntExit();  //�жϷ��������ϵͳ�����������
}


/*************************************************************************************
����˵����CAN1���ͺ���
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
����˵����CAN2���ͺ���
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
����˵����CAN1ģʽ��ʼ������
          CAN1�Ĺ�����Ϊ0~13�������������� FIFO0
*********************************************************************************/
void CAN1Config(uint32_t canId,uint32_t canIde,uint32_t mask,uint32_t baud)
{

	GPIO_InitTypeDef GPIO_InitStructure; 
	NVIC_InitTypeDef NVIC_InitStructure;
	
	static uint32_t canIdOld    = 0;
	static uint32_t canIdeOld   = 0;
	static uint32_t maskOld     = 0;
	static uint32_t baudOld = CANBAUD_100K;
	//�����ظ�����
	if(canIdOld == canId && canIdeOld == canIde && maskOld == mask &&baudOld == baud)
		return;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);//ʹ��PORTDʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//ʹ��CAN1ʱ��        

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;             
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������
	GPIO_Init(GPIOD, &GPIO_InitStructure);			//��ʼ��IO    CAN1 TX

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//��������
	GPIO_Init(GPIOD, &GPIO_InitStructure);       	//��ʼ��IO       CAN1 RX
	GPIO_PinRemapConfig(GPIO_Remap2_CAN1 , ENABLE); //CAN1��ȫ��ӳ�䵽PD0,PD1

	CAN_SetBaud(CAN1,baud);				//CAN ����������

	CAN1SetFilter(canId,canIde,mask);	//CAN�˲�������
	
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	// �����ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			// �����ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);//FIFO0��Ϣ�Һ��ж�����.                                                    
}  
/**********************************************************************************
����˵����CAN2ģʽ��ʼ������
          CAN2�Ĺ���������Ϊ14~28��������������FIFO 1
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
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//ʹ��PORTBʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ��CAN �ܽŸ��� 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);//ʹ��CAN1ʱ
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);//ʹ��CAN2ʱ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;             
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//��ʼ��IO    CAN2 TX

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//��ʼ��IO       CAN2 RX
		
	
	
	CAN_SetBaud(CAN2,baud);
	CAN2SetFilter(canId,canIde,mask);

	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;	// �����ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			// �����ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	CAN_ITConfig(CAN2,CAN_IT_FMP1,ENABLE);
} 
void CAN_SetBaud(CAN_TypeDef * CAN_BASE,uint32_t baud)
{
	CAN_InitTypeDef         CAN_InitStructure;
	
	CAN_DeInit(CAN_BASE); //��λCAN�Ĵ���
	CAN_StructInit(&CAN_InitStructure);  //���CAN�����ṹ���ԱΪȱʡֵ
	
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//����ģʽ CAN_Mode_LoopBack CAN_Mode_Normal
	//���� ������
	//CAN ������ = RCC_APB1Periph_CAN / Prescaler /(sjw + bs1 +bs2)
	//CAN ������ = 36000000/8/(1+4+4)=500kbps
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
//����CAN1�˲���
void CAN1SetFilter(uint32_t canId,uint32_t canIde,uint32_t mask)
{
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber=0;          //������0
	CAN_FilterInitStructure.CAN_FilterMode  = CAN_FilterMode_IdMask;  //��ʶ������λģʽ
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; //������λ��32λ 
		
	if(canIde == CAN_ID_STD)//��׼֡
	{
		CAN_FilterInitStructure.CAN_FilterIdHigh   = (((u32)canId<<21)&0xffff0000)>>16;
		CAN_FilterInitStructure.CAN_FilterIdLow    = (((u32)canId<<21)|CAN_ID_STD)&0xffff;

	}else					//��չ֡
	{
		CAN_FilterInitStructure.CAN_FilterIdHigh   = (((u32)canId<<3)&0xffff0000)>>16;
		CAN_FilterInitStructure.CAN_FilterIdLow    = (((u32)canId<<3)|CAN_ID_EXT)&0xffff;
	}
	
	mask <<= 3;//���� IDE RTR 0 λ
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh  = (mask >> 16)&0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow   = (mask&0xFFFF)|0x02;		//ֻ��������֡
	
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;	//������0������FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation  = ENABLE; 				//���������0
	CAN_FilterInit(&CAN_FilterInitStructure);							//�˲�����ʼ�� CAN1
}

//����CAN2�˲���
void CAN2SetFilter(uint32_t canId,uint32_t canIde,uint32_t mask)
{
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	CAN_FilterInitStructure.CAN_FilterNumber = 14;		//������14
	CAN_FilterInitStructure.CAN_FilterMode  = CAN_FilterMode_IdMask; 
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; //32λ 
	
	if(canIde == CAN_ID_STD)//��׼֡
	{
		CAN_FilterInitStructure.CAN_FilterIdHigh   = (((u32)canId<<21)&0xffff0000)>>16;
		CAN_FilterInitStructure.CAN_FilterIdLow    = (((u32)canId<<21)|CAN_ID_STD)&0xffff;

	}else					//��չ֡
	{
		CAN_FilterInitStructure.CAN_FilterIdHigh   = (((u32)canId<<3)&0xffff0000)>>16;
		CAN_FilterInitStructure.CAN_FilterIdLow    = (((u32)canId<<3)|CAN_ID_EXT)&0xffff;
	}
	
	mask <<= 3;//���� IDE RTR 0 λ
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh  = (mask >> 16)&0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow   = (mask&0xFFFF)|0x02;		//ֻ��������֡
	
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO1;	//������0������FIFO1
	CAN_FilterInitStructure.CAN_FilterActivation    =ENABLE;			//���������0
	CAN_FilterInit(&CAN_FilterInitStructure);							//�˲�����ʼ�� CAN2
}





















