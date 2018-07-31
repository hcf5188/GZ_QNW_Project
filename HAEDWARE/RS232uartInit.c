/************************************************************
*��˾����:             �Ϻ����������Ƽ�
*�ļ�����:      RS232_uart_init.c
*��������:      �鴺��  QQ:1341189542
*�ļ�����:      RS232  ���ڳ�ʼ�����շ�
*��д����:       2018-4-10
*************************************************************/


#include "bsp.h"
#include "includes.h"

extern pCIR_QUEUE send232_Q;	//ָ�� 232 ���ڷ��ͻ�������ָ��
extern pSTORE     rece232_S;	//ָ�� 232 ���ڽ��ջ�������ָ��
extern OS_EVENT * RS232SendQ;		//232 ������Ϣ���е�ָ��
void RS232SendByte(uint8_t dat)
{
#if OS_CRITICAL_METHOD == 3u           /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0u;
#endif
	if(CirQ_GetLength(send232_Q) > 0)
	{
		OS_ENTER_CRITICAL();
		CirQ_OnePush(send232_Q,dat);
		OS_EXIT_CRITICAL();
	}
	else
		USART_SendData(USART1, dat);
}
uint8_t RS232SendDatas(const uint8_t* s,uint16_t length)
{
#if OS_CRITICAL_METHOD == 3u           /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0u;
#endif
//	uint8_t data;
	if(length < 1 || length >500)
		return 1;
	OS_ENTER_CRITICAL();
	
	if(CirQ_Pushs(send232_Q,s,length) != OK)
	{
		OS_EXIT_CRITICAL();
		return 2;
	}
	OS_EXIT_CRITICAL();
	if(CirQ_GetLength(send232_Q) > 0)
	{
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	}
	return  0;
}
void Test232RxDataDeal(u8 * ptr);//���ø�ͨѶ�ӿ�

