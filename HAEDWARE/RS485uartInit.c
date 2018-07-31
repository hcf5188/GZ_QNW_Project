/************************************************************
*��˾����:             �Ϻ����������Ƽ�
*�ļ�����:      RS485_uart_init.c
*��������:      �鴺��  QQ:1341189542
*�ļ�����:      RS232  ���ڳ�ʼ�����շ�
*��д����:       2018-4-10
*************************************************************/

#include "includes.h"

extern pCIR_QUEUE send485_Q;	//ָ�� 485 ���ڷ��Ͷ���  ��ָ��
extern pSTORE     rece485_S;	//ָ�� 485 ���ڽ������ݶѵ�ָ��
extern OS_EVENT * rs485RcvQ;				//485 ������Ϣ���е�ָ��
void RS485SendByte(uint8_t dat)
{
#if OS_CRITICAL_METHOD == 3u           /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0u;
#endif
	if(CirQ_GetLength(send485_Q) > 0)
	{
		OS_ENTER_CRITICAL();
		CirQ_OnePush(send485_Q,dat);
		OS_EXIT_CRITICAL();
	}
	else
		USART_SendData(USART2, dat);
}
uint8_t RS485SendDatas(const uint8_t* s,uint16_t length)
{
#if OS_CRITICAL_METHOD == 3u           /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0u;
#endif
//	uint8_t data;
	if(length < 1 || length >500)
		return 1;
	OS_ENTER_CRITICAL();
	
	if(CirQ_Pushs(send485_Q,s,length) != OK)
	{
		OS_EXIT_CRITICAL();
		return 2;
	}
	OS_EXIT_CRITICAL();
	if(CirQ_GetLength(send485_Q) > 0)
	{
		USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	}
	return  0;
}

static NORMAL_STATE RS485State;
static u8 rx5Data = 0;
static u8 tx5Data = 0;
static u8 clear2;
u8* p485Rece;
static uint16_t frameLen;


void USART2_IRQHandler(void)
{
	OSIntEnter();//ϵͳ�����жϷ������
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//���յ�����
  	{
    	rx5Data = USART_ReceiveData(USART2);
		Store_Push(rece485_S,rx5Data);   
		USART_ClearITPendingBit(USART2, USART_IT_RXNE) ;
	}
	else if(USART_GetITStatus(USART2, USART_IT_TC) != RESET)//�������  ��λ�Ĵ�����
	{
		if(CirQ_GetLength(send485_Q) > 0)
		{
			RS485State = CirQ_Pop(send485_Q,&tx5Data);
			if(RS485State == OK)
				USART_SendData(USART2,tx5Data);
		}
		USART_ClearITPendingBit(USART2, USART_IT_TC);
	}
	else if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)//��  ���ͼĴ�����
	{
		if(CirQ_GetLength(send485_Q) > 0)
		{
			RS485State = CirQ_Pop(send485_Q,&tx5Data);
			if(RS485State == OK)
				USART_SendData(USART2,tx5Data);
		}
		USART_ITConfig(USART2,USART_IT_TXE,DISABLE);
		USART_ClearITPendingBit(USART2,USART_IT_TXE);
	}
	else if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)//������һ֡����
	{
		clear2 = USART2->SR;
		clear2 = USART2->DR;
		clear2 = clear2;
		frameLen = Store_Getlength(rece485_S);

		p485Rece = Mem_malloc(frameLen+2);
		if(p485Rece != NULL)	//�ڴ������ɹ�
		{
			memcpy(p485Rece,&frameLen,2);
			Store_Getdates(&p485Rece[2],rece485_S,frameLen);
			if( OSQPost(rs485RcvQ,p485Rece)!=OS_ERR_NONE)
				Mem_free((void **)(&p485Rece));
			Store_Clear(rece485_S);//�������ν��յ�����
		}
	}
	OSIntExit();  //�жϷ��������ϵͳ�����������
}










void RS485_UartConfig(u32 baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; // �жϳ�ʼ��

	/* ��GPIOCʱ�ӡ�AFIOʱ�ӣ�USART2ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2; //USART2 TX��
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;//�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); //�˿�C��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3; //USART2 RX��
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN_FLOATING;  //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); //�˿�D��
	
//	GPIO_PinRemapConfig(GPIO_Remap_USART2,ENABLE);
								
	/* USART ���� */
	USART_DeInit(USART2);
	USART_InitStructure.USART_BaudRate   = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;
	USART_InitStructure.USART_Parity     = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode       = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	/* ʹ��USART2�շ��ж� */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_TC, ENABLE);		   
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
	/* ʹ�� USART2 */
	USART_Cmd(USART2, ENABLE);
	USART_ClearFlag(USART2, USART_FLAG_TC);
	   
	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;		//�����ж�����
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ���
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //��Ӧ���ȼ���
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}






