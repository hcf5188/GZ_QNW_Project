#include "includes.h"



extern void RS485SendByte(uint8_t dat);
extern uint8_t RS485SendDatas(const uint8_t* s,uint16_t length);


extern void ChannelDCM(uint8_t channel,uint8_t resCh);	//DCM  ͨ������
extern void ChannelPMos(uint8_t channel,uint8_t resCh);	//PMos С���Ӳ���
extern void ChannelNMos(uint8_t channel,uint8_t resCh);	//NMos С���Ӳ���
extern void ResTest(uint8_t channel,uint8_t resCh);		//�������
extern void VolTest(uint8_t volCh);		//��ѹ����
 
void UpElecInit(void);	//�ϵ���һȦ�̵��������Ƿ��нӴ�����������

extern OS_EVENT * rs485RcvQ;		//485 ������Ϣ���е�ָ��
uint8_t DB26_TestFlag = 0;
void RS485Task(void *pdata)
{
	u8 err,i,mode,cha,resCh;
	u8 * p485Rcv;
	u16 frameLen = 0;
	uint8_t checkSum;
	OSTimeDlyHMSM(0,0,0,300);
	UpElecInit();//�ϵ����һ�¼̵����л�
	while(1)
	{
		p485Rcv = OSQPend(rs485RcvQ,0,&err);   //�ȴ�Ҫ���͵� 485 ����
		memcpy(&frameLen,p485Rcv,2);	//��ȡ����
		
		if(frameLen != 7)				//���Ȳ���
			goto errEnd;
		for(i=0;i<4;i++)
			checkSum += p485Rcv[i+2];
		if(checkSum != p485Rcv[7])		//У���ûͨ��
			goto errEnd;
		mode  = p485Rcv[4];		//ģʽѡ��
		cha   = p485Rcv[5];		//ͨ��ѡ��
		resCh = p485Rcv[6];		//ͨ����ƥ����衢��ѹѡ��
		switch(mode)
		{
			case 0:Reset_AllGPIO();			break;	//ȫ���ر�
			case 1:ChannelDCM(cha,resCh);	break;	//DCM   ֱ���������
			case 2:ChannelPMos(cha,resCh);	break;	//P ��С�����л�
			case 3:ChannelNMos(cha,resCh);	break;	//N ��С�����л�
			case 4:ResTest(cha,resCh);		break;	//�������
			case 5:VolTest(resCh);			break;	//��ѹ����
			case 6:DB26_TestFlag = 1;		break;	//todo:  DB26 ״̬����
			default:break;
		}
		RS485SendDatas(&p485Rcv[2],frameLen);	 //���յ������ݷ��ͻ�ȥ
	errEnd:		
		Mem_free((void**)(&p485Rcv));			 //�ͷ��ڴ��
	}
}

//�ϵ���һȦ
void UpElecInit(void)
{
	uint8_t i = 0;
	uint8_t testTime = 50;
	//DCM �л�����
	for(i = 0;i<4;i++){	ChannelDCM(i,0);OSTimeDlyHMSM(0,0,0,testTime);	}
	for(i = 0;i<4;i++){	ChannelDCM(0,i);OSTimeDlyHMSM(0,0,0,testTime);	}
	//P С�����л�����
	for(i = 0;i<3;i++){	ChannelPMos(i,0);OSTimeDlyHMSM(0,0,0,testTime);	}
	for(i = 0;i<4;i++){	ChannelPMos(0,i);OSTimeDlyHMSM(0,0,0,testTime);	}
	//N С�����л�����
	for(i = 0;i<7;i++){	ChannelNMos(i,0);OSTimeDlyHMSM(0,0,0,testTime);	}
	for(i = 0;i<4;i++){	ChannelNMos(0,i);OSTimeDlyHMSM(0,0,0,testTime);	}
	//��·������ѹ �л�����
	for(i = 0;i<5;i++){	VolTest(i);	OSTimeDlyHMSM(0,0,0,testTime);	}
	
	Reset_AllGPIO();//��λ����IO
}





