#include "includes.h"


u8 updateBuff[2048];
extern void SbootParaToFlash(ROM_FLASH_PARAM* parameter);
ROM_FLASH_PARAM newSoftInfo;
void OTA_Updata(uint8_t *ptrData)
{
	uint16_t cmdId;
	uint16_t datLength = 0;
	uint16_t i = 0;
	uint8_t  sid;						//ƽ�巢�͹�����sid,ԭ�����ؼ���
	uint8_t *ptrToSend;
							//���ڼ���У��� CheckSum ��ָ��
	static uint16_t currentNum = 0;		//������һ�������
	static uint32_t fileCheckzSum = 0;	//�ļ�CRCУ��
	static uint32_t flashAddr  = 0;		//��ַ��Ϣ��д2K������0x800,��Flashһ��д2K�ֽ�
	static uint16_t frameIndex = 0;		//Ҫ�����֡����
	
	datLength = ptrData[1];				//��С������ĳ���
	datLength = (datLength<<8) + ptrData[0];
	datLength -= 3;						//����Э�飬��ȥ1�ֽڵ�sid 2�ֽڰ���ŵõ���ʵ���ݳ���
	
	sid		  = ptrData[2];	
	
	cmdId     = ptrData[4];				//�����
	cmdId     = (cmdId << 8) + ptrData[3];
	
	if(cmdId == 0xFFFF)					//����Ҫ�����Ĵ�����Ϣ
	{
		newSoftInfo.isUpdate = 1;
		newSoftInfo.hardVer  = updatDataRam.hardVer;
		
		newSoftInfo.softVer = ptrData[6];			//����汾
		newSoftInfo.softVer = (newSoftInfo.softVer<<8) + ptrData[5];
		
		newSoftInfo.newSoftCheckSum = ptrData[8];	//У���
		newSoftInfo.newSoftCheckSum = (newSoftInfo.newSoftCheckSum<<8) + ptrData[7];
		
		newSoftInfo.frameCnt = ptrData[10];			//����
		newSoftInfo.frameCnt = (newSoftInfo.frameCnt<<8) + ptrData[9];
		
		newSoftInfo.fileSize = ptrData[14];			//�ļ���С
		newSoftInfo.fileSize = (newSoftInfo.fileSize<<8) + ptrData[13];
		newSoftInfo.fileSize = (newSoftInfo.fileSize<<8) + ptrData[12];
		newSoftInfo.fileSize = (newSoftInfo.fileSize<<8) + ptrData[11];
		
		newSoftInfo.newPageCnt = (newSoftInfo.fileSize/2048) + 1;//��Ҫ�������µ�ҳ��
		/***�������������ʼ��***/
		currentNum		= 0x0001;	//��һ������
		fileCheckzSum	= 0;		//CheckSum
		flashAddr		= 0;		//ҳ��ַ
		frameIndex		= 0;		//2K������������
		/****�����һ������****/
		ptrToSend	= Mem_malloc(10);
		ptrToSend[0] = 0x08;ptrToSend[1] = 0x00;ptrToSend[2] = sid;
		ptrToSend[3] = (uint8_t)currentNum&0x00FF;
		ptrToSend[4] = (currentNum)>>8;
		ptrToSend[5] = updatDataRam.softVer&0x00FF;//���ͱ�������汾
		ptrToSend[6] = (updatDataRam.softVer)>>8;
		ptrToSend[7] = updatDataRam.hardVer&0x00FF;//���ͱ���Ӳ���汾
		ptrToSend[8] = updatDataRam.hardVer>>8;
		DateToSend(ptrToSend,10,0,0x0E);			//�����һ������
		Mem_free((void **)&ptrToSend);
	}else if(cmdId > 0&&cmdId < 0xF000)				//��ʼ���ճ���
	{
		ptrToSend = Mem_malloc(10);

		if(cmdId != currentNum)//�ϵ�����
		{
			ptrToSend[0] = 0x04;ptrToSend[1] = 0x00;ptrToSend[2] = sid;
			ptrToSend[3] = (uint8_t)currentNum&0x00FF;
			ptrToSend[4] = (currentNum)>>8;
			DateToSend(ptrToSend,6,0,0x0E);
			Mem_free((void **)&ptrToSend);
			return;
		}
		if(cmdId >= newSoftInfo.frameCnt)	//�����������
		{
			memcpy(&updateBuff[frameIndex],&ptrData[5],datLength);
			frameIndex += datLength;		//
			
			SoftErasePage(flashAddr);
			SoftProgramUpdate(flashAddr,updateBuff,2048);
			
			for(i = 0;i<2048;i++)			//����У���
				fileCheckzSum += updateBuff[i];
				
			fileCheckzSum &= 0x0000FFFF;
			if(fileCheckzSum == newSoftInfo.newSoftCheckSum)//�ж��ļ�CRCУ���Ƿ�ͨ��
			{
				memcpy(&updatDataRam,&newSoftInfo,sizeof(ROM_FLASH_PARAM));
			
				SbootParaToFlash(&updatDataRam);			//������д�������	0x8007800
				ptrToSend[0] = 0x04;ptrToSend[1] = 0x00;ptrToSend[2] = sid;
				ptrToSend[3] = 0xFF;
				ptrToSend[4] = 0xFF;
				DateToSend(ptrToSend,6,0,0x0E);
				Mem_free((void **)&ptrToSend);
				OSTimeDlyHMSM(0,0,0,500);
				
				//todo:���������д��Flash
				OSTimeDlyHMSM(0,0,0,500);		//��ʱ500���봦��ȴ����һ����֤���ݷ������
				//todo:����
				__disable_fault_irq();          //����
				NVIC_SystemReset();
				return;
			}
			/***�ļ�CRCУ��ʧ��***/
			ptrToSend[0] = 0x04;ptrToSend[1] = 0x00;ptrToSend[2] = sid;
			ptrToSend[3] = 0xAA;
			ptrToSend[4] = 0xBB;
			DateToSend(ptrToSend,6,0,0x0E);
			Mem_free((void **)&ptrToSend);
			return;
		}
		memcpy(&updateBuff[frameIndex],&ptrData[5],datLength);
		frameIndex += datLength;		//
		if(frameIndex >= 2048)			//���յ�������2K�ֽڵ�����
		{
			SoftErasePage(flashAddr);
			SoftProgramUpdate(flashAddr,updateBuff,2048);
			
			for(i = 0;i<2048;i++)
				fileCheckzSum += updateBuff[i];
				
			memset(updateBuff,0,2048);
			flashAddr += 0x800;
			frameIndex = 0;
		}
		currentNum ++;					//������һ������
		ptrToSend[0] = 0x04;ptrToSend[1] = 0x00;ptrToSend[2] = sid;
		ptrToSend[3] = (uint8_t)currentNum&0x00FF;
		ptrToSend[4] = (currentNum)>>8;
		DateToSend(ptrToSend,6,0,0x0E);
		Mem_free((void **)&ptrToSend);					
	}
}








