#ifndef __DEAL_H__
#define __DEAL_H__


#include "includes.h"






#include "sys.h" 

void delay_init(void);
//void delay_ms(u16 nms);
//void delay_us(u32 nus);

extern uint8_t RecvPackAnalysis(u8 *ptrDataToDeal,u8 mode);
extern void OTA_Updata(uint8_t *ptrData);					// OTA  Éý¼¶
void SoftProgramUpdate(uint32_t wAddr,uint8_t* ptrBuff,uint16_t datLength);
void SoftErasePage(uint32_t addr);
//void SbootParaToFlash(ROM_FLASH_PARAM* parameter);
void Save2KDataToFlash(uint8_t* ptrBuff,uint32_t flashAddr,uint16_t datLength);


#endif 






