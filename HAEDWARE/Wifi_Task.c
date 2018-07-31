#include "includes.h"


const u8 atCmd[] = "AT\r";				//AT����ָ��
const u8 atRST[] = "AT+RST\r";			//��λָ��
const u8 atCIPMUX[] = "AT+CIPMUX=1\r";	//���ö�����
const u8 atCIPSERVER[] = "AT+CIPSERVER=1,8080\r";//���÷������˿ں�
const u8 atCWMODE[]	= "AT+CWMODE=2\r";	//����ģʽ����ΪAP  1-STA   2-AP   3-AP+STA
//����WIFI���ơ����롢���ܷ�ʽ
const u8 atNAMEPAS[] = "AT+CWSAP=\"ESP8266\",\"0123456789\",11,4\r";
const u8 atCIPSEND[] = "AT+CIPSEND=0,%d\r";	// AT ��������


extern OS_EVENT * WIFICmdQ;				//���յ� WiFi ������Ϣ
extern OS_EVENT * WIFIDatQ;				//���յ��豸������������Ϣ
void WIFI_SendCmd(const u8 *ptrCmd,char * comString,uint16_t sendLength);

//WIFIģ���ʼ��
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
extern pCIR_QUEUE sendBT_Q;		//ָ�� ���� ���ڷ��Ͷ���  ��ָ��
extern OS_EVENT * MessageRecvQ;	//��ƽ�������Ϣ���е�ָ��

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
	uint8_t err;			//������Ϣ
	uint8_t count = 0;		//����ֵ
	uint8_t cmpData;
	uint8_t *ptrWIFICfg;	//���յ� WIFI �ظ�����Ϣ
	do{
		count ++;
		WIFISendDatas((u8*)ptrCmd,sendLength);
		ptrWIFICfg = OSQPend(WIFICmdQ,200,&err);
		if(err != OS_ERR_NONE)        //��-�ճ�ʱ
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
	ptrData[offset++] = 0x00;		//���ļ���
	ptrData[offset++] = 0x00;		//
	ptrData[offset++] = frameType;	//��������
	ptrData[offset++] = serverType;	//��������
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	
	//�ϳ�Ԥ�����ַ���
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















