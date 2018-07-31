
#include "includes.h"
extern OS_EVENT * CANRecvQ;				//CAN 接收消息队列的指针
extern BXWAllData varBXWRun;	
 u8 manyPack[8] = {0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00};//接收多包
extern pSTORE     BXW_CAN;		//便携王接收 CAN 泵信息
extern OS_EVENT * BXWFlowSem;				//便携王数据流发送信号量
extern BXWCAN_Data canBXWToSend;
extern OS_EVENT * canRCV30;				// VCI 接收到0x30指令 信号量
extern OS_EVENT * BXWSafe;				//便携王后处理安全算法
extern OS_EVENT * testMbox;				//通讯检测 消息邮箱
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
			//泵都是扩展帧
			bxwCandatLen = Store_Getlength(BXW_CAN);
			if(bxwCandatLen>300)//已存的数据量比较大，放弃此包
			{
				Mem_free((void **)&RxMessage);
				continue;
			}//判断是否是安全算法，截取seed
			if(RxMessage->ExtId == canBXWToSend.receId)
			{
				switch(canBXWToSend.safeCH)
				{
					case 0:break;
					case 1://凯龙安全算法是固定的
						OSSemPost(BXWSafe);
						continue;
					case 2://艾可蓝安全算法
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
				OSSemPost(BXWFlowSem);//数据流 CANID
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
			if((RxMessage->Data[0] == 0x10)&&(varOperaRun.manyPackFlag == OPEN))//sid为0x18的指令，请求多包
			{
				if(canVCIData.canCH == 0||canVCIData.canCH == 1)//CAN1 2通道
					Can1_SendData(varOperaRun.sendCANID,canVCIData.std_exd,(u8 *)manyPack);//发送
				else if(canVCIData.canCH == 2||canVCIData.canCH == 3)//CAN3 4通道
					Can2_SendData(varOperaRun.sendCANID,canVCIData.std_exd,(u8 *)manyPack);//发送
			}
			else if((RxMessage->Data[0] == 0x30)&&(varOperaRun.manyPackFlag == OPEN))
			{
				OSSemPost(canRCV30);//通知 sid - 0x17 接收到ECU请求多包的0x30指令 
				Mem_free((void **)&ptrCANDat);
				Mem_free((void **)&RxMessage);
				continue;
			}
			//todo:30指令不需要上报
			DateToSend(ptrCANDat,15,0x02,0x09);
			Mem_free((void **)&ptrCANDat);
			Mem_free((void **)&RxMessage);
		}
		else if(varOperaRun.sysMode == TEST_PORT_SEND_MODE)//通讯测试 主测 模式
		{
			if(OSMboxPost(testMbox,RxMessage) != OS_ERR_NONE)
				Mem_free((void **)&RxMessage);
		}
		else if(varOperaRun.sysMode == TEST_PORT_RECV_MODE)//通讯测试 被测 模式
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
//K线接收任务处理，主要是上报接收到的信息
void KLineTask(void *pdata)
{
	uint8_t  err;
	uint8_t  checkIsOK = 0;	
	uint8_t  *ptrKDat;
	uint16_t datLen;

	while(1)
	{
		ptrKDat = OSQPend(KLineRecvQ,0,&err);
		
		memcpy(&datLen,ptrKDat,2);//K线返回的数据长度
		
		checkIsOK = K_CheckSum(&ptrKDat[3],(datLen - 2));//减去CS，减去sid
		if(checkIsOK != 0)//校验不成功
		{
			Mem_free((void **)&ptrKDat);
			continue;
		}
		//todo: 蓝牙  WiFi  USB
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
	if(checkK == ptrK[i])//校验成功
		return 0;
	return 1;
}
void CANChannelCheck(u8 ch,u8 res);	//CAN通道切换、匹配电阻
//诊断功能 CAN K 线数据解析回复
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
		case 0x03://todo: 设置针脚
			CANChannelCheck(0,0);//默认针脚,Debug时调试程序用
			switch(ptrData[14])
			{
				case 0x00://CAN 线自定义引脚
					if(ptrData[15] == 15)//CAN1AH 映射到  DB15    通道1
					{
						
					}else if(ptrData[15] == 3)//CAN1BH 映射到  DB3     通道2
					{
						
					}
					else if(ptrData[15] == 2)//CAN2AH 映射到 DB2     通道3
					{
						
					}
					else if(ptrData[15] == 6)//CAN2BH 映射到 DB6    通道4
					{
						
					}
					break;
				case 0x04://RS232引脚自定义             全能王不支持
				case 0x05://RS485/J1708引脚自定义		全能王不支持
				case 0x06:// K 线引脚自定义				全能王不支持
					break;
				default:break;
			}
			
			ptrData = Mem_malloc(20);	//回复全能王版本号，此包是为了兼容VCI才发送的数据
			ptrData[0] = 0x01;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;	//0x53
			DateToSend(ptrData,3,0x02,0x09);
			Mem_free((void **)&ptrData);
			break;
			
		case 0x08:	case 0x0F:			//todo:关闭所有通道  恢复上电配置
			varOperaRun.manyPackFlag = CLOSE;		
			varOperaRun.manyPackFlagDown = CLOSE;
//			memset(&canVCIData,0,sizeof(VCICAN_Data));//清空配置参数
//			K_Config(10400);				//波特率10400
//			varOperaRun.dat595 &= 0x0000FFFF;
//			LS595Deal(varOperaRun.dat595);	//关闭两路 CAN  以及匹配电阻
				
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
		case 0x10:	//设置 CAN 波特率4字节 +  1字节匹配电阻 todo:确定数字与之对应的电阻
			memcpy((u8*)&canVCIData.canBaud,&ptrData[14],5);//获得CAN波特率
//			CANChannelCheck(100,canVCIData.canRes);
//			CANChannelCheck(100,0);
//			//回复版本号
			ptrData = Mem_malloc(50);
			ptrData[0] = 0x01;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			DateToSend(ptrData,4,0x02,0x09);
			Mem_free((void **)&ptrData);
			
			break;
			
		case 0x20:						// K 线快速初始化
			varOperaRun.cmdIdK = cmdId;
			KLineFastInit(&ptrData[14],datLen-1);
			break;	
			
		case 0x22:	// K 线传输
			varOperaRun.cmdIdK = cmdId;
			KLineSendDatas(&ptrData[14],datLen-1);
			break;
		
		case 0x23:	//配置起始字节间的时间参数
			//todo:K初始化指令字节 时间参数
			if(varOperaRun.isVCIchangeCANTrue)//平板让配置才行
			{
				canVCIData.canCH = ptrData[15];	//通道选择
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
		case 0x14:	//普通 CAN 单帧、多帧
			varOperaRun.cmdIdCAN = cmdId;
			ptrCAN = (VCICAN_Data *)&ptrData[13];
			memcpy((u8*)&canVCIData.cmdId,ptrCAN,23);//获取2ID，MASK，Data
			
			if(canVCIData.receId < 0x80000000)//标准帧
			{
				canVCIData.std_exd = CAN_ID_STD;
				varOperaRun.receCANID = canVCIData.receId;
				varOperaRun.sendCANID = canVCIData.sendId;
			}
			else//扩展帧
			{
				canVCIData.std_exd = CAN_ID_EXT;
				varOperaRun.receCANID = canVCIData.receId - 0x80000000;
				if(canVCIData.sendId > 0x80000000)
					varOperaRun.sendCANID = canVCIData.sendId - 0x80000000;
				else//(兼容康明斯)
					varOperaRun.sendCANID = canVCIData.sendId;
			}
			if(canVCIData.canCH == 1||canVCIData.canCH == 3)//配置 CAN2 4通道
				CAN1Config(varOperaRun.receCANID,canVCIData.std_exd,canVCIData.mask,canVCIData.canBaud);//CAN1 配置
			else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//配置 CAN1 3通道
				CAN2Config(varOperaRun.receCANID,canVCIData.std_exd,canVCIData.mask,canVCIData.canBaud);//CAN1 配置
			
			memcpy((u8*)&canVCIData.data,ptrCAN->data,8);
			cmp = memcmp(manyPack,ptrCAN->data,8);
			if(cmp != 0)											//请求多包，若是 30 指令的话，直接忽视
			{
				if(canVCIData.canCH == 1||canVCIData.canCH == 3)	//CAN 2 4 通道
					Can1_SendData(varOperaRun.sendCANID,canVCIData.std_exd,canVCIData.data);//发送
				else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//CAN 1 3 通道
					Can2_SendData(varOperaRun.sendCANID,canVCIData.std_exd,canVCIData.data);//发送
			}
			offset = 0x0018;
			if(offset >= datLen)//单包的话，数据已经发送
				break;	
			else				//发送第一包之后的包
			{
				do{
					if(canVCIData.canCH == 1||canVCIData.canCH == 3)	 //CAN2 4通道
						Can1_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//发送
					else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//CAN1 3通道
						Can2_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//发送
					offset += 8;
					CANDelay(30);//延时是必要的，否则会丢失包，甚至跑飞程序
				}while(offset < datLen);
			}break;
		case 0x15://普通多帧
			break;
		case 0x17://多包，接收下位机发送的30指令
			varOperaRun.manyPackFlagDown = OPEN;
			varOperaRun.cmdIdCAN = cmdId;
			ptrCAN = (VCICAN_Data *)&ptrData[13];
			cmp = memcmp(ptrCAN,&canVCIData,13);//比较发送ID，接收ID，mask是否相同
			if(cmp != 0)
			{
				memcpy((u8*)&canVCIData.cmdId,ptrCAN,23);//获取2个CANID，MASK，Data
				if(canVCIData.receId < 0x80000000)//标准帧
				{
					canVCIData.std_exd = CAN_ID_STD;
					varOperaRun.receCANID = canVCIData.receId;
					varOperaRun.sendCANID = canVCIData.sendId;
				}
				else//扩展帧
				{
					canVCIData.std_exd = CAN_ID_EXT;
					varOperaRun.receCANID = canVCIData.receId - 0x80000000;
					if(canVCIData.sendId > 0x80000000)
						varOperaRun.sendCANID = canVCIData.sendId - 0x80000000;
					else//(兼容康明斯)
						varOperaRun.sendCANID = canVCIData.sendId;
				}
				if(canVCIData.canCH == 1||canVCIData.canCH == 3)//配置 CAN1 2通道
					CAN1Config(varOperaRun.receCANID,canVCIData.std_exd,canVCIData.mask,canVCIData.canBaud);//CAN1 配置
				else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//配置 CAN3 4通道
					CAN2Config(varOperaRun.receCANID,canVCIData.std_exd,canVCIData.mask,canVCIData.canBaud);//CAN1 配置
			}else{
				memcpy((u8*)&canVCIData.data,ptrCAN->data,8);
			}
			cmp = memcmp(manyPack,ptrCAN->data,8);
			if(cmp != 0)//请求多包 30 指令的话，直接忽视
			{
				if(canVCIData.canCH == 1||canVCIData.canCH == 3)	 //CAN2 4通道
					Can1_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//发送
				else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//CAN1 3通道
					Can2_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//发送
			}
			offset = 0x0018;
			if(offset >= datLen)//单包的话，数据已经发送
				break;	
			else		//发送第一包之后的包
			{
				OSSemPend(canRCV30,20,&err);//等待ECU回复0x30包
				if(err != OS_ERR_NONE)
					break;
				do{
					if(canVCIData.canCH == 1||canVCIData.canCH == 3)	 //CAN2 4通道
						Can1_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//发送
					else if(canVCIData.canCH == 0||canVCIData.canCH == 2)//CAN1 3通道
						Can2_SendData(varOperaRun.sendCANID,canVCIData.std_exd,&ptrData[offset+12]);//发送
					offset += 8;
					CANDelay(30);
				}while(offset < datLen);
			}break;
		case 0x18://接收下位机的多包，发送规定的30指令
				varOperaRun.manyPackFlag = OPEN;
				//复制将要发送的30指令，ptrData[14]目前不清楚其作用
				memcpy(manyPack,&ptrData[15],8);//缓存规定的0x30指令
			break;
		default: break;
	}
}
void CANChannelCheck(u8 ch,u8 res)//CAN通道选择、电阻匹配电阻
{
	uint16_t dat595 = 0x0000;
	switch(ch)
	{
		case 0x00://选择 CAN  1通道         CAN2A
			dat595 += (1<<12);//打开  can2  选择CAN2A
			dat595 += (1<<13);        
			dat595 += (1<<15);//120欧姆电阻
			
			break;
		case 0x01://选择 CAN  2通道			CAN1A
			dat595 += (1<<11);	//打开  can1A  默认CAN1A
			dat595 += (1<<8);	//120欧姆电阻
			break;
		case 0x02://选择 CAN  3通道			CAN2B
			dat595 += (1<<12);	//打开  can2  默认CAN2B	
			dat595 += (1<<15);	//120欧姆电阻
			break;
		case 0x03://选择 CAN  4通道			CAN1B
			dat595 += (1<<11);	//打开  can1B  选择CAN1B
			dat595 += (1<<10);	
			dat595 += (1<<8);	//120欧姆电阻//120欧姆电阻
			break;
		default:
			break;
	}
	
//	if(canVCIData.canCH == 2||canVCIData.canCH == 4)//CAN  2  4  通道
//	{
//		switch(res)
//		{
//			case 0x00://CAN  1  2 通道匹配 120 欧姆电阻 
//				varOperaRun.dat595 &= 0x3CFF;
//				varOperaRun.dat595 |= 0x0100;
//				
//				break;
//			case 0x01://CAN  1  2 通道匹配 60  欧姆电阻 
//				varOperaRun.dat595 &= 0x3CFF;
//				varOperaRun.dat595 |= 0x0300;
//				break;
//			case 0x02://CAN  1  2 通道不匹配电阻 
//				varOperaRun.dat595 &= 0x3CFF;
//				break;
//			default:
//				break;
//		}
//	}else if(canVCIData.canCH == 1||canVCIData.canCH == 3)//CAN 1  3  通道
//	{
//		switch(res)
//		{
//			case 0x00://CAN  3  4 通道匹配 120 欧姆电阻 
//				varOperaRun.dat595 &= 0x3CFF;
//				varOperaRun.dat595 |= 0x4000;
//				break;
//			case 0x01://CAN  3  4 通道匹配 60  欧姆电阻 
//				varOperaRun.dat595 &= 0x3CFF;
//				varOperaRun.dat595 |= 0xC000;
//				break;
//			case 0x02://CAN  3  4 通道不匹配电阻 
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

















