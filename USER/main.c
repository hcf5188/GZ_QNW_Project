
#include "includes.h"
#include "apptask.h"
#include  "usbd_hid_core.h"
#include  "usbd_usr.h"
#include  "usbd_desc.h"

/*************************   任务堆栈   ******************************/

OS_STK START_TASK_STK[START_STK_SIZE];	//起始任务堆栈

OS_STK RS485_TASK_STK[RS485_STK_SIZE];	//485收发任务堆栈


/*************************   任务间通信  *****************************/
OS_EVENT * rs485RcvQ;				//485 发送消息队列的指针

#define RS485SENDBUF_SIZE   5		//485 发送消息队列保存消息的最大量

void *rs485SendBuf[RS485SENDBUF_SIZE];  //指向 485  接收消息的指针数组
/*************************   全局变量   ******************************/
pCIR_QUEUE send485_Q;	//指向 485 串口发送缓冲区的指针
pSTORE     rece485_S;	//指向 485 串口接收缓冲区的指针

/*************************   主函数   ******************************/
int main(void)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH,0x4800);      //中断向量重映射
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);      //0 
	
	OSInit();		//系统初始化
	MemBuf_Init();	//内存块初始化
	SystemBspInit();		            //外设初始化
	send485_Q = Cir_Queue_Init(100);	//485  发送循环队列 缓冲区
	rece485_S = Store_Init(100);		//485  串口接收 数据堆 缓冲区
	OSTaskCreate(StartTask,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE - 1],START_TASK_PRIO);
	OSStart();				            //系统启动  开始任务调度
}

void ChannelDCM(uint8_t channel,uint8_t resCh)
{
	// 3 路 直流电机 DCM
	GPIO_ResetBits(GPIOB,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5);//复位
	switch(channel)
	{
		case 0:	 break;
		case 1: GPIO_SetBits(GPIOB,GPIO_Pin_3);break;
		case 2: GPIO_SetBits(GPIOB,GPIO_Pin_4);break;
		case 3: GPIO_SetBits(GPIOB,GPIO_Pin_5);break;
		default :
			break;
	}
	//通道1-1  直流DCM 3 路负载切换（负载并联方案）
	GPIO_ResetBits(GPIOD,GPIO_Pin_2);
	GPIO_ResetBits(GPIOC,GPIO_Pin_11|GPIO_Pin_12);
	switch(resCh)
	{
		case 0:  break;
		case 1:  GPIO_SetBits(GPIOD,GPIO_Pin_2);break;	// DCM 负载1
		case 2:  GPIO_SetBits(GPIOC,GPIO_Pin_12);break;	// DCM 负载2
		case 3:  GPIO_SetBits(GPIOC,GPIO_Pin_11);break;	// DCM 负载3
		default :break;
	}
}

