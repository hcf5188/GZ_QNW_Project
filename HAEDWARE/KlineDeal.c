#include "sys.h"
#include "bsp.h"
#include "includes.h"

//K 线快速初始化
extern OS_EVENT * KFastStartSem;			//PWM 信号量
void KLineFastInit(uint8_t *ptrData,uint16_t datLen)
{
	uint8_t err;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE );
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;        //UART4 TX；
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP; //复用推挽输出；
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);            
	
	GPIO_SetBits(GPIOC,GPIO_Pin_10);    //Tx Idle 拉高300ms以上
	OSTimeDlyHMSM(0,0,0,500);
	
	USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);//关闭串口接收中断
	
	varVciRun.kFastStart = OPEN;        //开始快速初始化
	OSSemPend(KFastStartSem,0,&err);	//等待50ms结束
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//发送快速连接指令
	KLineSendDatas(ptrData,datLen);
}
//K线 5波特率初始化
void KLine5BitInit(void)
{
	
}
//解包、封包ECU的校验和
uint8_t KCheckSum(uint8_t* ptr ,uint8_t cmd)
{
	uint8_t checkSum = 0;
	uint8_t i = 0;
	for(i = 0;i < (ptr[0] - 0x7D);i ++)
		checkSum += ptr[i]; 
	if(cmd == 0)
	{
		ptr[i] = checkSum;
		return 0;
	}
	else if(ptr[i] == checkSum)	
		return 0;
	return 1;
}
void K_5Baud_putc(uint8_t data)
{
	uint8_t i;

	//Start Bit
	GPIO_ResetBits(GPIOB,GPIO_Pin_10);
	OSTimeDlyHMSM(0,0,0,200);
	
	for(i = 0;i < 8;i++)
	{
		if(data & 0x01)
		{
			GPIO_SetBits(GPIOB,GPIO_Pin_10);
		}else
		{
			GPIO_ResetBits(GPIOB,GPIO_Pin_10);
		}

		OSTimeDlyHMSM(0,0,0,200);
		data = data >> 1;
	}
	//5 Baud 
	/*
	for(i=0;i<8;i++)
	{
		if((data>>i)&0x01)
		{
			GPIO_SetBits(GPIOB,GPIO_Pin_10);
		}else
		{
			GPIO_ResetBits(GPIOB,GPIO_Pin_10);
		}
		OSTimeDlyHMSM(0,0,0,200);
	}*/
	//Stop Bit
	GPIO_SetBits(GPIOB,GPIO_Pin_10);
	OSTimeDlyHMSM(0,0,0,200);
}
void K_Line_5BaudInit(void)
{	
	//USART_DATA_STRUCT *p_usart_data=&(g_usart_data[USART_OBD_INDEX]);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	//TX IO Output
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//T Idle 拉高300ms
	GPIO_SetBits(GPIOB,GPIO_Pin_10);
	OSTimeDlyHMSM(0,0,0,500);

	K_5Baud_putc(0x33);//0x33
	//p_usart_data->rx_last=p_usart_data->rx_used;
	
	//USART3  TX 转为收发器
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

}




