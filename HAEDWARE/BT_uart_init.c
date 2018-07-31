/************************************************************
*公司名称:             上海星融汽车科技
*文件名称:      BT_uart_init.c
*作者姓名:      洪春峰  QQ:1341189542
*文件内容:      蓝牙串口初始化、收发
*编写日期:       2018-4-10
*************************************************************/

#include "bsp.h"
#include "includes.h"


extern pCIR_QUEUE sendBT_Q;			//指向 蓝牙 串口发送队列  的指针
extern pSTORE     receBT_S;			//指向 蓝牙 串口接收数据堆的指针
extern OS_EVENT * MessageRecvQ;		//蓝牙 接收消息队列的指针
extern OS_EVENT * WIFICmdQ;			//接收到WiFi链接信息
extern OS_EVENT * WIFIDatQ;			//接收到设备发来的数据信息
void BTSendByte(uint8_t dat)
{
#if OS_CRITICAL_METHOD == 3u           /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0u;
#endif
	if(CirQ_GetLength(sendBT_Q) > 0)
	{
		OS_ENTER_CRITICAL();
		CirQ_OnePush(sendBT_Q,dat);
		OS_EXIT_CRITICAL();
	}
	else
		USART_SendData(USART3, dat);
}
uint8_t BTSendDatas(const uint8_t* s,uint16_t length)
{
#if OS_CRITICAL_METHOD == 3u           /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0u;
#endif
//	uint8_t data;
	if(length < 1 || length >500)
		return 1;
	OS_ENTER_CRITICAL();
	
	if(CirQ_Pushs(sendBT_Q,s,length) != OK)
	{
		OS_EXIT_CRITICAL();
		return 2;
	}
	OS_EXIT_CRITICAL();
	if(CirQ_GetLength(sendBT_Q) > 0)
	{
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	}
	return  0;
}

