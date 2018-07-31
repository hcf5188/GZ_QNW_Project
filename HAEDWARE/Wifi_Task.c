#include "includes.h"


const u8 atCmd[] = "AT\r";				//AT测试指令
const u8 atRST[] = "AT+RST\r";			//复位指令
const u8 atCIPMUX[] = "AT+CIPMUX=1\r";	//设置多连接
const u8 atCIPSERVER[] = "AT+CIPSERVER=1,8080\r";//设置服务器端口号
const u8 atCWMODE[]	= "AT+CWMODE=2\r";	//工作模式设置为AP  1-STA   2-AP   3-AP+STA
//设置WIFI名称、密码、加密方式
const u8 atNAMEPAS[] = "AT+CWSAP=\"ESP8266\",\"0123456789\",11,4\r";
const u8 atCIPSEND[] = "AT+CIPSEND=0,%d\r";	// AT 发送数据


extern OS_EVENT * WIFICmdQ;				//接收到 WiFi 链接信息
extern OS_EVENT * WIFIDatQ;				//接收到设备发来的数据信息
void WIFI_SendCmd(const u8 *ptrCmd,char * comString,uint16_t sendLength);

//WIFI模块初始化
void WIFI_Init(void)
{
	WIFI_SendCmd(atCIPMUX,(char *)atCIPMUX,sizeof(atCIPMUX));
	
	WIFI_SendCmd(atCIPSERVER,(char *)atCIPSERVER,sizeof(atCIPSERVER));
	
	WIFI_SendCmd(atNAMEPAS,(char *)atNAMEPAS,sizeof(atNAMEPAS));
}
u8 WIFI_ReceDeal(uint8_t *ptrRece,char* ptrComp)
{
	char* p1 = NULL;
	p1 = strstr((const char*)ptrRece,ptrComp);
	if(p1 == NULL)
		return 1;
	return 0;
}
extern pCIR_QUEUE sendBT_Q;		//指向 蓝牙 串口发送队列  的指针
extern OS_EVENT * MessageRecvQ;	//从平板接收消息队列的指针

u8 WIFISendDatas(u8* s,u16 length)
{
#if OS_CRITICAL_METHOD == 3u	/* Allocate storage for CPU status register	*/
	OS_CPU_SR  cpu_sr = 0u;
#endif
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
		USART_ITConfig(USART3,USART_IT_TXE,ENABLE);
	}
	return  0;
}

void WIFI_SendCmd(const u8 *ptrCmd,char * comString,uint16_t sendLength)
{
	uint8_t err;			//错误信息
	uint8_t count = 0;		//计算值
	uint8_t cmpData;
	uint8_t *ptrWIFICfg;	//接收到 WIFI 回复的信息
	do{
		count ++;
		WIFISendDatas((u8*)ptrCmd,sendLength);
		ptrWIFICfg = OSQPend(WIFICmdQ,200,&err);
		if(err != OS_ERR_NONE)        //接-收超时
		{
			WIFISendDatas((u8*)ptrCmd,sendLength);
			ptrWIFICfg = OSQPend(WIFICmdQ,500,&err);
			cmpData = WIFI_ReceDeal(ptrWIFICfg,comString);
			if(cmpData == 0)
			{
			
			}
			Mem_free((void **)&ptrWIFICfg);
		}
		else
		{
			err = WIFI_ReceDeal(ptrWIFICfg,comString);
			Mem_free((void **)&ptrWIFICfg);
		}
	}while(err!=0&&count<5);
}

void WIFIDataToSend(uint8_t * ptrToSend,uint16_t datLen,uint8_t frameType,uint8_t serverType)
{
	uint8_t * ptrData;
	u8 err;
	uint16_t frameLen= 10;
	uint16_t offset = 0;
	uint16_t sendLen;
	u8 * sendCmd;
	
	frameLen = datLen+10;
	ptrData = Mem_malloc(250);
	ptrData[offset++] = 0x7E;
	ptrData[offset++] = (uint8_t)((frameLen>>0) & 0x00FF);
	ptrData[offset++] = (uint8_t)((frameLen>>8) & 0x00FF);
	ptrData[offset++] = 0x00;		//报文计数
	ptrData[offset++] = 0x00;		//
	ptrData[offset++] = frameType;	//报文类型
	ptrData[offset++] = serverType;	//服务类型
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	
	//合成预发送字符串
	sendCmd = Mem_malloc(30);
	sendLen = sprintf((char *)sendCmd,(const char*)atCIPSEND,(frameLen+4));
	WIFISendDatas(sendCmd,sendLen);
	Mem_free((void **)&sendCmd);
	
	sendCmd = OSQPend(WIFICmdQ,200,&err);
	if(err == OS_ERR_NONE)
	{
		if(*sendCmd == '>')
		{
			Mem_free((void **)&sendCmd);
			
			memcpy(&ptrData[offset],ptrToSend,datLen);
			RecvPackAnalysis(ptrData,0);
			WIFISendDatas(ptrData,(frameLen+4));
			sendCmd = OSQPend(WIFICmdQ,200,&err);
			Mem_free((void **)&sendCmd);
		}
	}
	Mem_free((void **)&ptrData);
}















