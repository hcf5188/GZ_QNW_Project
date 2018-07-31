
#include "includes.h"
#include "apptask.h"
#include  "usbd_hid_core.h"
#include  "usbd_usr.h"
#include  "usbd_desc.h"

/*************************   �����ջ   ******************************/

OS_STK START_TASK_STK[START_STK_SIZE];	//��ʼ�����ջ

OS_STK RS485_TASK_STK[RS485_STK_SIZE];	//485�շ������ջ


/*************************   �����ͨ��  *****************************/
OS_EVENT * rs485RcvQ;				//485 ������Ϣ���е�ָ��

#define RS485SENDBUF_SIZE   5		//485 ������Ϣ���б�����Ϣ�������

void *rs485SendBuf[RS485SENDBUF_SIZE];  //ָ�� 485  ������Ϣ��ָ������
/*************************   ȫ�ֱ���   ******************************/
pCIR_QUEUE send485_Q;	//ָ�� 485 ���ڷ��ͻ�������ָ��
pSTORE     rece485_S;	//ָ�� 485 ���ڽ��ջ�������ָ��

/*************************   ������   ******************************/
int main(void)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH,0x4800);      //�ж�������ӳ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);      //0 
	
	OSInit();		//ϵͳ��ʼ��
	MemBuf_Init();	//�ڴ���ʼ��
	SystemBspInit();		            //�����ʼ��
	send485_Q = Cir_Queue_Init(100);	//485  ����ѭ������ ������
	rece485_S = Store_Init(100);		//485  ���ڽ��� ���ݶ� ������
	OSTaskCreate(StartTask,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE - 1],START_TASK_PRIO);
	OSStart();				            //ϵͳ����  ��ʼ�������
}

void ChannelDCM(uint8_t channel,uint8_t resCh)
{
	// 3 · ֱ����� DCM
	GPIO_ResetBits(GPIOB,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5);//��λ
	switch(channel)
	{
		case 0:	 break;
		case 1: GPIO_SetBits(GPIOB,GPIO_Pin_3);break;
		case 2: GPIO_SetBits(GPIOB,GPIO_Pin_4);break;
		case 3: GPIO_SetBits(GPIOB,GPIO_Pin_5);break;
		default :
			break;
	}
	//ͨ��1-1  ֱ��DCM 3 ·�����л������ز���������
	GPIO_ResetBits(GPIOD,GPIO_Pin_2);
	GPIO_ResetBits(GPIOC,GPIO_Pin_11|GPIO_Pin_12);
	switch(resCh)
	{
		case 0:  break;
		case 1:  GPIO_SetBits(GPIOD,GPIO_Pin_2);break;	// DCM ����1
		case 2:  GPIO_SetBits(GPIOC,GPIO_Pin_12);break;	// DCM ����2
		case 3:  GPIO_SetBits(GPIOC,GPIO_Pin_11);break;	// DCM ����3
		default :break;
	}
}

