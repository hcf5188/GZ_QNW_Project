#include "includes.h"



extern void RS485SendByte(uint8_t dat);
extern uint8_t RS485SendDatas(const uint8_t* s,uint16_t length);


extern void ChannelDCM(uint8_t channel,uint8_t resCh);	//DCM  通道测量
extern void ChannelPMos(uint8_t channel,uint8_t resCh);	//PMos 小板子测量
extern void ChannelNMos(uint8_t channel,uint8_t resCh);	//NMos 小板子测量
extern void ResTest(uint8_t channel,uint8_t resCh);		//电阻测量
extern void VolTest(uint8_t volCh);		//电压测量
 
void UpElecInit(void);	//上电跑一圈继电器，看是否有接触不良的问题

extern OS_EVENT * rs485RcvQ;		//485 发送消息队列的指针
uint8_t DB26_TestFlag = 0;
void RS485Task(void *pdata)
{
	u8 err,i,mode,cha,resCh;
	u8 * p485Rcv;
	u16 frameLen = 0;
	uint8_t checkSum;
	OSTimeDlyHMSM(0,0,0,300);
	UpElecInit();//上电测试一下继电器切换
	while(1)
	{
		p485Rcv = OSQPend(rs485RcvQ,0,&err);   //等待要发送的 485 数据
		memcpy(&frameLen,p485Rcv,2);	//获取长度
		
		if(frameLen != 7)				//长度不对
			goto errEnd;
		for(i=0;i<4;i++)
			checkSum += p485Rcv[i+2];
		if(checkSum != p485Rcv[7])		//校验和没通过
			goto errEnd;
		mode  = p485Rcv[4];		//模式选择
		cha   = p485Rcv[5];		//通道选择
		resCh = p485Rcv[6];		//通道的匹配电阻、电压选择
		switch(mode)
		{
			case 0:Reset_AllGPIO();			break;	//全部关闭
			case 1:ChannelDCM(cha,resCh);	break;	//DCM   直流电机控制
			case 2:ChannelPMos(cha,resCh);	break;	//P 型小板子切换
			case 3:ChannelNMos(cha,resCh);	break;	//N 型小板子切换
			case 4:ResTest(cha,resCh);		break;	//电阻测量
			case 5:VolTest(resCh);			break;	//电压测量
			case 6:DB26_TestFlag = 1;		break;	//todo:  DB26 状态反馈
			default:break;
		}
		RS485SendDatas(&p485Rcv[2],frameLen);	 //将收到的数据发送回去
	errEnd:		
		Mem_free((void**)(&p485Rcv));			 //释放内存块
	}
}

//上电跑一圈
void UpElecInit(void)
{
	uint8_t i = 0;
	uint8_t testTime = 50;
	//DCM 切换测试
	for(i = 0;i<4;i++){	ChannelDCM(i,0);OSTimeDlyHMSM(0,0,0,testTime);	}
	for(i = 0;i<4;i++){	ChannelDCM(0,i);OSTimeDlyHMSM(0,0,0,testTime);	}
	//P 小板子切换测试
	for(i = 0;i<3;i++){	ChannelPMos(i,0);OSTimeDlyHMSM(0,0,0,testTime);	}
	for(i = 0;i<4;i++){	ChannelPMos(0,i);OSTimeDlyHMSM(0,0,0,testTime);	}
	//N 小板子切换测试
	for(i = 0;i<7;i++){	ChannelNMos(i,0);OSTimeDlyHMSM(0,0,0,testTime);	}
	for(i = 0;i<4;i++){	ChannelNMos(0,i);OSTimeDlyHMSM(0,0,0,testTime);	}
	//四路测量电压 切换测试
	for(i = 0;i<5;i++){	VolTest(i);	OSTimeDlyHMSM(0,0,0,testTime);	}
	
	Reset_AllGPIO();//复位所有IO
}





