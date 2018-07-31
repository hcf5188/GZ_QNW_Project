#include "includes.h"


extern OS_EVENT * MessageRecvQ;			//平板 接收消息队列的指针
extern OS_EVENT * CANRecvQ;				//CAN 接收消息队列的指针
extern OS_EVENT * KLineRecvQ;			//K线 接收消息队列的指针

extern uint8_t BTSendDatas(const uint8_t* s,uint16_t length);
extern void FrameDeal(FrameStruct* ptrFrame);
extern void RcvSeed_CatchMask(FrameStruct* ptrFrame);//安全算法处理函数
void MotorDeal(uint8_t * ptrFrame);
const char vciVer[] = "CSR 2nd VCI by M&D 20180620\r\n";
//处理从平板接收到的信息，包括USB、蓝牙、WIFI
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
		ptrBTRecv = OSQPend(MessageRecvQ,1500,&err);//蓝牙收到消息，进行处理
		if(err != OS_ERR_NONE)				//5s没有接受到数据
		{
			varOperaRun.ledFlag[1] = LEDOFF;//无线通讯指示灯灭
//			flag7E = 0;
			continue;
		}
		varOperaRun.ledFlag[1] = LEDON;		//无线通讯指示灯亮
		memcpy(&frameLen,ptrBTRecv,2);		//得到要发送的帧长度
		//解析 PPP 协议中 0x7E   0x7D  的问题
		for(i = 2;i < (frameLen	+ 2);i++)
		{
			if(ptrBTRecv[i] == 0x7E)
			{
				flag ++;
				if(flag % 2 == 0)	//帧尾 0x7E  正常解析  完整的一包
				{
					ptrDeal[offset++] = 0x7E;
					sumErr = RecvPackAnalysis(ptrDeal,1);//校验接收数据帧的CheckSum
					if(sumErr != 0)						 //校验失败
					{
						Mem_free((void **)&ptrDeal);
						continue;
					}//处理所有接收到的数据
					ptrFrame = (FrameStruct *)ptrDeal;
					switch(ptrFrame->serverType)
					{	
						case 0x0A:			// 通知版本信息 旧版本还是新版本
							ptrtosend = Mem_malloc(50);
							ptrtosend[0] = sizeof(vciVer);
							ptrtosend[1] = 0x00;
							ptrtosend[2] = ptrDeal[13] + 0x40;
							memcpy(&ptrtosend[3],vciVer,sizeof(vciVer));
							DateToSend(ptrtosend,sizeof(vciVer)+2,0x02,0x0A);
							Mem_free((void **)&ptrtosend);
							break;
						case 0x01:			// 原 VCI CCP 上传
							break;
						case 0x0C:			// VCI 过安全算法
							RcvSeed_CatchMask(ptrFrame);
							break;
					
						case 0x0F:			// 便携王
							varOperaRun.sysMode = BXW_MODE;	//	便携王模式		
							MotorDeal(ptrDeal);
							break;
							
						case 0x09:			// 诊断能手
							varOperaRun.sysMode = VCI_MODE;	//诊断模式
							FrameDeal(ptrFrame);			// VCI 诊断指令处理
							break;

						case 0x0E:			// OTA 升级
							varOperaRun.sysMode = IDLE_MODE;//空模式
							OTA_Updata(&ptrDeal[11]);
							break;
						case 0x7F:			//工装 CAN、K、485、232 通讯自动化检测
							varOperaRun.sysMode = TEST_PORT_SEND_MODE;//通讯检测模式
							ComTest();

							break;
						default:	
							break;
					}
					Mem_free((void **)&ptrDeal);//处理完数据要释放内存块
				}else					//帧头 0x7E
				{
					offset = 0;
					ptrDeal = Mem_malloc(frameLen + 2 - i);
					if(ptrDeal == NULL)//内存不够用了
					{
						ptrDeal = NULL;
						break;
					}
					ptrDeal[offset++] = 0x7E;
				}
			}
			else if((ptrBTRecv[i] == 0x7D) && (ptrBTRecv[i+1] == 0x5E)) //复原 0x7E
			{
				ptrDeal[offset++] = 0x7E;
				i++;
			}else if((ptrBTRecv[i] == 0x7D) && (ptrBTRecv[i+1] == 0x5D))//复原 0x7D
			{
				ptrDeal[offset++] = 0x7D;
				i++;
			}
			else if(flag % 2 == 1)//正常的数据 已经有了帧头
			{
				ptrDeal[offset++] = ptrBTRecv[i];
			}
		}
		Mem_free((void **)&ptrDeal);	//释放
		Mem_free((void **)&ptrBTRecv);
	}
}
extern const u8 atCIPSEND[];
extern OS_EVENT * WIFICmdQ;				//接收到WiFi链接信息
extern u8 WIFISendDatas(u8* s,u16 length);
extern OS_EVENT * USBSendQ;	
//蓝牙发送数据
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
	
	if(datLen % 2 == 1)				//数据区 奇数补齐
		datLen ++;
		
	frameLen = datLen+10;
	ptrData = Mem_malloc(frameLen+4);
	ptrOK   = Mem_malloc(frameLen+18);
	if(ptrData == NULL || ptrOK == NULL)
		return;
	ptrData[offset++] = 0x7E;
	ptrData[offset++] = (uint8_t)((frameLen>>0) & 0x00FF);
	ptrData[offset++] = (uint8_t)((frameLen>>8) & 0x00FF);
	ptrData[offset++] = 0x00;		//报文计数
	ptrData[offset++] = 0x01;		//
	ptrData[offset++] = frameType;	//报文类型
	ptrData[offset++] = serverType;	//服务类型
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	ptrData[offset++] = 0x00;
	
	memcpy(&ptrData[offset],ptrToSend,datLen);
	RecvPackAnalysis(ptrData,0);
	offset = 0;
	ptrOK[offset++] = 0x7E;
	//对要发送的数据进行  PPP 协议处理 0x7E-0x7D 0x5E    0x7D-0x7D  0x5D
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
	//USB模式发送
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
	else if(varOperaRun.wirelessMode == BT_MODE)	//通过蓝牙发送数据
		BTSendDatas(ptrOK,offset);
	else if(varOperaRun.wirelessMode == WIFI_MODE)//通过WIFI发送数据
	{
		//合成预发送字符串
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
	//释放申请的内存块
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
	
	frameLen = ptrFrame[2];				//整帧数据的长度
	frameLen = (frameLen<<8) + ptrFrame[1];
	frameLen -= 10;						//数据区长度
	
	offset 	 = 11;
	for(i = 0;i < 11;i++)				//最多一次性处理11包
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
extern TIMEOUTPUT_pwm pwmOC[8];			//8路大占空比大频率发生器
extern TIMEOUTPUT_pwm lowPWMOC[8];		//可产生小占空比
void BXW_StartUp(STARTUP *ptr);
void BXW_CANSafe(uint8_t * ptrSafe,uint8_t safeCH);
void BXWCmdDeal(uint8_t *ptrdata)
{
	u8 i;
	switch(*ptrdata)
	{
		case 0x01:	BXW_StartUp((STARTUP *)ptrdata);break;				//供电选择、通讯模式、终端电阻
		
		//9路PWM波形控制
		case 0x02:	case 0x03:	case 0x04:	case 0x05:	case 0x06:	
		case 0x07:	case 0x08:	case 0x09:	
		case 0x0A:	BXW_PWMDataDeal((PWM_Data *)ptrdata,(*ptrdata) - 2);break;
		
		//直流电机 DCM 控制
		case 0x0B:	case 0x0C:	
		case 0x0D:	BXW_DCMDataDeal((DCM_Data *)ptrdata,(*ptrdata) - 11);break;
		
		//步进电机控制
		case 0x0F:	//正确接线
					//错误接线
		case 0x11:  BXW_DCMDataDeal((DCM_Data *)ptrdata,3);break;
		
		// CAN 泵控制
		case 0x0E:	BXW_CANdataDeal((BXWCAN_Data *)ptrdata); break;
		case 0x12:	
		case 0x13:	
			BXW_CANSafe(ptrdata,(*ptrdata)-0x11);
			break;
		//关闭所有 PWM 输出
		case 0x10:	
			for(i = 0;i < 8;i++)
			{
				pwmOC[i].isOPEN = CLOSE;
				lowPWMOC[i].isOPEN = CLOSE;
				varBXWRun.dutyCyclePWM[i] = 0;//所有占空比清零
			}
			break;
		case 0x1F:
			BXW_StartUp((STARTUP *)ptrdata);//VCI 模式下 CAN 切换
			break;
		default:break;
	}
}
extern BXWCAN_Data canBXWToSend;
extern OS_EVENT * BXWFlowSem;				//便携王数据流发送信号量
//全能王后处理部分，CAN2数据发送
void BXW_CANdataDeal(BXWCAN_Data *ptr)
{
	uint8_t err;
	u32 canFlowID;
	static u32 canbaud = 1000;
	static u32 canBaud = 0;
	static uint32_t mask = 10;		//过滤CANID 的 掩码mask
	
	//赋值可以用memcpy进行优化操作
	canBXWToSend.sendId = ptr->sendId;		//发送ID
	canBXWToSend.receId = ptr->receId;		//接收ID
	canBXWToSend.mask   = ptr->mask;		//掩码
	canBXWToSend.baud   = ptr->baud;		//波特率
	canBXWToSend.std_exd = ptr->std_exd;	//帧类型  0-标准帧   1-扩展帧
	memcpy(canBXWToSend.data,ptr->data,8);	//要发送的数据
	switch(canBXWToSend.baud)
	{
		case 1:	canbaud = 250000;break;
		case 2:	canbaud = 500000;break;
		case 3:	canbaud = 1000000;break;
		default:break;
	}
	if(canBXWToSend.std_exd == 0)//标准帧、扩展帧
		canBXWToSend.canType = CAN_ID_STD;
	else 
		canBXWToSend.canType = CAN_ID_EXT;
	if((canbaud != canBaud)||(mask != canBXWToSend.mask))
	{
		CAN2Config(canBXWToSend.receId,canBXWToSend.canType,canBXWToSend.mask,canbaud);	//CAN2 配置
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
extern OS_EVENT * BXWSafe;				//便携王后处理安全算法
//艾可蓝安全算法
extern ulong CalculateAikelanECU(uword EngineModel, ubyte *Seed, ubyte *Key);
const u8 kaiLongSafe[8] = {0x06,0x27,0x1C,0xD0,0x19,0x72,0xA9,0xAA};
// safeCH  0 - 正常数据交互     1 - 凯龙  2 - 艾可蓝
void BXW_CANSafe(uint8_t * ptrSafe,uint8_t safeCH)//凯龙、艾可蓝后处理过安全算法
{
	uint8_t  err;
	OSTimeDlyHMSM(0,0,0,20);
	memcpy((uint8_t *)&canBXWToSend.receId,&ptrSafe[5],8);// 获取 CANID 
	memcpy(canBXWToSend.data,&ptrSafe[9],8);   			  // 获取发送的内容
	
	canBXWToSend.safeCH = safeCH;
	Can2_SendData(canBXWToSend.sendId,canBXWToSend.canType,canBXWToSend.data);		//
	OSSemPend(BXWSafe,20,&err);//等待 ECU 回复 seed
	if(err == OS_ERR_NONE)
	{
		switch(safeCH)
		{
			case 0:
				break;
			case 1://凯龙后处理 安全算法
				memcpy(canBXWToSend.data,kaiLongSafe,8);
				canBXWToSend.safeCH = 0;
				Can2_SendData(canBXWToSend.sendId,canBXWToSend.canType,canBXWToSend.data);//发送Key
				break;
			case 2:
				canBXWToSend.data[0] = 0x06;
				canBXWToSend.data[1] = 0x27;
				canBXWToSend.data[2] = 0x0A;
				CalculateAikelanECU(0,canBXWToSend.safeSeed,&canBXWToSend.data[3]);	//计算key
				canBXWToSend.safeCH = 0;
				Can2_SendData(canBXWToSend.sendId,canBXWToSend.canType,canBXWToSend.data);//发送Key
				break;
			default:break;
		}
	}
}
//全能王后处理程序启动初始化，电源选择、 CAN 路选择、匹配电阻选择等
void BXW_StartUp(STARTUP *ptr)
{	
	uint16_t dat595 = 0x0000;
	uint8_t canChannel = 0;
	varOperaRun.startUp.powCheck = ptr->powCheck;		//电源选择
	varOperaRun.startUp.commMode = ptr->commMode;		//通讯模式
	varOperaRun.startUp.terminalRes = ptr->terminalRes;	//终端电阻
	
	switch(varOperaRun.startUp.powCheck)
	{
		//0 1 - 24V     X 0 - 12V    1 1-不供电
		case 0:	//0 V 
			GPIO_SetBits(GPIOC,GPIO_Pin_14);	//Pwr_24
			GPIO_SetBits(GPIOC,GPIO_Pin_13);	//Pwr_Sw
//			varOperaRun.ledFlag[0] = LEDOFF;	//电源灯灭
			break;
		case 1:	//24V
			GPIO_ResetBits(GPIOC,GPIO_Pin_14);	//Pwr_24
			GPIO_SetBits(GPIOC,GPIO_Pin_13);	//Pwr_Sw
//			varOperaRun.ledFlag[0] = LEDON;		//电源灯亮
			break;
		case 2:	//12V
			GPIO_SetBits(GPIOC,GPIO_Pin_14);	//Pwr_24
			GPIO_ResetBits(GPIOC,GPIO_Pin_13);	//Pwr_Sw
//			varOperaRun.ledFlag[0] = LEDON;		//电源灯亮
			break;
		default:break;
	}
	switch(varOperaRun.startUp.commMode)
	{
		case 3://CAN1                 
			dat595 += (1<<12);//打开  can2  选择CAN2A  OBD-6/14   DB15-1/15
			dat595 += (1<<13); 
			canChannel = 1;   
			canVCIData.canCH = 0;		
			break;   
		case 4://CAN2
			dat595 += (1<<11);//打开  can1  默认CAN1A  OBD-3/11   DB15-2/14
			canChannel = 2;    
			canVCIData.canCH = 1;				
			break;
		case 5://CAN3                 
			dat595 += (1<<12);//打开  can2  默认CAN2B  OBD-1/9   DB15-3/11
			canChannel = 3;
			canVCIData.canCH = 2;	
			break;                   
		case 6://CAN4                 
			dat595 += (1<<11);//打开  can1  选择CAN1B  OBD-11/12   DB15-6/10
			dat595 += (1<<10);        
			canChannel = 4;    
			canVCIData.canCH = 3;				
			break;                    
		default:break;
	}
	if(canChannel != 0)
		varOperaRun.isVCIchangeCANTrue = 0;//VCI 通道切换无效
	else
		varOperaRun.isVCIchangeCANTrue = 1;//VCI 通道切换有效
		
	if(varOperaRun.startUp.terminalRes == 1)	//120欧姆
	{
		if(canChannel == 2||canChannel == 4)	//CAN1A  B 
			dat595 += (1<<8);
		else if(canChannel == 1||canChannel == 3)//CAN2A  B  120欧姆电阻
			dat595 += (1<<15);
	}
	else if(varOperaRun.startUp.terminalRes == 2)//60欧姆
	{
		if(canChannel == 2||canChannel == 4)	//CAN1A  B   60欧姆电阻
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
	


























