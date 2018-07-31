#include "includes.h"


extern OS_EVENT * MessageRecvQ;			//ƽ�� ������Ϣ���е�ָ��
extern OS_EVENT * CANRecvQ;				//CAN ������Ϣ���е�ָ��
extern OS_EVENT * KLineRecvQ;			//K�� ������Ϣ���е�ָ��

extern uint8_t BTSendDatas(const uint8_t* s,uint16_t length);
extern void FrameDeal(FrameStruct* ptrFrame);
extern void RcvSeed_CatchMask(FrameStruct* ptrFrame);//��ȫ�㷨������
void MotorDeal(uint8_t * ptrFrame);
const char vciVer[] = "CSR 2nd VCI by M&D 20180620\r\n";
//�����ƽ����յ�����Ϣ������USB��������WIFI
//extern uint8_t flag7E;
void MessageDealTask(void *pdata)
{ 
	u8 err,sumErr,flag;
	u8 * ptrBTRecv;
	u8 * ptrDeal;
	u8 * ptrtosend;
	uint16_t frameLen;
	uint16_t offset,i,j;
	FrameStruct * ptrFrame;
	
	while(1)
	{
		offset = 0;
		flag = 0;
		ptrBTRecv = OSQPend(MessageRecvQ,1500,&err);//�����յ���Ϣ�����д���
		if(err != OS_ERR_NONE)				//5sû�н��ܵ�����
		{
			varOperaRun.ledFlag[1] = LEDOFF;//����ͨѶָʾ����
//			flag7E = 0;
			continue;
		}
		varOperaRun.ledFlag[1] = LEDON;		//����ͨѶָʾ����
		memcpy(&frameLen,ptrBTRecv,2);		//�õ�Ҫ���͵�֡����
		//���� PPP Э���� 0x7E   0x7D  ������
		for(i = 2;i < (frameLen	+ 2);i++)
		{
			if(ptrBTRecv[i] == 0x7E)
			{
				flag ++;
				if(flag % 2 == 0)	//֡β 0x7E  ��������  ������һ��
				{
					ptrDeal[offset++] = 0x7E;
					sumErr = RecvPackAnalysis(ptrDeal,1);//У���������֡��CheckSum
					if(sumErr != 0)						 //У��ʧ��
					{
						Mem_free((void **)&ptrDeal);
						continue;
					}//�������н��յ�������
					ptrFrame = (FrameStruct *)ptrDeal;
					switch(ptrFrame->serverType)
					{	
						case 0x0A:			// ֪ͨ�汾��Ϣ �ɰ汾�����°汾
							ptrtosend = Mem_malloc(50);
							ptrtosend[0] = sizeof(vciVer);
							ptrtosend[1] = 0x00;
							ptrtosend[2] = ptrDeal[13] + 0x40;
							memcpy(&ptrtosend[3],vciVer,sizeof(vciVer));
							DateToSend(ptrtosend,sizeof(vciVer)+2,0x02,0x0A);
							Mem_free((void **)&ptrtosend);
							break;
						case 0x01:			// ԭ VCI CCP �ϴ�
							break;
						case 0x0C:			// VCI ����ȫ�㷨
							RcvSeed_CatchMask(ptrFrame);
							break;
					
						case 0x0F:			// ��Я��
							varOperaRun.sysMode = BXW_MODE;	//	��Я��ģʽ		
							MotorDeal(ptrDeal);
							break;
							
						case 0x09:			// �������
							varOperaRun.sysMode = VCI_MODE;	//���ģʽ
							FrameDeal(ptrFrame);			// VCI ���ָ���
							break;

						case 0x0E:			// OTA ����
							varOperaRun.sysMode = IDLE_MODE;//��ģʽ
							OTA_Updata(&ptrDeal[11]);
							break;
						case 0x7F:			//��װ CAN��K��485��232 ͨѶ�Զ������
							varOperaRun.sysMode = TEST_PORT_SEND_MODE;//ͨѶ���ģʽ
							ComTest();

							break;
						default:	
							break;
					}
					Mem_free((void **)&ptrDeal);//����������Ҫ�ͷ��ڴ��
				}else					//֡ͷ 0x7E
				{
					offset = 0;
					ptrDeal = Mem_malloc(frameLen + 2 - i);
					if(ptrDeal == NULL)//�ڴ治������
					{
						ptrDeal = NULL;
						break;
					}
					ptrDeal[offset++] = 0x7E;
				}
			}
			else if((ptrBTRecv[i] == 0x7D) && (ptrBTRecv[i+1] == 0x5E)) //��ԭ 0x7E
			{
				ptrDeal[offset++] = 0x7E;
				i++;
			}else if((ptrBTRecv[i] == 0x7D) && (ptrBTRecv[i+1] == 0x5D))//��ԭ 0x7D
			{
				ptrDeal[offset++] = 0x7D;
				i++;
			}
			else if(flag % 2 == 1)//���������� �Ѿ�����֡ͷ
			{
				ptrDeal[offset++] = ptrBTRecv[i];
			}
		}
		Mem_free((void **)&ptrDeal);	//�ͷ�
		Mem_free((void **)&ptrBTRecv);
	}
}
extern const u8 atCIPSEND[];
extern OS_EVENT * WIFICmdQ;				//���յ�WiFi������Ϣ
extern u8 WIFISendDatas(u8* s,u16 length);
extern OS_EVENT * USBSendQ;	
//������������
void DateToSend(uint8_t * ptrToSend,uint16_t datLen,uint8_t frameType,uint8_t serverType)
{
	uint8_t err;
	uint8_t * ptrData;
	uint8_t * ptrOK;
	uint8_t * usbSend;
	uint8_t * sendCmd;
	uint16_t sendLen;
	uint16_t frameLen= 10;
	uint8_t frameDevNums = 0;
	uint8_t frameDevNum = 0;
	uint16_t offset = 0,i = 0;
	
	if(datLen % 2 == 1)				//������ ��������
		datLen ++;
		
	frameLen = datLen+10;
	ptrData = Mem_malloc(frameLen+4);
	ptrOK   = Mem_malloc(frameLen+18);
	if(ptrData == NULL || ptrOK == NULL)
		return;
	ptrData[offset++] = 0x7E;
	ptrData[offset++] = (uint8_t)((frameLen>>0) & 0x00FF);
	ptrData[offset++] = (uint8_t)((frameLen>>8) & 0x00FF);
	ptrData[offset++] = 0x00;		//���ļ���
	ptrData[offset++] = 0x01;		//
	ptrData[offset++] = frameType;	//��������
	ptrData[offset++] = serverType;	//��������
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	
	memcpy(&ptrData[offset],ptrToSend,datLen);
	RecvPackAnalysis(ptrData,0);
	offset = 0;
	ptrOK[offset++] = 0x7E;
	//��Ҫ���͵����ݽ���  PPP Э�鴦�� 0x7E-0x7D 0x5E    0x7D-0x7D  0x5D
	for(i = 1;i<frameLen+3;i++)
	{
		if(ptrData[i] == 0x7E)
		{
			ptrOK[offset++] = 0x7D;
			ptrOK[offset++] = 0x5E;
		}else if(ptrData[i] == 0x7D)
		{
			ptrOK[offset++] = 0x7D;
			ptrOK[offset++] = 0x5D;
		}
		else
		{
			ptrOK[offset++] = ptrData[i];
		}
	}
	ptrOK[offset++] = 0x7E;
	//USBģʽ����
	if(varOperaRun.wirelessMode == USB_MODE)
	{
		frameDevNums = offset % 61 == 0? offset / 61:offset / 61 + 1;
		frameDevNum = 1;
		while(frameDevNum<=frameDevNums)
		{
			usbSend = Mem_malloc(64);
			if(usbSend == NULL)
				break;
			usbSend[1] = frameDevNums;
			usbSend[2] = frameDevNum;
			if(frameDevNum<frameDevNums)
			{
				memcpy(&usbSend[3],&ptrOK[(frameDevNum-1)*61],61);
			}else
			{
				memcpy(&usbSend[3],&ptrOK[(frameDevNum-1)*61],offset - ((frameDevNum-1)*61));
			}
			if(OSQPost(USBSendQ,usbSend)!=OS_ERR_NONE)
			{
				Mem_free((void **)&usbSend);
				break;
			}
			frameDevNum++;
		}
	}
	else if(varOperaRun.wirelessMode == BT_MODE)	//ͨ��������������
		BTSendDatas(ptrOK,offset);
	else if(varOperaRun.wirelessMode == WIFI_MODE)//ͨ��WIFI��������
	{
		//�ϳ�Ԥ�����ַ���
		sendCmd = Mem_malloc(30);
		sendLen = sprintf((char *)sendCmd,(const char*)atCIPSEND,offset);
		WIFISendDatas(sendCmd,sendLen);
		Mem_free((void **)&sendCmd);
		
		sendCmd = OSQPend(WIFICmdQ,200,&err);
		if(err == OS_ERR_NONE)
		{
			if(*sendCmd == '>')
			{
				Mem_free((void **)&sendCmd);
				WIFISendDatas(ptrOK,offset);
				sendCmd = OSQPend(WIFICmdQ,200,&err);
				Mem_free((void **)&sendCmd);
//				sendCmd = OSQPend(WIFICmdQ,2,&err);
//				Mem_free((void **)&sendCmd);
			}
		}
	}
	//�ͷ�������ڴ��
	Mem_free((void **)&ptrOK);
	Mem_free((void **)&ptrData);
	
}
extern PWM_Control PWMDat[9];
extern DCM_Control DCMDat[4];
void BXWCmdDeal(uint8_t *ptrdata);
void MotorDeal(uint8_t * ptrFrame)
{
	uint8_t i;
	uint8_t offset;
	uint16_t datLen = 0;
	uint16_t cmdLen =0;
	uint16_t frameLen = 0;
	
	frameLen = ptrFrame[2];				//��֡���ݵĳ���
	frameLen = (frameLen<<8) + ptrFrame[1];
	frameLen -= 10;						//����������
	
	offset 	 = 11;
	for(i = 0;i < 11;i++)				//���һ���Դ���11��
	{
		cmdLen = ptrFrame[offset + datLen+1];
		cmdLen = (cmdLen<<8) + ptrFrame[offset + datLen];
		BXWCmdDeal(&ptrFrame[offset + datLen + 2]);//cmd
		datLen += cmdLen+2;
		if((datLen == (frameLen - 1))||(datLen >= frameLen))
			break;
	}
}

