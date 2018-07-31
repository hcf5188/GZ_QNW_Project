#include "sys.h"
#include "includes.h"

extern pCIR_QUEUE sendK_Q;		//指向 K线  串口发送缓冲区的指针
extern pSTORE     receK_S;		//指向 K线  串口接收缓冲区的指针
extern OS_EVENT * KLineRecvQ;   //K线  接收消息队列


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
	
	USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);//关闭串口接收中断
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
extern OS_EVENT * testMbox;		//通讯检测 消息邮箱
static uint16_t i ;
void UART4_IRQHandler(void)
{
	OSIntEnter();//系统进入中断服务程序

	if(USART_GetITStatus(UART4,USART_IT_RXNE) != RESET)//接收到数据
  	{
    	rx4Data = USART_ReceiveData(UART4);
		Store_Push(receK_S,rx4Data);         //将接收到的数据保存到 receBT_S
		USART_ClearITPendingBit(UART4,USART_IT_RXNE) ;
	}
	else if(USART_GetITStatus(UART4, USART_IT_TC) != RESET)//发送完毕  移位寄存器空
	{
		if(CirQ_GetLength(sendK_Q) > 0)
		{
			uart4State = CirQ_Pop(sendK_Q,&tx4Data);
			if(uart4State == OK)
				USART_SendData(UART4,tx4Data);
		}
		else{
			rx4Data = USART_ReceiveData(UART4);
			USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//打开串口接收中断
		}
		USART_ClearITPendingBit(UART4,USART_IT_TC);
	}
	else if(USART_GetITStatus(UART4,USART_IT_TXE) != RESET)//空  发送寄存器空
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
	else if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)//接收完一帧数据
	{
		clear4 = UART4->SR;
		clear4 = UART4->DR;//清除标志位
		clear4 = clear4;

		frameLen = Store_Getlength(receK_S);
		if(varOperaRun.sysMode == TEST_PORT_SEND_MODE)      //主测 模式
		{
			pKRece = Mem_malloc(frameLen);
			if(pKRece != NULL)	//内存块申请成功
			{
				Store_Getdates(pKRece,receK_S,frameLen);
				if(OSMboxPost(testMbox,pKRece) != OS_ERR_NONE)//推送不成功需要释放内存块
				{
					Mem_free((void **)(&pKRece));
					Store_Clear(receK_S);//舍弃本次接收的数据
				}
			}
		}else if(varOperaRun.sysMode == TEST_PORT_RECV_MODE)//被测 模式
		{
			pKRece = Mem_malloc(frameLen);
			if(pKRece != NULL)	//内存块申请成功
			{
				Store_Getdates(pKRece,receK_S,frameLen);
				for(i = 0;i < frameLen;i++)
				{
					pKRece[i] ++;
				}
				KLineSendDatas(pKRece,frameLen);
				Mem_free((void **)(&pKRece));
				Store_Clear(receK_S);//舍弃本次接收的数据
			}
		}
		else 
		{
			if(frameLen > 1)
			{
				frameLen++;//加上sid 一个字节
				pKRece = Mem_malloc(frameLen + 3);
				if(pKRece != NULL)	//内存块申请成功
				{
					memcpy(pKRece,&frameLen,2);
					pKRece[2] = varOperaRun.cmdIdK + 0x40;//发送的sid
					Store_Getdates(&pKRece[3],receK_S,(frameLen-1));
					if(OSQPost(KLineRecvQ,pKRece) != OS_ERR_NONE)//推送不成功需要释放内存块
					{
						Mem_free((void **)(&pKRece));
						Store_Clear(receK_S);//舍弃本次接收的数据
					}
				}
			}
			else if(frameLen == 1)//非正常数据，清空
			{
				Store_Clear(receK_S);
			}
		}
	}

	OSIntExit();  //中断服务结束，系统进行任务调度
}


void K_Config(u32 baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE );

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;			//UART4 RX；
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;	//浮空输入；
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);					//端口C；

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;				//UART4 TX；
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出；
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//端口D；

	USART_InitStructure.USART_BaudRate   = baud;			//波特率；
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //数据位8位；
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;	//停止位1位；
	USART_InitStructure.USART_Parity     = USART_Parity_No ;	//无校验位；
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	//无硬件流控；
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	//收发模式；
	USART_Init(UART4, &USART_InitStructure);//配置串口参数；
	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	USART_ITConfig(UART4, USART_IT_TC, ENABLE);    
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE); 

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;			//中断号；
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级；
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//响应优先级；
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(UART4, ENABLE);									//使能串口；
}




















