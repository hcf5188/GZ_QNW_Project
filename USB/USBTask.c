#include  "usbd_hid_core.h"
#include  "usbd_usr.h"
#include  "usbd_desc.h"
#include "includes.h"
#include "apptask.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;
extern uint8_t USBRxbuff[64];


extern OS_EVENT * USBSendQ;				//USB发送数据消息队列
void USBSendTask(void *pdata)
{
	uint8_t err;
	uint8_t *ptrUSBSend;

	while(1)
	{
		ptrUSBSend = OSQPend(USBSendQ,0,&err);

		USBD_HID_SendReport (&USB_OTG_dev,ptrUSBSend,64);
		Mem_free((void **)&ptrUSBSend);
	}
}

extern OS_EVENT * USBRecvQ;				//USB接收数据消息队列
extern OS_EVENT * MessageRecvQ;			//平板 接收消息队列的指针
void USBRecvTask(void *pdata)
{
	u8 err = 0;
	uint8_t * ptrUSBRecBuf;
	uint8_t * ptrDeal = NULL;
	uint16_t index = 0;
	uint16_t frameDevNums = 0;//总包数
	uint16_t frameDevNum  = 0;//分包数
	
	
	//USB 初始化
	USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc, &USBD_HID_cb,&USR_cb);
	
	while(1)
	{	
		ptrUSBRecBuf = OSQPend(USBRecvQ,5000,&err);
		if(err != OS_ERR_NONE)//USB 接收超时
		{
			index = 0;
			Mem_free((void **)&ptrDeal);
			continue;
		}
		//系统处在USB通讯模式下
		varOperaRun.wirelessMode = USB_MODE;
		
		if(ptrUSBRecBuf[0] >= 15)//数据量太大
		{
			Mem_free((void **)&ptrUSBRecBuf);
			continue;
		}	
		frameDevNums = ptrUSBRecBuf[0];
		frameDevNum = ptrUSBRecBuf[1];
		if(frameDevNum == 1)
		{
			index = 0;
			ptrDeal = Mem_malloc(frameDevNums*62+2);
			ptrDeal[0] = (frameDevNums*62)%256;
			ptrDeal[1] = ((frameDevNums*62)>>8)%256;
		}
		
		memcpy(&ptrDeal[index*62+2],&ptrUSBRecBuf[2],62);
		index ++;
		//
		if(frameDevNums <= frameDevNum)
		{
			if(OSQPost(MessageRecvQ,ptrDeal)!=OS_ERR_NONE)
				Mem_free((void **)&ptrDeal);
		}

		Mem_free((void **)&ptrUSBRecBuf);
	}
}







