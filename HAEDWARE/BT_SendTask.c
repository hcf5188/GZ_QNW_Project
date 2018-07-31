#include "includes.h"

extern OS_EVENT * BXWSendSem;	//��Я��ͨ�����������ź���
extern pSTORE     BXW_CAN;		//��Я������ CAN ����Ϣ
void BTSendTask(void *pdata)
{
	uint8_t err;
	uint8_t * ptrToSend;
	uint16_t  canDatLen;
	OSTimeDlyHMSM(0,0,5,0);
//	WIFI_Init();
	while(1)
	{
		OSSemPend(BXWSendSem,0,&err);
		ptrToSend = Mem_malloc(500);
		if(ptrToSend == NULL)
		{
			continue;
		}
		canDatLen = Store_Getlength(BXW_CAN);
		memcpy(ptrToSend,(u8 *)&varBXWRun,sizeof(varBXWRun));
		if(canDatLen>0)
			Store_Getdates(&ptrToSend[sizeof(varBXWRun)],BXW_CAN,canDatLen);
		else//��׿��Ҫ��û�� CAN ����ʱ��Ҫ��һ������� 09 00 41 00 00 
		{
			canDatLen = 15;
			ptrToSend[sizeof(varBXWRun)] = 13;
			ptrToSend[sizeof(varBXWRun)+2] = 9;
			ptrToSend[sizeof(varBXWRun)+4] = 65;
		}
		DateToSend(ptrToSend,sizeof(varBXWRun)+canDatLen,0x40,0x0F);
		Mem_free((void **)&ptrToSend);
	}
}

void ADDealTask(void *pdata)
{
	
}