void ChannelPMos(uint8_t channel,uint8_t resCh)
{
	//2 · P ͨ��
	GPIO_ResetBits(GPIOB,GPIO_Pin_1|GPIO_Pin_7);
	switch(channel)
	{
		case 0: break;
		case 1: GPIO_SetBits(GPIOB,GPIO_Pin_1);break;
		case 2: GPIO_SetBits(GPIOB,GPIO_Pin_7);break;
		default:break;
	}
	//3 · P ͨ�� �����л������ز���������
	GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_6|GPIO_Pin_15);
	switch(resCh)
	{
		case 0:  break;
		case 1:  GPIO_SetBits(GPIOB,GPIO_Pin_15);break;	//����  1
		case 2:  GPIO_SetBits(GPIOB,GPIO_Pin_6) ;break;	//����  2
		case 3:  GPIO_SetBits(GPIOB,GPIO_Pin_2) ;break;	//����  3
		default :break;
	}	
}
void ChannelNMos(uint8_t channel,uint8_t resCh)
{
	//6· N ͨ�� 
	GPIO_ResetBits(GPIOB,GPIO_Pin_14);
	GPIO_ResetBits(GPIOC,GPIO_Pin_10|GPIO_Pin_7);
	GPIO_ResetBits(GPIOA,GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_15);
	switch(channel)
	{
		case 0: break;
		case 1: GPIO_SetBits(GPIOC,GPIO_Pin_10);break;  // 1  ͨ��
		case 2: GPIO_SetBits(GPIOB,GPIO_Pin_14);break;  // 2  ͨ��
		case 3: GPIO_SetBits(GPIOC,GPIO_Pin_7); break;  // 3  ͨ��
		case 4: GPIO_SetBits(GPIOA,GPIO_Pin_11);break;  // 4  ͨ��
		case 5: GPIO_SetBits(GPIOA,GPIO_Pin_15);break;  // 5  ͨ��
		case 6: GPIO_SetBits(GPIOA,GPIO_Pin_12);break;  // 6  ͨ��
		default :break;
	}
	//6 · N ͨ�� ��Ӧ�� 3 ·�����л�
	GPIO_ResetBits(GPIOC,GPIO_Pin_9|GPIO_Pin_8|GPIO_Pin_6);
	switch(resCh)
	{
		case 0: break;
		case 1:  GPIO_SetBits(GPIOC,GPIO_Pin_9)  ;break; // ����  1
		case 2:  GPIO_SetBits(GPIOC,GPIO_Pin_8)  ;break; // ����  2
		case 3:  GPIO_SetBits(GPIOC,GPIO_Pin_6)  ;break; // ����  3
		default :break;
	}
}
//��·�������
void ResTest(uint8_t channel,uint8_t resCh)
{
	//��·�������
	GPIO_SetBits(GPIOB,GPIO_Pin_9);
	GPIO_SetBits(GPIOC,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
	switch(channel)
	{
		case 0: break;
		case 1: GPIO_ResetBits(GPIOB,GPIO_Pin_9);  GPIO_SetBits(GPIOC,GPIO_Pin_13);  GPIO_SetBits(GPIOC,GPIO_Pin_14);  GPIO_SetBits(GPIOC,GPIO_Pin_15);break;//  �������ͨ· 1
		case 2:   GPIO_SetBits(GPIOB,GPIO_Pin_9);GPIO_ResetBits(GPIOC,GPIO_Pin_13);  GPIO_SetBits(GPIOC,GPIO_Pin_14);  GPIO_SetBits(GPIOC,GPIO_Pin_15);break;//  �������ͨ· 2
		case 3:   GPIO_SetBits(GPIOB,GPIO_Pin_9);  GPIO_SetBits(GPIOC,GPIO_Pin_13);GPIO_ResetBits(GPIOC,GPIO_Pin_14);  GPIO_SetBits(GPIOC,GPIO_Pin_15);break;//  �������ͨ· 3
		case 4:   GPIO_SetBits(GPIOB,GPIO_Pin_9);  GPIO_SetBits(GPIOC,GPIO_Pin_13);  GPIO_SetBits(GPIOC,GPIO_Pin_14);GPIO_ResetBits(GPIOC,GPIO_Pin_15);break;//  �������ͨ· 4
		default :break;                            
	}	
	//��·�������
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
void VolTest(uint8_t volCh)	//��ѹ����
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
// ��ʼ����
void StartTask(void *pdata)
{
//	uint8_t err;
	uint8_t resCh = 0;
	uint8_t i,checkSum = 0;

/*************************    ���������ͨ�ŵ���Ϣ     ******************************/	
	
	rs485RcvQ	= OSQCreate(&rs485SendBuf[0],RS485SENDBUF_SIZE);	//���� 485  ���� ��Ϣ����

/*************************************     ����������    ************************************************/		
	OSTaskCreate(RS485Task,(void *)0,(OS_STK *)&RS485_TASK_STK[RS485_STK_SIZE-1],RS485_TASK_PRIO);
	
	while(1)							//���� LED �ƹ���ʾ����· CAN �л�����
	{
		OSTimeDlyHMSM(0,0,0,200);		//400ms ִ��һ��
		
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
			RS485SendDatas(reportDB,7);	 //��DB26����״̬�����ݷ��ͻ�ȥ
		}
	}
}