static u8 rx1Data = 0;
static u8 tx1Data = 0;
extern OS_EVENT * testMbox;		//ͨѶ��� ��Ϣ����
static NORMAL_STATE uart1State;
static u8 clear1;
uint16_t rx1Len = 0;
uint8_t * ptrRx232;
uint8_t   testMode[2]={0,0};
static uint16_t i;
void USART1_IRQHandler(void)
{
	OSIntEnter();//ϵͳ�����жϷ������

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//���յ�����
  	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE) ;
    	rx1Data = USART_ReceiveData(USART1);
		Store_Push(rece232_S,rx1Data);   
	}
	else if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)//�������  ��λ�Ĵ�����
	{
		USART_ClearITPendingBit(USART1, USART_IT_TC);
		if(CirQ_GetLength(send232_Q) > 0)
		{
			uart1State = CirQ_Pop(send232_Q,&tx1Data);
			if(uart1State == OK)
				USART_SendData(USART1, tx1Data);
		}
	}
	else if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)//��  ���ͼĴ�����
	{
		if(CirQ_GetLength(send232_Q) > 0)
		{
			uart1State = CirQ_Pop(send232_Q,&tx1Data);
			if(uart1State == OK)
				USART_SendData(USART1, tx1Data);
		}
		USART_ITConfig(USART1,USART_IT_TXE,DISABLE);
		USART_ClearITPendingBit(USART1,USART_IT_TXE);
	}
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)//������һ֡����
	{
		clear1 = USART1->SR;			//���־λ
		clear1 = USART1->DR;
		clear1 = clear1;
		rx1Len = Store_Getlength(rece232_S);
		if(rx1Len == 2 && rece232_S->base[0] == 0xAA && rece232_S->base[1] == 0xBB)
		{
			varOperaRun.sysMode = TEST_PORT_RECV_MODE;
			RS232_UartConfig(9600);	
		}
		if(varOperaRun.sysMode == TEST_PORT_SEND_MODE)//���� ���� ģʽ
		{
			ptrRx232 = Mem_malloc(rx1Len);
			if(ptrRx232 != NULL)
			{
				Store_Getdates(ptrRx232,rece232_S,rx1Len);
				if(OSMboxPost(testMbox,ptrRx232) != OS_ERR_NONE)
					Mem_free((void **)&ptrRx232);
			}	
		}else if(varOperaRun.sysMode == TEST_PORT_RECV_MODE)//���� ���� ģʽ
		{
			if(rx1Len == 2)
			{
				Store_Getdates(testMode,rece232_S,rx1Len);
				Test232RxDataDeal(testMode);
			}
			else if(rx1Len > 2)
			{
				ptrRx232 = Mem_malloc(rx1Len+2);
				memcpy(ptrRx232,&rx1Len,2);
				Store_Getdates(&ptrRx232[2],rece232_S,rx1Len);
				
				for(i = 0;i < rx1Len;i++)
				{
					ptrRx232[i+2] ++;
				}
				if( OSQPost(RS232SendQ,ptrRx232)!=OS_ERR_NONE)
					Mem_free((void **)(&ptrRx232));
				
			}Store_Clear(rece232_S);
		}
		else 
		{
			Store_Clear(rece232_S);
		}
	}
	OSIntExit();  //�жϷ��������ϵͳ�����������
}
uint8_t a = 0;
//���ճ�ʱ�ж�
void Test232RxDataDeal(u8 * ptr)
{
	uint16_t dat595 = 0x0000;
	switch(ptr[0])
	{
		case 1:
			dat595 += (1<<11);	//��  can1A  Ĭ��CAN1A
			dat595 += (1<<8);	//120ŷķ����
			switch(ptr[1])
			{
				case 1:
					CAN1Config(0,CAN_ID_EXT,0,100000);break;
				case 2:
					CAN1Config(0,CAN_ID_EXT,0,250000);break;
				case 3:
					CAN1Config(0,CAN_ID_EXT,0,500000);break;
				case 4:
					CAN1Config(0,CAN_ID_EXT,0,1000000);break;
			}
			break;
		case 2:
			dat595 += (1<<11);	//��  can1B  ѡ��CAN1B
			dat595 += (1<<10);	
			dat595 += (1<<8);	//120ŷķ����//120ŷķ����
			switch(ptr[1])
			{
				case 1:
					CAN1Config(0,CAN_ID_EXT,0,100000);break;
				case 2:
					CAN1Config(0,CAN_ID_EXT,0,250000);break;
				case 3:
					CAN1Config(0,CAN_ID_EXT,0,500000);break;
				case 4:
					CAN1Config(0,CAN_ID_EXT,0,1000000);break;
			}
			break;
		case 3:
			dat595 += (1<<12);//��  can2  ѡ��CAN2A
			dat595 += (1<<13);        
			dat595 += (1<<15);//120ŷķ����
			switch(ptr[1])
			{
				case 1:
					CAN2Config(0,CAN_ID_EXT,0,100000);break;
				case 2:
					CAN2Config(0,CAN_ID_EXT,0,250000);break;
				case 3:
					CAN2Config(0,CAN_ID_EXT,0,500000);break;
				case 4:
					CAN2Config(0,CAN_ID_EXT,0,1000000);break;
			}
			break;
		case 4:
			dat595 += (1<<12);	//��  can2  Ĭ��CAN2B	
			dat595 += (1<<15);	//120ŷķ����
			switch(ptr[1])
			{
				case 1:
					CAN2Config(0,CAN_ID_EXT,0,100000);break;
				case 2:
					CAN2Config(0,CAN_ID_EXT,0,250000);break;
				case 3:
					CAN2Config(0,CAN_ID_EXT,0,500000);break;
				case 4:
					CAN2Config(0,CAN_ID_EXT,0,1000000);break;
			}
			break;
		
		case 5:	//K�߼��
			switch(ptr[1]-4)
			{
				case 1:
					K_Config(10400);break;
				case 2:
					K_Config(19200);break;
				case 3:
					K_Config(57600);break;
				case 4:
					K_Config(100000);break;
			}
			break;
		case 6:	//485���
			switch(ptr[1]-4)
			{
				case 1:
					RS485_UartConfig(9600);break;
				case 2:
					RS485_UartConfig(38400);break;
				case 3:
					RS485_UartConfig(57600);break;
				case 4:
					RS485_UartConfig(115200);break;
			}
			break;
		case 7:	//232���
			switch(ptr[1]-4)
			{
				case 1:
					RS232_UartConfig(9600);break;
				case 2:
					RS232_UartConfig(38400);break;
				case 3:
					RS232_UartConfig(57600);break;
				case 4:
					RS232_UartConfig(115200);break;
			}
			break;
	}
	dat595 += (varOperaRun.dat595 & 0x00FF);
	if(dat595 != varOperaRun.dat595)
	{
		varOperaRun.dat595 = dat595;
//		LS595Deal(varOperaRun.dat595);
	}
}

/********************************************************************
					���ڳ�ʼ������ʱ����ʼ������
********************************************************************/
void RS232_UartConfig(uint32_t baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE );

/* UART5 TX PB6 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/* UART5 RX PB7 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);

	USART_InitStructure.USART_BaudRate            = baud  ;	//ͨ�Ų�����
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	//�շ�ģʽ��
	USART_Init(USART1, &USART_InitStructure);//���ô��ڲ�����
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
	
	USART_Cmd(USART1, ENABLE); //ʹ�ܴ��ڣ�
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; //�жϺţ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ���
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //��Ӧ���ȼ���
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


}



