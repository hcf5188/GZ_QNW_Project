#include "includes.h"



//新程序的数据更新
void SoftProgramUpdate(uint32_t wAddr,uint8_t* ptrBuff,uint16_t datLength)
{
	OS_CPU_SR  cpu_sr = 0u;
	uint16_t i = 0;
	if(wAddr > 0x80000)
		return ;
	wAddr += NEWSOFT_ADDR;
	
	OS_ENTER_CRITICAL();//禁止中断
	{
		volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
		volatile uint32_t     w_addr      = wAddr;
		volatile uint32_t*    p_w_data    = (uint32_t*)ptrBuff;	//4字节指针
		volatile uint32_t     w_data      = (uint32_t)(*p_w_data);//取得相应数据

		for(i=0;(i<(datLength/4))&&(FLASHStatus == FLASH_COMPLETE);i++) 
		{
			FLASHStatus = FLASH_ProgramWord(w_addr, w_data);
			w_addr = w_addr + 4;
			p_w_data++;
			w_data=(uint32_t)(*p_w_data)  ;	
		}
	}
	OS_EXIT_CRITICAL(); //打开中断
}

void SoftErasePage(uint32_t addr)
{
	OS_CPU_SR  cpu_sr = 0u;
	OS_ENTER_CRITICAL();		//禁止中断
	{
		volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
		
		if(addr<0x80000)		//偏移地址加上起始地址
			addr += NEWSOFT_ADDR;
		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

		if(FLASHStatus==FLASH_COMPLETE)
			FLASHStatus = FLASH_ErasePage(addr);
	}
	OS_EXIT_CRITICAL(); //打开中断
}

void SbootParaToFlash(ROM_FLASH_PARAM* parameter)
{
	OS_CPU_SR  cpu_sr = 0u;
	uint16_t i = 0;
	OS_ENTER_CRITICAL();//禁止中断
	{
		volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
		volatile uint32_t w_addr = 0x00;
		volatile uint32_t w_data=0x00;
		volatile uint32_t* p_w_data=0x00;
		//int i=0;
		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

		//擦去相应页
		if(FLASHStatus==FLASH_COMPLETE)
		FLASHStatus = FLASH_ErasePage(UPDATAPARA_ADDR);
		//写入
		if(FLASHStatus==FLASH_COMPLETE)
		{
			w_addr = (uint32_t)(UPDATAPARA_ADDR);	
					
			p_w_data=(uint32_t*)(parameter);

			w_data=(uint32_t)(*p_w_data);//取得相应数据

			for(i=0;(i<(sizeof(ROM_FLASH_PARAM)/4 + 1))&&(FLASHStatus == FLASH_COMPLETE);i++) //
			{
				FLASHStatus = FLASH_ProgramWord(w_addr, w_data);

				if(FLASHStatus!=FLASH_COMPLETE)
					break;
		 		w_addr = w_addr + 4;
		 		p_w_data++;
		 		w_data=(uint32_t)(*p_w_data) ;	//取得数据
			}
		}
	}
	OS_EXIT_CRITICAL(); //打开中断
}

//新程序的数据更新
void Save2KDataToFlash(uint8_t* ptrBuff,uint32_t flashAddr,uint16_t datLength)
{
	OS_CPU_SR  cpu_sr = 0u;
	uint16_t i = 0;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(flashAddr);
	
	OS_ENTER_CRITICAL();//禁止中断
	{
		volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
		volatile uint32_t     w_addr      = flashAddr;
		volatile uint32_t*    p_w_data    = (uint32_t*)ptrBuff;	//4字节指针
		volatile uint32_t     w_data      = (uint32_t)(*p_w_data);//取得相应数据

		for(i=0;(i<(datLength/4))&&(FLASHStatus == FLASH_COMPLETE);i++) 
		{
			FLASHStatus = FLASH_ProgramWord(w_addr, w_data);
			w_addr = w_addr + 4;
			p_w_data++;
			w_data = (uint32_t)(*p_w_data)  ;	
		}
	}
	OS_EXIT_CRITICAL(); //打开中断
}
//程序启动的时候，从Flash中读取全局参数用




