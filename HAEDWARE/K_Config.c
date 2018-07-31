#include "sys.h"
#include "includes.h"

extern pCIR_QUEUE sendK_Q;		//ָ�� K��  ���ڷ��ͻ�������ָ��
extern pSTORE     receK_S;		//ָ�� K��  ���ڽ��ջ�������ָ��
extern OS_EVENT * KLineRecvQ;   //K��  ������Ϣ����


void KLineSendByte(uint8_t dat)
{
#if OS_CRITICAL_METHOD == 3u           /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0u;
#endif
	if(CirQ_GetLength(sendK_Q) > 0)
	{
		OS_ENTER_CRITICAL();
		CirQ_OnePush(sendK_Q,dat);
		OS_EXIT_CRITICAL();
	}
	else
		USART_SendData(UART4, dat);
}
uint8_t KLineSendDatas(const uint8_t* s,uint16_t length)
{
#if OS_CRITICAL_METHOD == 3u           /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0u;
#endif
//	uint8_t data;
	if(length < 1 || length >500)
		return 1;
	OS_ENTER_CRITICAL();
	
	if(CirQ_Pushs(sendK_Q,s,length) != OK)
	{
		OS_EXIT_CRITICAL();
		return 2;
	}
	OS_EXIT_CRITICAL();
	
	USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);//�رմ��ڽ����ж�
	if(CirQ_GetLength(sendK_Q) > 0)
	{
		USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
	}
	return  0;
}

static u8 rx4Data = 0;
static u8 tx4Data = 0;
static u8 clear4;
static u16 frameLen  = 0;
static NORMAL_STATE uart4State;
u8 * pKRece;
extern OS_EVENT * testMbox;		//ͨѶ��� ��Ϣ����
static uint16_t i ;
void UART4_IRQHandler(void)
{
	OSIntEnter();//ϵͳ�����жϷ������

	if(USART_GetITStatus(UART4,USART_IT_RXNE) != RESET)//���յ�����
  	{
    	rx4Data = USART_ReceiveData(UART4);
		Store_Push(receK_S,rx4Data);         //�����յ������ݱ��浽 receBT_S
		USART_ClearITPendingBit(UART4,USART_IT_RXNE) ;
	}
	else if(USART_GetITStatus(UART4, USART_IT_TC) != RESET)//�������  ��λ�Ĵ�����
	{
		if(CirQ_GetLength(sendK_Q) > 0)
		{
			uart4State = CirQ_Pop(sendK_Q,&tx4Data);
			if(uart4State == OK)
				USART_SendData(UART4,tx4Data);
		}
		else{
			rx4Data = USART_ReceiveData(UART4);
			USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//�򿪴��ڽ����ж�
		}
		USART_ClearITPendingBit(UART4,USART_IT_TC);
	}
	else if(USART_GetITStatus(UART4,USART_IT_TXE) != RESET)//��  ���ͼĴ�����
	{
		if(CirQ_GetLength(sendK_Q) > 0)
		{
			uart4State = CirQ_Pop(sendK_Q,&tx4Data);
			if(uart4State == OK)
				USART_SendData(UART4, tx4Data);
		}
		USART_ITConfig(UART4,USART_IT_TXE,DISABLE);
		USART_ClearITPendingBit(UART4,USART_IT_TXE);
	}
	else if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)//������һ֡����
	{
		clear4 = UART4->SR;
		clear4 = UART4->DR;//�����־λ
		clear4 = clear4;

		frameLen = Store_Getlength(receK_S);
		if(varOperaRun.sysMode == TEST_PORT_SEND_MODE)      //���� ģʽ
		{
			pKRece = Mem_malloc(frameLen);
			if(pKRece != NULL)	//�ڴ������ɹ�
			{
				Store_Getdates(pKRece,receK_S,frameLen);
				if(OSMboxPost(testMbox,pKRece) != OS_ERR_NONE)//���Ͳ��ɹ���Ҫ�ͷ��ڴ��
				{
					Mem_free((void **)(&pKRece));
					Store_Clear(receK_S);//�������ν��յ�����
				}
			}
		}else if(varOperaRun.sysMode == TEST_PORT_RECV_MODE)//���� ģʽ
		{
			pKRece = Mem_malloc(frameLen);
			if(pKRece != NULL)	//�ڴ������ɹ�
			{
				Store_Getdates(pKRece,receK_S,frameLen);
				for(i = 0;i < frameLen;i++)
				{
					pKRece[i] ++;
				}
				KLineSendDatas(pKRece,frameLen);
				Mem_free((void **)(&pKRece));
				Store_Clear(receK_S);//�������ν��յ�����
			}
		}
		else 
		{
			if(frameLen > 1)
			{
				frameLen++;//����sid һ���ֽ�
				pKRece = Mem_malloc(frameLen + 3);
				if(pKRece != NULL)	//�ڴ������ɹ�
				{
					memcpy(pKRece,&frameLen,2);
					pKRece[2] = varOperaRun.cmdIdK + 0x40;//���͵�sid
					Store_Getdates(&pKRece[3],receK_S,(frameLen-1));
					if(OSQPost(KLineRecvQ,pKRece) != OS_ERR_NONE)//���Ͳ��ɹ���Ҫ�ͷ��ڴ��
					{
						Mem_free((void **)(&pKRece));
						Store_Clear(receK_S);//�������ν��յ�����
					}
				}
			}
			else if(frameLen == 1)//���������ݣ����
			{
				Store_Clear(receK_S);
			}
		}
	}

	OSIntExit();  //�жϷ��������ϵͳ�����������
}


void K_Config(u32 baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE );

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;			//UART4 RX��
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;	//�������룻
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);					//�˿�C��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;				//UART4 TX��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//�������������
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//�˿�D��

	USART_InitStructure.USART_BaudRate   = baud;			//�����ʣ�
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //����λ8λ��
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;	//ֹͣλ1λ��
	USART_InitStructure.USART_Parity     = USART_Parity_No ;	//��У��λ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	//��Ӳ�����أ�
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	//�շ�ģʽ��
	USART_Init(UART4, &USART_InitStructure);//���ô��ڲ�����
	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	USART_ITConfig(UART4, USART_IT_TC, ENABLE);    
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE); 

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;			//�жϺţ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//��ռ���ȼ���
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//��Ӧ���ȼ���
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(UART4, ENABLE);									//ʹ�ܴ��ڣ�
}




















