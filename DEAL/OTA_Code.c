#include "includes.h"


u8 updateBuff[2048];
extern void SbootParaToFlash(ROM_FLASH_PARAM* parameter);
ROM_FLASH_PARAM newSoftInfo;
void OTA_Updata(uint8_t *ptrData)
{
	uint16_t cmdId;
	uint16_t datLength = 0;
	uint16_t i = 0;
	uint8_t  sid;						//平板发送过来的sid,原样返回即可
	uint8_t *ptrToSend;
							//用于计算校验和 CheckSum 的指针
	static uint16_t currentNum = 0;		//发送下一个请求包
	static uint32_t fileCheckzSum = 0;	//文件CRC校验
	static uint32_t flashAddr  = 0;		//地址信息，写2K便自增0x800,向Flash一次写2K字节
	static uint16_t frameIndex = 0;		//要保存的帧索引
	
	datLength = ptrData[1];				//此小包代码的长度
	datLength = (datLength<<8) + ptrData[0];
	datLength -= 3;						//根据协议，减去1字节的sid 2字节包序号得到真实数据长度
	
	sid		  = ptrData[2];	
	
	cmdId     = ptrData[4];				//包序号
	cmdId     = (cmdId << 8) + ptrData[3];
	
	if(cmdId == 0xFFFF)					//处理要升级的代码信息
	{
		newSoftInfo.isUpdate = 1;
		newSoftInfo.hardVer  = updatDataRam.hardVer;
		
		newSoftInfo.softVer = ptrData[6];			//软件版本
		newSoftInfo.softVer = (newSoftInfo.softVer<<8) + ptrData[5];
		
		newSoftInfo.newSoftCheckSum = ptrData[8];	//校验和
		newSoftInfo.newSoftCheckSum = (newSoftInfo.newSoftCheckSum<<8) + ptrData[7];
		
		newSoftInfo.frameCnt = ptrData[10];			//包数
		newSoftInfo.frameCnt = (newSoftInfo.frameCnt<<8) + ptrData[9];
		
		newSoftInfo.fileSize = ptrData[14];			//文件大小
		newSoftInfo.fileSize = (newSoftInfo.fileSize<<8) + ptrData[13];
		newSoftInfo.fileSize = (newSoftInfo.fileSize<<8) + ptrData[12];
		newSoftInfo.fileSize = (newSoftInfo.fileSize<<8) + ptrData[11];
		
		newSoftInfo.newPageCnt = (newSoftInfo.fileSize/2048) + 1;//需要擦除更新的页数
		/***相关升级参数初始化***/
		currentNum		= 0x0001;	//第一包数据
		fileCheckzSum	= 0;		//CheckSum
		flashAddr		= 0;		//页地址
		frameIndex		= 0;		//2K缓冲区的索引
		/****请求第一包数据****/
		ptrToSend	= Mem_malloc(10);
		ptrToSend[0] = 0x08;ptrToSend[1] = 0x00;ptrToSend[2] = sid;
		ptrToSend[3] = (uint8_t)currentNum&0x00FF;
		ptrToSend[4] = (currentNum)>>8;
		ptrToSend[5] = updatDataRam.softVer&0x00FF;//发送本机软件版本
		ptrToSend[6] = (updatDataRam.softVer)>>8;
		ptrToSend[7] = updatDataRam.hardVer&0x00FF;//发送本机硬件版本
		ptrToSend[8] = updatDataRam.hardVer>>8;
		DateToSend(ptrToSend,10,0,0x0E);			//请求第一包数据
		Mem_free((void **)&ptrToSend);
	}else if(cmdId > 0&&cmdId < 0xF000)				//开始接收程序
	{
		ptrToSend = Mem_malloc(10);

		if(cmdId != currentNum)//断点续传
		{
			ptrToSend[0] = 0x04;ptrToSend[1] = 0x00;ptrToSend[2] = sid;
			ptrToSend[3] = (uint8_t)currentNum&0x00FF;
			ptrToSend[4] = (currentNum)>>8;
			DateToSend(ptrToSend,6,0,0x0E);
			Mem_free((void **)&ptrToSend);
			return;
		}
		if(cmdId >= newSoftInfo.frameCnt)	//程序升级完成
		{
			memcpy(&updateBuff[frameIndex],&ptrData[5],datLength);
			frameIndex += datLength;		//
			
			SoftErasePage(flashAddr);
			SoftProgramUpdate(flashAddr,updateBuff,2048);
			
			for(i = 0;i<2048;i++)			//计算校验和
				fileCheckzSum += updateBuff[i];
				
			fileCheckzSum &= 0x0000FFFF;
			if(fileCheckzSum == newSoftInfo.newSoftCheckSum)//判断文件CRC校验是否通过
			{
				memcpy(&updatDataRam,&newSoftInfo,sizeof(ROM_FLASH_PARAM));
			
				SbootParaToFlash(&updatDataRam);			//将参数写入参数区	0x8007800
				ptrToSend[0] = 0x04;ptrToSend[1] = 0x00;ptrToSend[2] = sid;
				ptrToSend[3] = 0xFF;
				ptrToSend[4] = 0xFF;
				DateToSend(ptrToSend,6,0,0x0E);
				Mem_free((void **)&ptrToSend);
				OSTimeDlyHMSM(0,0,0,500);
				
				//todo:将程序参数写入Flash
				OSTimeDlyHMSM(0,0,0,500);		//延时500毫秒处理等待最后一包验证数据发送完成
				//todo:重启
				__disable_fault_irq();          //重启
				NVIC_SystemReset();
				return;
			}
			/***文件CRC校验失败***/
			ptrToSend[0] = 0x04;ptrToSend[1] = 0x00;ptrToSend[2] = sid;
			ptrToSend[3] = 0xAA;
			ptrToSend[4] = 0xBB;
			DateToSend(ptrToSend,6,0,0x0E);
			Mem_free((void **)&ptrToSend);
			return;
		}
		memcpy(&updateBuff[frameIndex],&ptrData[5],datLength);
		frameIndex += datLength;		//
		if(frameIndex >= 2048)			//接收到完整的2K字节的数据
		{
			SoftErasePage(flashAddr);
			SoftProgramUpdate(flashAddr,updateBuff,2048);
			
			for(i = 0;i<2048;i++)
				fileCheckzSum += updateBuff[i];
				
			memset(updateBuff,0,2048);
			flashAddr += 0x800;
			frameIndex = 0;
		}
		currentNum ++;					//请求下一包数据
		ptrToSend[0] = 0x04;ptrToSend[1] = 0x00;ptrToSend[2] = sid;
		ptrToSend[3] = (uint8_t)currentNum&0x00FF;
		ptrToSend[4] = (currentNum)>>8;
		DateToSend(ptrToSend,6,0,0x0E);
		Mem_free((void **)&ptrToSend);					
	}
}








