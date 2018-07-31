
#include "includes.h"
extern OS_EVENT * CANRecvQ;				//CAN ������Ϣ���е�ָ��
extern BXWAllData varBXWRun;	
 u8 manyPack[8] = {0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//���ն��
extern pSTORE     BXW_CAN;		//��Я������ CAN ����Ϣ
extern OS_EVENT * BXWFlowSem;				//��Я�������������ź���
extern BXWCAN_Data canBXWToSend;
extern OS_EVENT * canRCV30;				// VCI ���յ�0x30ָ�� �ź���
extern OS_EVENT * BXWSafe;				//��Я������ȫ�㷨
extern OS_EVENT * testMbox;				//ͨѶ��� ��Ϣ����
extern uint8_t testMode[2];
void CANTask(void *pdata)
{
	uint8_t err,i;
	CanRxMsg *RxMessage;
	uint8_t *ptrCANDat;
	uint16_t bxwCandatLen = 0;
	while(1)
	{
		RxMessage = OSQPend(CANRecvQ,0,&err);
		if(varOperaRun.sysMode == BXW_MODE)
		{			
			//�ö�����չ֡
			bxwCandatLen = Store_Getlength(BXW_CAN);
			if(bxwCandatLen>300)//�Ѵ���������Ƚϴ󣬷����˰�
			{
				Mem_free((void **)&RxMessage);
				continue;
			}//�ж��Ƿ��ǰ�ȫ�㷨����ȡseed
			if(RxMessage->ExtId == canBXWToSend.receId)
			{
				switch(canBXWToSend.safeCH)
				{
					case 0:break;
					case 1://������ȫ�㷨�ǹ̶���
						OSSemPost(BXWSafe);
						continue;
					case 2://��������ȫ�㷨
						memcpy(canBXWToSend.safeSeed,&RxMessage->Data[3],4);
						OSSemPost(BXWSafe);
						continue;
					default:break;
				}
			}
			ptrCANDat = Mem_malloc(30);
			ptrCANDat[0] = 13;ptrCANDat[1] = 0x00;
			ptrCANDat[2] = 9;
			if(RxMessage->IDE == CAN_ID_EXT)
				memcpy(&ptrCANDat[3],&RxMessage->ExtId,4);
			else
				memcpy(&ptrCANDat[3],&RxMessage->StdId,4);
			memcpy(&ptrCANDat[7],RxMessage->Data,8);

			Store_Pushs(BXW_CAN,ptrCANDat,15);
			
			if(canBXWToSend.flowRecId==RxMessage->ExtId||canBXWToSend.flowRecId==RxMessage->ExtId)
				OSSemPost(BXWFlowSem);//������ CANID
			Mem_free((void **)&ptrCANDat);
			Mem_free((void **)&RxMessage);
		}else if(varOperaRun.sysMode == VCI_MODE)
		{
			ptrCANDat = Mem_malloc(30);
			ptrCANDat[0] = 0x0D;ptrCANDat[1] = 0x00;
			ptrCANDat[2] = varOperaRun.cmdIdCAN + 0x40;
			if(canVCIData.std_exd == CAN_ID_EXT)
				memcpy(&ptrCANDat[3],&RxMessage->ExtId,4);
			else
				memcpy(&ptrCANDat[3],&RxMessage->StdId,4);
			memcpy(&ptrCANDat[7],RxMessage->Data,8);
			if((RxMessage->Data[0] == 0x10)&&(varOperaRun.manyPackFlag == OPEN))//sidΪ0x18��ָ�������
			{
				if(canVCIData.canCH == 0||canVCIData.canCH == 1)//CAN1 2ͨ��
					Can1_SendData(varOperaRun.sendCANID,canVCIData.std_exd,(u8 *)manyPack);//����
				else if(canVCIData.canCH == 2||canVCIData.canCH == 3)//CAN3 4ͨ��
					Can2_SendData(varOperaRun.sendCANID,canVCIData.std_exd,(u8 *)manyPack);//����
			}
			else if((RxMessage->Data[0] == 0x30)&&(varOperaRun.manyPackFlag == OPEN))
			{
				OSSemPost(canRCV30);//֪ͨ sid - 0x17 ���յ�ECU��������0x30ָ�� 
				Mem_free((void **)&ptrCANDat);
				Mem_free((void **)&RxMessage);
				continue;
			}
			//todo:30ָ���Ҫ�ϱ�
			DateToSend(ptrCANDat,15,0x02,0x09);
			Mem_free((void **)&ptrCANDat);
			Mem_free((void **)&RxMessage);
		}
		else if(varOperaRun.sysMode == TEST_PORT_SEND_MODE)//ͨѶ���� ���� ģʽ
		{
			if(OSMboxPost(testMbox,RxMessage) != OS_ERR_NONE)
				Mem_free((void **)&RxMessage);
		}
		else if(varOperaRun.sysMode == TEST_PORT_RECV_MODE)//ͨѶ���� ���� ģʽ
		{	
			for(i = 0;i<8;i++)
			{
				RxMessage->Data[i]++;
			}
			if(testMode[0] == 1 || testMode[0] == 2)
			{
				Can1_SendData(RxMessage->ExtId,CAN_ID_EXT,RxMessage->Data);	
			}
			else if(testMode[0] == 3 || testMode[0] == 4)
			{
				Can2_SendData(RxMessage->ExtId,CAN_ID_EXT,RxMessage->Data);	
			}
			Mem_free((void **)&RxMessage);
		}
	}
}
extern OS_EVENT * KLineRecvQ;	
u8 K_CheckSum(u8 *ptrK,u16 datLen);
//K�߽�����������Ҫ���ϱ����յ�����Ϣ
void KLineTask(void *pdata)
{
	uint8_t  err;
	uint8_t  checkIsOK = 0;	
	uint8_t  *ptrKDat;
	uint16_t datLen;

	while(1)
	{
		ptrKDat = OSQPend(KLineRecvQ,0,&err);
		
		memcpy(&datLen,ptrKDat,2);//K�߷��ص����ݳ���
		
		checkIsOK = K_CheckSum(&ptrKDat[3],(datLen - 2));//��ȥCS����ȥsid
		if(checkIsOK != 0)//У�鲻�ɹ�
		{
			Mem_free((void **)&ptrKDat);
			continue;
		}
		//todo: ����  WiFi  USB
		DateToSend(ptrKDat,datLen+2,0x00,0x09);
		
		Mem_free((void **)&ptrKDat);
	}
}

u8 K_CheckSum(u8 *ptrK,u16 datLen)
{
	u8 checkK = 0;
	u16 i;
	for(i = 0;i<datLen;i++)
	{
		checkK += ptrK[i];
	}	
	if(checkK == ptrK[i])//У��ɹ�
		return 0;
	return 1;
}
void CANChannelCheck(u8 ch,u8 res);	//CANͨ���л���ƥ�����
//��Ϲ��� CAN K �����ݽ����ظ�
void CANDelay(int z)
{
	int i,j;
	for(i=0;i<100;i++)
		for(j = z;j > 0;j--);
}
extern void BXW_StartUp(STARTUP *ptr);
extern const char vciVer[30];
void FrameDeal(FrameStruct* ptrFrame)
{
	uint8_t * ptrData = NULL;
	VCICAN_Data *ptrCAN;
	uint8_t cmdId = 0,err;
	char    cmp;
	uint16_t datLen = 0,offset = 0;
	
	ptrData = (uint8_t *)ptrFrame;
	datLen = ptrData[12];
	datLen = (datLen<<8) + ptrData[11];
	cmdId  = ptrData[13];
	
	switch(cmdId)
	{
		case 0x03://todo: �������
			CANChannelCheck(0,0);//Ĭ�����,Debugʱ���Գ�����
			switch(ptrData[14])
			{
				case 0x00://CAN ���Զ�������
					if(ptrData[15] == 15)//CAN1AH ӳ�䵽  DB15    ͨ��1
					{
						
					}else if(ptrData[15] == 3)//CAN1BH ӳ�䵽  DB3     ͨ��2
					{
						
					}
					else if(ptrData[15] == 2)//CAN2AH ӳ�䵽 DB2     ͨ��3
					{
						
					}
					else if(ptrData[15] == 6)//CAN2BH ӳ�䵽 DB6    ͨ��4
					{
						
					}
					break;
				case 0x04://RS232�����Զ���             ȫ������֧��
				case 0x05://RS485/J1708�����Զ���		ȫ������֧��
				case 0x06:// K �������Զ���				ȫ������֧��
					break;
				default:break;
			}
			
			ptrData = Mem_malloc(20);	//�ظ�ȫ�����汾�ţ��˰���Ϊ�˼���VCI�ŷ��͵�����
			ptrData[0] = 0x01;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;	//0x53
			DateToSend(ptrData,3,0x02,0x09);
			Mem_free((void **)&ptrData);
			break;
			
		case 0x08:	case 0x0F:			//todo:�ر�����ͨ��  �ָ��ϵ�����
			varOperaRun.manyPackFlag = CLOSE;		
			varOperaRun.manyPackFlagDown = CLOSE;
//			memset(&canVCIData,0,sizeof(VCICAN_Data));//������ò���
//			K_Config(10400);				//������10400
//			varOperaRun.dat595 &= 0x0000FFFF;
//			LS595Deal(varOperaRun.dat595);	//�ر���· CAN  �Լ�ƥ�����
				
			ptrData = Mem_malloc(5);
			ptrData[0] = 1;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			DateToSend(ptrData,4,0x02,0x09);
			Mem_free((void **)&ptrData);
			break;
//		case 0x0A:
//			BXW_StartUp((STARTUP *)&ptrData[13]);
			break;
		case 0x10:	//���� CAN ������4�ֽ� +  1�ֽ�ƥ����� todo:ȷ��������֮��Ӧ�ĵ���
			memcpy((u8*)&canVCIData.canBaud,&ptrData[14],5);//���CAN������
//			CANChannelCheck(100,canVCIData.canRes);
//			CANChannelCheck(100,0);
//			//�ظ��汾��
			ptrData = Mem_malloc(50);
			ptrData[0] = 0x01;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			DateToSend(ptrData,4,0x02,0x09);
			Mem_free((void **)&ptrData);
			
			break;
			
		case 0x20:						// K �߿��ٳ�ʼ��
			varOperaRun.cmdIdK = cmdId;
			KLineFastInit(&ptrData[14],datLen-1);
			break;	
			
		case 0x22:	// K �ߴ���
			varOperaRun.cmdIdK = cmdId;
			KLineSendDatas(&ptrData[14],datLen-1);
			break;
		
		case 0x23:	//������ʼ�ֽڼ��ʱ�����
			//todo:K��ʼ��ָ���ֽ� ʱ�����
			if(varOperaRun.isVCIchangeCANTrue)//ƽ�������ò���
			{
				canVCIData.canCH = ptrData[15];	//ͨ��ѡ��
				CANChannelCheck(canVCIData.canCH,100);
			}
//			CANChannelCheck(0,100);
			ptrData = Mem_malloc(5);
			ptrData[0] = 1;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			DateToSend(ptrData,4,0x02,0x09);
			Mem_free((void **)&ptrData);
			break;
		case 0x14:	//��ͨ CAN ��֡����֡
			varOperaRun.cmdIdCAN = cmdId;
			ptrCAN = (VCICAN_Data *)&ptrData[13];
			memcpy((u8*)&canVCIData.cmdId,ptrCAN,23);//��ȡ2ID��MASK��Data
			
			if(canVCIData.receId < 0x80000000)//��׼֡
			{
				canVCIData.std_exd = CAN_ID_STD;
				varOperaRun.receCANID = canVCIData.receId;
				varOperaRun.sendCANID = canVCIData.sendId;
			}
			else//��չ֡
			{
				canVCIData.std_exd = CAN_ID_EXT;
				varOperaRun.receCANID = canVCIData.receId - 0x80000000;
				if(canVCIData.sendId > 0x80000000)
					varOperaRun.sendCANID = canVCIData.sendId - 0x80000000;
				else//(���ݿ���˹)
					varOperaRun.sendCANID = canVCIData.sendId;
			}
			if(canVCIData.canCH == 1||canVCIData.canCH == 3)//���� CAN2 4ͨ��
				CAN1Config(varOperaRun.receCANID,canVCIData.std_exd,canVCIData.mask,canVCIData.canBaud);//CAN1 ����
			else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//���� CAN1 3ͨ��
				CAN2Config(varOperaRun.receCANID,canVCIData.std_exd,canVCIData.mask,canVCIData.canBaud);//CAN1 ����
			
			memcpy((u8*)&canVCIData.data,ptrCAN->data,8);
			cmp = memcmp(manyPack,ptrCAN->data,8);
			if(cmp != 0)											//������������ 30 ָ��Ļ���ֱ�Ӻ���
			{
				if(canVCIData.canCH == 1||canVCIData.canCH == 3)	//CAN 2 4 ͨ��
					Can1_SendData(varOperaRun.sendCANID,canVCIData.std_exd,canVCIData.data);//����
				else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//CAN 1 3 ͨ��
					Can2_SendData(varOperaRun.sendCANID,canVCIData.std_exd,canVCIData.data);//����
			}
			offset = 0x0018;
			if(offset >= datLen)//�����Ļ��������Ѿ�����
				break;	
			else				//���͵�һ��֮��İ�
			{
				do{
					if(canVCIData.canCH == 1||canVCIData.canCH == 3)	 //CAN2 4ͨ��
						Can1_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//����
					else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//CAN1 3ͨ��
						Can2_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//����
					offset += 8;
					CANDelay(30);//��ʱ�Ǳ�Ҫ�ģ�����ᶪʧ���������ܷɳ���
				}while(offset < datLen);
			}break;
		case 0x15://��ͨ��֡
			break;
		case 0x17://�����������λ�����͵�30ָ��
			varOperaRun.manyPackFlagDown = OPEN;
			varOperaRun.cmdIdCAN = cmdId;
			ptrCAN = (VCICAN_Data *)&ptrData[13];
			cmp = memcmp(ptrCAN,&canVCIData,13);//�ȽϷ���ID������ID��mask�Ƿ���ͬ
			if(cmp != 0)
			{
				memcpy((u8*)&canVCIData.cmdId,ptrCAN,23);//��ȡ2��CANID��MASK��Data
				if(canVCIData.receId < 0x80000000)//��׼֡
				{
					canVCIData.std_exd = CAN_ID_STD;
					varOperaRun.receCANID = canVCIData.receId;
					varOperaRun.sendCANID = canVCIData.sendId;
				}
				else//��չ֡
				{
					canVCIData.std_exd = CAN_ID_EXT;
					varOperaRun.receCANID = canVCIData.receId - 0x80000000;
					if(canVCIData.sendId > 0x80000000)
						varOperaRun.sendCANID = canVCIData.sendId - 0x80000000;
					else//(���ݿ���˹)
						varOperaRun.sendCANID = canVCIData.sendId;
				}
				if(canVCIData.canCH == 1||canVCIData.canCH == 3)//���� CAN1 2ͨ��
					CAN1Config(varOperaRun.receCANID,canVCIData.std_exd,canVCIData.mask,canVCIData.canBaud);//CAN1 ����
				else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//���� CAN3 4ͨ��
					CAN2Config(varOperaRun.receCANID,canVCIData.std_exd,canVCIData.mask,canVCIData.canBaud);//CAN1 ����
			}else{
				memcpy((u8*)&canVCIData.data,ptrCAN->data,8);
			}
			cmp = memcmp(manyPack,ptrCAN->data,8);
			if(cmp != 0)//������ 30 ָ��Ļ���ֱ�Ӻ���
			{
				if(canVCIData.canCH == 1||canVCIData.canCH == 3)	 //CAN2 4ͨ��
					Can1_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//����
				else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//CAN1 3ͨ��
					Can2_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//����
			}
			offset = 0x0018;
			if(offset >= datLen)//�����Ļ��������Ѿ�����
				break;	
			else		//���͵�һ��֮��İ�
			{
				OSSemPend(canRCV30,20,&err);//�ȴ�ECU�ظ�0x30��
				if(err != OS_ERR_NONE)
					break;
				do{
					if(canVCIData.canCH == 1||canVCIData.canCH == 3)	 //CAN2 4ͨ��
						Can1_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//����
					else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//CAN1 3ͨ��
						Can2_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//����
					offset += 8;
					CANDelay(30);
				}while(offset < datLen);
			}break;
		case 0x18://������λ���Ķ�������͹涨��30ָ��
				varOperaRun.manyPackFlag = OPEN;
				//���ƽ�Ҫ���͵�30ָ�ptrData[14]Ŀǰ�����������
				memcpy(manyPack,&ptrData[15],8);//����涨��0x30ָ��
			break;
		default: break;
	}
}
void CANChannelCheck(u8 ch,u8 res)//CANͨ��ѡ�񡢵���ƥ�����
{
	uint16_t dat595 = 0x0000;
	switch(ch)
	{
		case 0x00://ѡ�� CAN  1ͨ��         CAN2A
			dat595 += (1<<12);//��  can2  ѡ��CAN2A
			dat595 += (1<<13);        
			dat595 += (1<<15);//120ŷķ����
			
			break;
		case 0x01://ѡ�� CAN  2ͨ��			CAN1A
			dat595 += (1<<11);	//��  can1A  Ĭ��CAN1A
			dat595 += (1<<8);	//120ŷķ����
			break;
		case 0x02://ѡ�� CAN  3ͨ��			CAN2B
			dat595 += (1<<12);	//��  can2  Ĭ��CAN2B	
			dat595 += (1<<15);	//120ŷķ����
			break;
		case 0x03://ѡ�� CAN  4ͨ��			CAN1B
			dat595 += (1<<11);	//��  can1B  ѡ��CAN1B
			dat595 += (1<<10);	
			dat595 += (1<<8);	//120ŷķ����//120ŷķ����
			break;
		default:
			break;
	}
	
//	if(canVCIData.canCH == 2||canVCIData.canCH == 4)//CAN  2  4  ͨ��
//	{
//		switch(res)
//		{
//			case 0x00://CAN  1  2 ͨ��ƥ�� 120 ŷķ���� 
//				varOperaRun.dat595 &= 0x3CFF;
//				varOperaRun.dat595 |= 0x0100;
//				
//				break;
//			case 0x01://CAN  1  2 ͨ��ƥ�� 60  ŷķ���� 
//				varOperaRun.dat595 &= 0x3CFF;
//				varOperaRun.dat595 |= 0x0300;
//				break;
//			case 0x02://CAN  1  2 ͨ����ƥ����� 
//				varOperaRun.dat595 &= 0x3CFF;
//				break;
//			default:
//				break;
//		}
//	}else if(canVCIData.canCH == 1||canVCIData.canCH == 3)//CAN 1  3  ͨ��
//	{
//		switch(res)
//		{
//			case 0x00://CAN  3  4 ͨ��ƥ�� 120 ŷķ���� 
//				varOperaRun.dat595 &= 0x3CFF;
//				varOperaRun.dat595 |= 0x4000;
//				break;
//			case 0x01://CAN  3  4 ͨ��ƥ�� 60  ŷķ���� 
//				varOperaRun.dat595 &= 0x3CFF;
//				varOperaRun.dat595 |= 0xC000;
//				break;
//			case 0x02://CAN  3  4 ͨ����ƥ����� 
//				varOperaRun.dat595 &= 0x3CFF;
//				break;
//			default:
//				break;
//		}
//	}
	dat595 += (varOperaRun.dat595 & 0x00FF);
	varOperaRun.dat595 = dat595;
	LS595Deal(varOperaRun.dat595);	
}

















