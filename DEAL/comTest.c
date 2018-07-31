

//此文件用来处理 通讯测试

#include "includes.h"
extern uint8_t RS232SendDatas(const uint8_t* s,uint16_t length);
extern uint8_t RS485SendDatas(const uint8_t* s,uint16_t length);


extern void Test232RxDataDeal(u8 * ptr);//配置各通讯接口
extern OS_EVENT * testMbox;		//通讯检测 消息邮箱
uint8_t testSend[2]={0,0};
void ComTest(void)
{
	uint8_t * prtToSend;
	uint8_t * prtRecv;
	uint8_t recFlag = 0;
	CanRxMsg *RxMessage;
	uint8_t i,j,z,d,err;
	uint8_t sendNum = 0;
	uint8_t recvNum = 0;
	uint8_t rightNum = 0;
	uint32_t SendId = 0x18DA00FA;
	uint8_t sendData[20] = {0xAA,0x55,0x00,0x11,0x22,0x33,0x44,0x55,0x89,0x74,\
							0xAC,0xDB,0x0f,0xF0,0x42,0x81,0x18,0x66,0x03,0x23};
	
	prtToSend = Mem_malloc(10);
	
	RS232_UartConfig(9600);     //配置232
	
	testSend[0]=0xAA; testSend[1]=0xBB;
	RS232SendDatas(testSend,2);
	OSTimeDlyHMSM(0,0,0,8);
	prtToSend[0] = 5;prtToSend[1] = 0;
	for(i = 1;i < 5;i++)//校验四路 CAN
	{
		sendNum = 0;recvNum = 0;rightNum = 0;
		
		testSend[0]= i; testSend[1]=0x01;
		RS232SendDatas(testSend,2);
		Test232RxDataDeal(testSend);//配置端口
		LS595Deal(varOperaRun.dat595);
		OSTimeDlyHMSM(0,0,0,80);
		for(z=0;z<15;z++)
		{
			sendNum++;
			if(i == 1 || i == 2)
			{
				Can1_SendData(SendId,CAN_ID_EXT,sendData);
				RxMessage = OSMboxPend(testMbox,10,&err);
				if(err == OS_ERR_NONE)
				{
					recvNum ++;
					recFlag = 0;
					if(RxMessage->ExtId == SendId)
						recFlag ++;
					for(d = 0;d<8;d++)
					{
						if(RxMessage->Data[d] == (sendData[d]+1))
							recFlag++;
					}
					if(recFlag == 9)
						rightNum ++;
					Mem_free((void**)&RxMessage);
				}
			}else
			{
				Can2_SendData(SendId,CAN_ID_EXT,sendData);
				RxMessage = OSMboxPend(testMbox,10,&err);
				if(err == OS_ERR_NONE)
				{
					recvNum ++;recFlag = 0;
					if(RxMessage->ExtId == SendId)
						recFlag ++;
					for(d = 0;d<8;d++)
					{
						if(RxMessage->Data[d] == (sendData[d]+1))
							recFlag++;
					}
					if(recFlag == 9)
						rightNum ++;
					Mem_free((void**)&RxMessage);
				}
			}
		}
		prtToSend[2] = i;prtToSend[3] = 1;
		prtToSend[4] = sendNum;prtToSend[5] = recvNum;prtToSend[6] = rightNum;
		DateToSend(prtToSend,7,0x02,0x7F);
		for(j = 2;j < 5;j ++)
		{
			testSend[1]= j;
			RS232SendDatas(testSend,2);
			Test232RxDataDeal(testSend);//配置端口
			LS595Deal(varOperaRun.dat595);
			sendNum = 0;recvNum = 0;rightNum = 0;
			for(z=0;z<15;z++)
			{
				sendNum++;
				if(i == 1 || i == 2)
				{
					Can1_SendData(SendId,CAN_ID_EXT,sendData);
					RxMessage = OSMboxPend(testMbox,10,&err);
					if(err == OS_ERR_NONE)
					{
						recvNum ++;recFlag = 0;
						if(RxMessage->ExtId == SendId)
							recFlag ++;
						for(d = 0;d<8;d++)
						{
							if(RxMessage->Data[d] == (sendData[d]+1))
								recFlag++;
						}
						if(recFlag == 9)
							rightNum ++;
						Mem_free((void**)&RxMessage);
					}
				}else
				{
					Can2_SendData(SendId,CAN_ID_EXT,sendData);
					RxMessage = OSMboxPend(testMbox,10,&err);
					if(err == OS_ERR_NONE)
					{
						recvNum ++;recFlag = 0;
						if(RxMessage->ExtId == SendId)
							recFlag ++;
						for(d = 0;d<8;d++)
						{
							if(RxMessage->Data[i] == (sendData[i]+1))
								recFlag++;
						}
						if(recFlag == 9)
							rightNum ++;
						Mem_free((void**)&RxMessage);
					}
				}
			}
			prtToSend[2] = i;prtToSend[3] = j;
			prtToSend[4] = sendNum;prtToSend[5] = recvNum;prtToSend[6] = rightNum;
			DateToSend(prtToSend,7,0x02,0x7F);
		}
	}//K 线测试
	prtToSend[2] = 5;testSend[0] = 5;
	for(i = 5;i<9;i++)
	{
		sendNum = 0;recvNum = 0;rightNum = 0;
		prtToSend[3] = i;testSend[1] = i;
		RS232SendDatas(testSend,2);
		Test232RxDataDeal(testSend);//配置端口
		OSTimeDlyHMSM(0,0,0,10);
		for(z= 0;z<10;z++)
		{
			sendNum++;
			KLineSendDatas(sendData,20); 
			prtRecv = OSMboxPend(testMbox,35,&err);
			if(err == OS_ERR_NONE)
			{
				recvNum ++;recFlag = 0;
				for(d = 0;d<20;d++)
				{
					if(prtRecv[d] == (sendData[d]+1))
						recFlag++;
				}
				if(recFlag == 20)
					rightNum ++;
				Mem_free((void**)&prtRecv);
			}
		}
		prtToSend[4] = sendNum;prtToSend[5] = recvNum;prtToSend[6] = rightNum;
		DateToSend(prtToSend,7,0x02,0x7F);
	}
	//485 线测试
	prtToSend[2] = 6;testSend[0] = 6;
	for(i = 5;i<9;i++)
	{
		sendNum = 0;recvNum = 0;rightNum = 0;
		prtToSend[3] = i;testSend[1] = i;
		RS232SendDatas(testSend,2);
		Test232RxDataDeal(testSend);//配置端口
		OSTimeDlyHMSM(0,0,0,10);
		for(z= 0;z<10;z++)
		{
			sendNum++;
			RS485SendDatas(sendData,20); 
			prtRecv = OSMboxPend(testMbox,35,&err);
			if(err == OS_ERR_NONE)
			{
				recvNum ++;recFlag = 0;
				for(d = 0;d<20;d++)
				{
					if(prtRecv[d] == (sendData[d]+1))
						recFlag++;
				}
				if(recFlag == 20)
					rightNum ++;
				Mem_free((void**)&prtRecv);
			}
		}
		prtToSend[4] = sendNum;prtToSend[5] = recvNum;prtToSend[6] = rightNum;
		DateToSend(prtToSend,7,0x02,0x7F);
	}
	//232 线测试
	prtToSend[2] = 7;testSend[0] = 7;
	for(i = 5;i<9;i++)
	{
		sendNum = 0;recvNum = 0;rightNum = 0;
		prtToSend[3] = i;testSend[1] = i;
		RS232SendDatas(testSend,2);
		OSTimeDlyHMSM(0,0,0,10);
		Test232RxDataDeal(testSend);//配置端口
		OSTimeDlyHMSM(0,0,0,10);
		for(z= 0;z<10;z++)
		{
			sendNum++;
			RS232SendDatas(sendData,20); 
			prtRecv = OSMboxPend(testMbox,40,&err);
			if(err == OS_ERR_NONE)
			{
				recvNum ++;recFlag = 0;
				for(d = 0;d<20;d++)
				{
					if(prtRecv[d] == (sendData[d]+1))
						recFlag++;
				}
				if(recFlag == 20)
					rightNum ++;
				Mem_free((void**)&prtRecv);
			}
		}
		prtToSend[4] = sendNum;prtToSend[5] = recvNum;prtToSend[6] = rightNum;
		DateToSend(prtToSend,7,0x02,0x7F);
	}
	OSTimeDlyHMSM(0,0,0,10);
	prtRecv = OSMboxPend(testMbox,40,&err);
	if(err == OS_ERR_NONE)
	{
		Mem_free((void**)&prtRecv);
	}
	testSend[0] = 0xAA;
	testSend[1] = 0xBB;
	RS232SendDatas(testSend,2);
	
	Mem_free((void **)&prtToSend);
}