extern void BXW_PWMDataDeal(PWM_Data * ptr,uint8_t PWMId);
extern void BXW_DCMDataDeal(DCM_Data * ptr,uint8_t PWMId);
void BXW_CANdataDeal(BXWCAN_Data *ptr);
extern TIMEOUTPUT_pwm pwmOC[8];			//8·��ռ�ձȴ�Ƶ�ʷ�����
extern TIMEOUTPUT_pwm lowPWMOC[8];		//�ɲ���Сռ�ձ�
void BXW_StartUp(STARTUP *ptr);
void BXW_CANSafe(uint8_t * ptrSafe,uint8_t safeCH);
void BXWCmdDeal(uint8_t *ptrdata)
{
	u8 i;
	switch(*ptrdata)
	{
		case 0x01:	BXW_StartUp((STARTUP *)ptrdata);break;				//����ѡ��ͨѶģʽ���ն˵���
		
		//9·PWM���ο���
		case 0x02:	case 0x03:	case 0x04:	case 0x05:	case 0x06:	
		case 0x07:	case 0x08:	case 0x09:	
		case 0x0A:	BXW_PWMDataDeal((PWM_Data *)ptrdata,(*ptrdata) - 2);break;
		
		//ֱ����� DCM ����
		case 0x0B:	case 0x0C:	
		case 0x0D:	BXW_DCMDataDeal((DCM_Data *)ptrdata,(*ptrdata) - 11);break;
		
		//�����������
		case 0x0F:	//��ȷ����
					//�������
		case 0x11:  BXW_DCMDataDeal((DCM_Data *)ptrdata,3);break;
		
		// CAN �ÿ���
		case 0x0E:	BXW_CANdataDeal((BXWCAN_Data *)ptrdata); break;
		case 0x12:	
		case 0x13:	
			BXW_CANSafe(ptrdata,(*ptrdata)-0x11);
			break;
		//�ر����� PWM ���
		case 0x10:	
			for(i = 0;i < 8;i++)
			{
				pwmOC[i].isOPEN = CLOSE;
				lowPWMOC[i].isOPEN = CLOSE;
				varBXWRun.dutyCyclePWM[i] = 0;//����ռ�ձ�����
			}
			break;
		case 0x1F:
			BXW_StartUp((STARTUP *)ptrdata);//VCI ģʽ�� CAN �л�
			break;
		default:break;
	}
}
extern BXWCAN_Data canBXWToSend;
extern OS_EVENT * BXWFlowSem;				//��Я�������������ź���
//ȫ���������֣�CAN2���ݷ���
void BXW_CANdataDeal(BXWCAN_Data *ptr)
{
	uint8_t err;
	u32 canFlowID;
	static u32 canbaud = 1000;
	static u32 canBaud = 0;
	static uint32_t mask = 10;		//����CANID �� ����mask
	
	//��ֵ������memcpy�����Ż�����
	canBXWToSend.sendId = ptr->sendId;		//����ID
	canBXWToSend.receId = ptr->receId;		//����ID
	canBXWToSend.mask   = ptr->mask;		//����
	canBXWToSend.baud   = ptr->baud;		//������
	canBXWToSend.std_exd = ptr->std_exd;	//֡����  0-��׼֡   1-��չ֡
	memcpy(canBXWToSend.data,ptr->data,8);	//Ҫ���͵�����
	switch(canBXWToSend.baud)
	{
		case 1:	canbaud = 250000;break;
		case 2:	canbaud = 500000;break;
		case 3:	canbaud = 1000000;break;
		default:break;
	}
	if(canBXWToSend.std_exd == 0)//��׼֡����չ֡
		canBXWToSend.canType = CAN_ID_STD;
	else 
		canBXWToSend.canType = CAN_ID_EXT;
	if((canbaud != canBaud)||(mask != canBXWToSend.mask))
	{
		CAN2Config(canBXWToSend.receId,canBXWToSend.canType,canBXWToSend.mask,canbaud);	//CAN2 ����
		canBaud = canbaud;
		mask = canBXWToSend.mask;
	}
	canFlowID  = (canBXWToSend.sendId>>8) & 0x000000FF;
	canFlowID += (canBXWToSend.sendId<<8) & 0x0000FF00;
	canFlowID += (canBXWToSend.sendId) & 0xFFFF0000;
	canBXWToSend.flowRecId = canFlowID;
	Can2_SendData(canBXWToSend.sendId,canBXWToSend.canType,canBXWToSend.data);		//
	OSSemPend(BXWFlowSem,20,&err);
}
extern OS_EVENT * BXWSafe;				//��Я������ȫ�㷨
//��������ȫ�㷨
extern ulong CalculateAikelanECU(uword EngineModel, ubyte *Seed, ubyte *Key);
const u8 kaiLongSafe[8] = {0x06,0x27,0x1C,0xD0,0x19,0x72,0xA9,0xAA};
// safeCH  0 - �������ݽ���     1 - ����  2 - ������
void BXW_CANSafe(uint8_t * ptrSafe,uint8_t safeCH)//�������������������ȫ�㷨
{
	uint8_t  err;
	OSTimeDlyHMSM(0,0,0,20);
	memcpy((uint8_t *)&canBXWToSend.receId,&ptrSafe[5],8);// ��ȡ CANID 
	memcpy(canBXWToSend.data,&ptrSafe[9],8);   			  // ��ȡ���͵�����
	
	canBXWToSend.safeCH = safeCH;
	Can2_SendData(canBXWToSend.sendId,canBXWToSend.canType,canBXWToSend.data);		//
	OSSemPend(BXWSafe,20,&err);//�ȴ� ECU �ظ� seed
	if(err == OS_ERR_NONE)
	{
		switch(safeCH)
		{
			case 0:
				break;
			case 1://�������� ��ȫ�㷨
				memcpy(canBXWToSend.data,kaiLongSafe,8);
				canBXWToSend.safeCH = 0;
				Can2_SendData(canBXWToSend.sendId,canBXWToSend.canType,canBXWToSend.data);//����Key
				break;
			case 2:
				canBXWToSend.data[0] = 0x06;
				canBXWToSend.data[1] = 0x27;
				canBXWToSend.data[2] = 0x0A;
				CalculateAikelanECU(0,canBXWToSend.safeSeed,&canBXWToSend.data[3]);	//����key
				canBXWToSend.safeCH = 0;
				Can2_SendData(canBXWToSend.sendId,canBXWToSend.canType,canBXWToSend.data);//����Key
				break;
			default:break;
		}
	}
}
//ȫ�����������������ʼ������Դѡ�� CAN ·ѡ��ƥ�����ѡ���
void BXW_StartUp(STARTUP *ptr)
{	
	uint16_t dat595 = 0x0000;
	uint8_t canChannel = 0;
	varOperaRun.startUp.powCheck = ptr->powCheck;		//��Դѡ��
	varOperaRun.startUp.commMode = ptr->commMode;		//ͨѶģʽ
	varOperaRun.startUp.terminalRes = ptr->terminalRes;	//�ն˵���
	
	switch(varOperaRun.startUp.powCheck)
	{
		//0 1 - 24V     X 0 - 12V    1 1-������
		case 0:	//0 V 
			GPIO_SetBits(GPIOC,GPIO_Pin_14);	//Pwr_24
			GPIO_SetBits(GPIOC,GPIO_Pin_13);	//Pwr_Sw
//			varOperaRun.ledFlag[0] = LEDOFF;	//��Դ����
			break;
		case 1:	//24V
			GPIO_ResetBits(GPIOC,GPIO_Pin_14);	//Pwr_24
			GPIO_SetBits(GPIOC,GPIO_Pin_13);	//Pwr_Sw
//			varOperaRun.ledFlag[0] = LEDON;		//��Դ����
			break;
		case 2:	//12V
			GPIO_SetBits(GPIOC,GPIO_Pin_14);	//Pwr_24
			GPIO_ResetBits(GPIOC,GPIO_Pin_13);	//Pwr_Sw
//			varOperaRun.ledFlag[0] = LEDON;		//��Դ����
			break;
		default:break;
	}
	switch(varOperaRun.startUp.commMode)
	{
		case 3://CAN1                 
			dat595 += (1<<12);//��  can2  ѡ��CAN2A  OBD-6/14   DB15-1/15
			dat595 += (1<<13); 
			canChannel = 1;   
			canVCIData.canCH = 0;		
			break;   
		case 4://CAN2
			dat595 += (1<<11);//��  can1  Ĭ��CAN1A  OBD-3/11   DB15-2/14
			canChannel = 2;    
			canVCIData.canCH = 1;				
			break;
		case 5://CAN3                 
			dat595 += (1<<12);//��  can2  Ĭ��CAN2B  OBD-1/9   DB15-3/11
			canChannel = 3;
			canVCIData.canCH = 2;	
			break;                   
		case 6://CAN4                 
			dat595 += (1<<11);//��  can1  ѡ��CAN1B  OBD-11/12   DB15-6/10
			dat595 += (1<<10);        
			canChannel = 4;    
			canVCIData.canCH = 3;				
			break;                    
		default:break;
	}
	if(canChannel != 0)
		varOperaRun.isVCIchangeCANTrue = 0;//VCI ͨ���л���Ч
	else
		varOperaRun.isVCIchangeCANTrue = 1;//VCI ͨ���л���Ч
		
	if(varOperaRun.startUp.terminalRes == 1)	//120ŷķ
	{
		if(canChannel == 2||canChannel == 4)	//CAN1A  B 
			dat595 += (1<<8);
		else if(canChannel == 1||canChannel == 3)//CAN2A  B  120ŷķ����
			dat595 += (1<<15);
	}
	else if(varOperaRun.startUp.terminalRes == 2)//60ŷķ
	{
		if(canChannel == 2||canChannel == 4)	//CAN1A  B   60ŷķ����
		{
			dat595 += (1<<8);
			dat595 += (1<<9);
		}
		else if(canChannel == 1||canChannel == 3)//CAN2A  B
		{
			dat595 += (1<<14);
			dat595 += (1<<15);
		}	
	}
	if(canChannel >= 1&&canChannel <= 4)
	{
		dat595 += (varOperaRun.dat595 & 0x00FF);
		varOperaRun.dat595 = dat595;
		LS595Deal(varOperaRun.dat595);
	}
}
	


