static u8 rx3Data    = 0;		//提取接收的数据
static u8 tx3Data    = 0;		//提取发送的数据
static u16 frameLen  = 0;
static NORMAL_STATE uart3State;
u8 *pBTRece;
static u8 clear3;
int cmpResult = 0;
uint8_t flag7E = 0;
void USART3_IRQHandler(void)
{
//	u8 wifiFlag = 0;
	frameLen = 0;
	OSIntEnter();//系统进入中断服务程序

	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//接收到数据
  	{
    	rx3Data = USART_ReceiveData(USART3);
		Store_Push(receBT_S,rx3Data);         //将接收到的数据保存到 receBT_S
		if(rx3Data == 0x7E)
		{
			flag7E ++;
			if(flag7E == 2)
			{
				flag7E = 0;
				frameLen = Store_Getlength(receBT_S);	//接收到的总长度
				if(frameLen > 1)
				{
					pBTRece = Mem_malloc(frameLen + 2);
					if(pBTRece != NULL)					//内存块申请成功
					{
						memcpy(pBTRece,&frameLen,2);	//得到长度
						Store_Getdates(&pBTRece[2],receBT_S,frameLen);
						if(OSQPost(MessageRecvQ,pBTRece) != OS_ERR_NONE)//推送不成功需要释放内存块
							Mem_free((void **)(&pBTRece));	
					}
					else 
					{
						Store_Clear(receBT_S);
					}
					Store_Clear(receBT_S);				//舍弃本次接收的数据
				}
				else if(frameLen == 1)
				{
					Store_Clear(receBT_S);
				}
			}
		}
		USART_ClearITPendingBit(USART3, USART_IT_RXNE) ;
	}
	if(USART_GetITStatus(USART3, USART_IT_TC) != RESET)//发送完毕  移位寄存器空
	{
		if(CirQ_GetLength(sendBT_Q) > 0)
		{
			uart3State = CirQ_Pop(sendBT_Q,&tx3Data);
			if(uart3State == OK)
				USART_SendData(USART3,tx3Data);
		}
		else
		{
			//todo:通知任务发送完成，可以发送下一包数据了
		}
		USART_ClearITPendingBit(USART3, USART_IT_TC) ;
	}
	if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)//空  发送寄存器空
	{
		if(CirQ_GetLength(sendBT_Q) > 0)
		{
			uart3State = CirQ_Pop(sendBT_Q,&tx3Data);
			if(uart3State == OK)
				USART_SendData(USART3,tx3Data);
		}
		USART_ITConfig(USART3, USART_IT_TXE,DISABLE);
		USART_ClearITPendingBit(USART3, USART_IT_TXE) ;
	}
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)//接收完一帧数据
	{
		clear3 = USART3->SR;			//清标志位
		clear3 = USART3->DR;
		clear3 = clear3;
//		flag7E = 0;
//		if(varOperaRun.wirelessMode == BT_MODE)	//如果是蓝牙模式，
//		{
//			frameLen = Store_Getlength(receBT_S);	//接收到的总长度
//			if(frameLen > 1)
//			{
//				pBTRece = Mem_malloc(frameLen + 2);
//				if(pBTRece != NULL)					//内存块申请成功
//				{
//					memcpy(pBTRece,&frameLen,2);	//得到长度
//					Store_Getdates(&pBTRece[2],receBT_S,frameLen);
//					if(OSQPost(MessageRecvQ,pBTRece) != OS_ERR_NONE)//推送不成功需要释放内存块
//						Mem_free((void **)(&pBTRece));	
//				}
//				Store_Clear(receBT_S);				//舍弃本次接收的数据
//			}
//			else if(frameLen == 1)
//			{
//				Store_Clear(receBT_S);
//			}
//		}else if(varOperaRun.wirelessMode == WIFI_MODE)//如果是WIFI模式
//		{
//			cmpResult = memcmp(receBT_S->base,"+IPD,",5);//接收到dat返回的信息
//			if(cmpResult == 0)
//			{	for(i = 0;i < 20;i++)
//				{
//					if(receBT_S->base[i] == ':')
//						break;
//					if(receBT_S->base[i] == ',')
//						wifiFlag ++;
//					if((wifiFlag == 2)&&(receBT_S->base[i] >= '0')&&(receBT_S->base[i] <= '9'))
//					{
//						frameLen = (frameLen*10) +(receBT_S->base[i] - '0');//得到平板发送的数据长度
//					}	
//				}
//				pBTRece = Mem_malloc(frameLen + 2);
//				if(pBTRece != NULL)								//内存块申请成功
//				{
//					i++;
//					memcpy(pBTRece,&frameLen,2);
//					memcpy(&pBTRece[2],&receBT_S->base[i],frameLen);
//					if(OSQPost(MessageRecvQ,pBTRece) != OS_ERR_NONE)	//推送不成功需要释放内存块
//						Mem_free((void **)(&pBTRece));
//					Store_Clear(receBT_S);						//舍弃本次接收的数据
//				}
//			}
//			else
//			{
//				if( (memcmp(receBT_S->base,"AT",2) == 0)||\
//					(receBT_S->base[0] == '>')||(memcmp(receBT_S->base,"SEND",4)==0))
//				{
//					frameLen = Store_Getlength(receBT_S);
//					pBTRece = Mem_malloc(frameLen + 2);
//					if(pBTRece != NULL)								//内存块申请成功
//					{
//						Store_Getdates(pBTRece,receBT_S,frameLen);
//						if(OSQPost(WIFICmdQ,pBTRece) != OS_ERR_NONE)//推送不成功需要释放内存块
//							Mem_free((void **)(&pBTRece));			
//					}
//				}
//				Store_Clear(receBT_S);
//			}
//		}
	}
	OSIntExit();  //中断服务结束，系统进行任务调度
}


/********************************************************************
					串口初始化、定时器初始化函数
********************************************************************/
void BT_UartConfig(uint32_t baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	//USART3 RX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	//USART3 TX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_PinRemapConfig(GPIO_FullRemap_USART3,ENABLE);
	
	USART_InitStructure.USART_BaudRate            = baud  ;	//通信波特率
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	
	USART_Init(USART3, &USART_InitStructure);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART3, USART_IT_TC, ENABLE);    
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE); 
	
	varOperaRun.wirelessMode = BT_MODE;						//无线通讯模式为蓝牙模式
	
	USART_Cmd(USART3, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}