void ChannelPMos(uint8_t channel,uint8_t resCh)
{
	//2 路 P 通道
	GPIO_ResetBits(GPIOB,GPIO_Pin_1|GPIO_Pin_7);
	switch(channel)
	{
		case 0: break;
		case 1: GPIO_SetBits(GPIOB,GPIO_Pin_1);break;
		case 2: GPIO_SetBits(GPIOB,GPIO_Pin_7);break;
		default:break;
	}
	//3 路 P 通道 负载切换（负载并联方案）
	GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_6|GPIO_Pin_15);
	switch(resCh)
	{
		case 0:  break;
		case 1:  GPIO_SetBits(GPIOB,GPIO_Pin_15);break;	//负载  1
		case 2:  GPIO_SetBits(GPIOB,GPIO_Pin_6) ;break;	//负载  2
		case 3:  GPIO_SetBits(GPIOB,GPIO_Pin_2) ;break;	//负载  3
		default :break;
	}	
}
void ChannelNMos(uint8_t channel,uint8_t resCh)
{
	//6路 N 通道 
	GPIO_ResetBits(GPIOB,GPIO_Pin_14);
	GPIO_ResetBits(GPIOC,GPIO_Pin_10|GPIO_Pin_7);
	GPIO_ResetBits(GPIOA,GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_15);
	switch(channel)
	{
		case 0: break;
		case 1: GPIO_SetBits(GPIOC,GPIO_Pin_10);break;  // 1  通道
		case 2: GPIO_SetBits(GPIOB,GPIO_Pin_14);break;  // 2  通道
		case 3: GPIO_SetBits(GPIOC,GPIO_Pin_7); break;  // 3  通道
		case 4: GPIO_SetBits(GPIOA,GPIO_Pin_11);break;  // 4  通道
		case 5: GPIO_SetBits(GPIOA,GPIO_Pin_15);break;  // 5  通道
		case 6: GPIO_SetBits(GPIOA,GPIO_Pin_12);break;  // 6  通道
		default :break;
	}
	//6 路 N 通道 对应的 3 路负载切换
	GPIO_ResetBits(GPIOC,GPIO_Pin_9|GPIO_Pin_8|GPIO_Pin_6);
	switch(resCh)
	{
		case 0: break;
		case 1:  GPIO_SetBits(GPIOC,GPIO_Pin_9)  ;break; // 负载  1
		case 2:  GPIO_SetBits(GPIOC,GPIO_Pin_8)  ;break; // 负载  2
		case 3:  GPIO_SetBits(GPIOC,GPIO_Pin_6)  ;break; // 负载  3
		default :break;
	}
}
//四路电阻测试
void ResTest(uint8_t channel,uint8_t resCh)
{
	//四路电阻测试
	GPIO_SetBits(GPIOB,GPIO_Pin_9);
	GPIO_SetBits(GPIOC,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
	switch(channel)
	{
		case 0: break;
		case 1: GPIO_ResetBits(GPIOB,GPIO_Pin_9);  GPIO_SetBits(GPIOC,GPIO_Pin_13);  GPIO_SetBits(GPIOC,GPIO_Pin_14);  GPIO_SetBits(GPIOC,GPIO_Pin_15);break;//  电阻测量通路 1
		case 2:   GPIO_SetBits(GPIOB,GPIO_Pin_9);GPIO_ResetBits(GPIOC,GPIO_Pin_13);  GPIO_SetBits(GPIOC,GPIO_Pin_14);  GPIO_SetBits(GPIOC,GPIO_Pin_15);break;//  电阻测量通路 2
		case 3:   GPIO_SetBits(GPIOB,GPIO_Pin_9);  GPIO_SetBits(GPIOC,GPIO_Pin_13);GPIO_ResetBits(GPIOC,GPIO_Pin_14);  GPIO_SetBits(GPIOC,GPIO_Pin_15);break;//  电阻测量通路 3
		case 4:   GPIO_SetBits(GPIOB,GPIO_Pin_9);  GPIO_SetBits(GPIOC,GPIO_Pin_13);  GPIO_SetBits(GPIOC,GPIO_Pin_14);GPIO_ResetBits(GPIOC,GPIO_Pin_15);break;//  电阻测量通路 4
		default :break;                            
	}	
	//六路电阻测量
	switch(resCh)
	{
		case 0: break;
		case 1:  GPIO_ResetBits(GPIOC,GPIO_Pin_1);GPIO_ResetBits(GPIOC,GPIO_Pin_2);GPIO_ResetBits(GPIOC,GPIO_Pin_3);break;    // Y0
		case 2:    GPIO_SetBits(GPIOC,GPIO_Pin_1);GPIO_ResetBits(GPIOC,GPIO_Pin_2);GPIO_ResetBits(GPIOC,GPIO_Pin_3);break;    // Y1
		case 3:  GPIO_ResetBits(GPIOC,GPIO_Pin_1);  GPIO_SetBits(GPIOC,GPIO_Pin_2);GPIO_ResetBits(GPIOC,GPIO_Pin_3);break;    // Y2
		case 4:    GPIO_SetBits(GPIOC,GPIO_Pin_1);  GPIO_SetBits(GPIOC,GPIO_Pin_2);GPIO_ResetBits(GPIOC,GPIO_Pin_3);break;    // Y3
		case 5:  GPIO_ResetBits(GPIOC,GPIO_Pin_1);GPIO_ResetBits(GPIOC,GPIO_Pin_2);  GPIO_SetBits(GPIOC,GPIO_Pin_3);break;    // Y4
		case 6:  GPIO_ResetBits(GPIOC,GPIO_Pin_1);  GPIO_SetBits(GPIOC,GPIO_Pin_2);  GPIO_SetBits(GPIOC,GPIO_Pin_3);break;    // Y6
		default :break;
	}
}
void VolTest(uint8_t volCh)	//电压测量
{

	GPIO_ResetBits(GPIOA,GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7);
	GPIO_ResetBits(GPIOC,GPIO_Pin_4);

	switch(volCh)
	{
		case 0: break;
		case 1: GPIO_SetBits(GPIOA,GPIO_Pin_4); break; //  5V
		case 2: GPIO_SetBits(GPIOA,GPIO_Pin_5); break; // 12V
		case 3: GPIO_SetBits(GPIOA,GPIO_Pin_7); break; // 24V
		case 4: GPIO_SetBits(GPIOC,GPIO_Pin_4); break; // 32V
		default :break;
	}
}
extern uint8_t DB26_TestFlag;
uint8_t pinDBState;
uint8_t reportDB[] = {0x7E,0x05,0x06,0x01,0x00,0x00,0x7E};
extern uint8_t RS485SendDatas(const uint8_t* s,uint16_t length);
// 起始任务
void StartTask(void *pdata)
{
//	uint8_t err;
	uint8_t resCh = 0;
	uint8_t i,checkSum = 0;

/*************************    创建任务间通信的消息     ******************************/	
	
	rs485RcvQ	= OSQCreate(&rs485SendBuf[0],RS485SENDBUF_SIZE);	//建立 485  发送 消息队列

/*************************************     创建各任务    ************************************************/		
	OSTaskCreate(RS485Task,(void *)0,(OS_STK *)&RS485_TASK_STK[RS485_STK_SIZE-1],RS485_TASK_PRIO);
	
	while(1)							//进行 LED 灯光显示、四路 CAN 切换、等
	{
		OSTimeDlyHMSM(0,0,0,200);		//400ms 执行一次
		
		if(DB26_TestFlag == 1)
		{
			checkSum = 0;
			pinDBState = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0);
			reportDB[4] = pinDBState;
			for(i = 0;i<4;i++)
			{
				checkSum += reportDB[i+1];
			}
			reportDB[5] = checkSum;
			RS485SendDatas(reportDB,7);	 //将DB26引脚状态的数据发送回去
		}
	}
}












