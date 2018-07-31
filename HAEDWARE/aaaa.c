#include "stm32f10x.h"
#include "stdio.h"
#include "usart_printf.h"
//#include "TouchScreen.h"
#include "filter.h"
//#include "sort.h"
#include "ADS1243.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "misc.h"
#include "systick.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_spi.h"

#include "usart_printf.h"
#include "main.h"
#include "gui_struct.h"
#include "key_board.h"
#include "touch_pad.h"
#include "kb_tp_msg.h"
#include "lcd.h"
#include "backlight.h"
#include "test_adc.h"
#include "adc_sample.h"


//#define ADS1243_SPI_ANALOG
//#define	TP_CLK_H 	GPIO_SetBits(GPIOB, GPIO_Pin_13)
//
//#define	TP_CLK_L 	GPIO_ResetBits(GPIOB, GPIO_Pin_13)
//#define	TP_OUT_H 	GPIO_SetBits(GPIOB, GPIO_Pin_15)
//#define	TP_OUT_L 	GPIO_ResetBits(GPIOB, GPIO_Pin_15)
//#define TP_IN 		GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)


/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*				SDIN   = PB15
*				SDOUT  = PB14
*				SCLK   = PB13
*				DRDY   = PB12
*				CS_ADC = PD11
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
void ADS1243_GPIO_Init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;


	SPI_InitTypeDef   	SPI_InitStructure;

	/* GPIO Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD  | RCC_APB2Periph_GPIOG,ENABLE);

	/* SPI2 Periph clock enable */
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

//	/* Configure SPI2 pins: SCK, MISO and MOSI */
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   	/* 复用推挽输出 */
//	GPIO_Init(GPIOB, &GPIO_InitStructure);

/*******模拟SPI******************/

	/*设置SPI2引脚: MISO*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GPIOB,&GPIO_InitStructure);	

	/*设置SPI2引脚: SCK, MOSI*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOB,&GPIO_InitStructure);
/***********************************************/

	/* Configure PB12 pin: DRDY pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		/* 浮空输入 */
	GPIO_Init(GPIOD, &GPIO_InitStructure);	

	/* Configure PD11 pin: CS_ADC pin */
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	/* 推挽输出 */
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOD, GPIO_Pin_11);	/* 将CS_ADC置为高,不选中ADC */	

/*****************************************************************************/
	/* Configure PD11 pin: CS_ADC pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	/* 推挽输出 */
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOG, GPIO_Pin_6);	
	GPIO_ResetBits(GPIOG, GPIO_Pin_7);
	GPIO_ResetBits(GPIOG, GPIO_Pin_15);	              //SELE1
	/* Configure SELE2.3 pin: CS_ADC pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	/* 推挽输出 */
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_8);	
	GPIO_SetBits(GPIOB, GPIO_Pin_9);
 	/* Configure SELE4-8 pin: CS_ADC pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	/* 推挽输出 */
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_SetBits(GPIOE, GPIO_Pin_2);
	GPIO_SetBits(GPIOE, GPIO_Pin_3);	
	GPIO_SetBits(GPIOE, GPIO_Pin_4);
	GPIO_SetBits(GPIOE, GPIO_Pin_5);	
	GPIO_SetBits(GPIOE, GPIO_Pin_6);

	/* Configure SELE9  pin: CS_ADC pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	/* 推挽输出 */
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOF, GPIO_Pin_1);

/**********************************************************************************/	
//	/* SPI2 Config */
//	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
//	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
//	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
//	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
//	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
//	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
//	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
//	SPI_InitStructure.SPI_CRCPolynomial = 7;
//	SPI_Init(SPI2, &SPI_InitStructure);
//
//	/* SPI2 enable */ 
//	SPI_Cmd(SPI2, ENABLE);


#ifdef ADS1243_SPI_ANALOG
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   	/* 复用推挽输出 */
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		/* 浮空输入 */
	GPIO_Init(GPIOD, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP; 	/* 推挽输出 */
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOD, GPIO_Pin_11);					/* 将CS_ADC置为高,不选中ADC */
#endif
}

void SPI_DIN(u8 assert)
{
	if (assert) {
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);			/* 选中 ADS1243 */		
	} else {
		GPIO_SetBits(GPIOB, GPIO_Pin_15);	
	}
}


void SPI_SCK(u32 assert)
{
	if (assert) {
		GPIO_ResetBits(GPIOB, GPIO_Pin_13);			/* 选中 ADS1243 */		
	} else {
		GPIO_SetBits(GPIOB, GPIO_Pin_13);	
	}
}


void SPI_CS(u32 assert)
{
	if (assert) {
		GPIO_ResetBits(GPIOD, GPIO_Pin_11);			/* 选中ADS1243 */		
	} else {
		GPIO_SetBits(GPIOD, GPIO_Pin_11);	
	}
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
void ADS1243_Init(void)
{	
	u8 num;
	ADS1243_GPIO_Init();
	/* 选中ADS1243 */  
    GPIO_ResetBits(GPIOD, GPIO_Pin_11);	
	/*  ADC  自动校准    */
   	while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;  					/* wait for /DRDY = 0 */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	/* Send byte through the SPI2 peripheral */
	SPI_I2S_SendData(SPI2, ADS1243_CMD_SELFCAL);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
  	  num = SPI_I2S_ReceiveData(SPI2);
	 for(num=0;num<5;num++)
	 {
	while ( !(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;  					/* wait for /DRDY = 1,转换一次读一次*/  
	while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;  					/* wait for /DRDY = 0 */  
	 }
	     /* 不选中ADS1243 */  
    GPIO_SetBits(GPIOD, GPIO_Pin_11);
	DelayMS(1);	

}
/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
void ADS1243_AssertCS(u32 assert)
{
	if (assert) {
		GPIO_ResetBits(GPIOD, GPIO_Pin_11);			/* 选中ADS1243 */		
	} else {
		GPIO_SetBits(GPIOD, GPIO_Pin_11);	
	}
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
u32 ADS1243_WaitDataReady(u32 time_out)
{

   if (time_out > 0) {           
      while (!(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12)) && (time_out-- > 0)) ;  /* wait for /DRDY = 1 */       
      while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12)) && (time_out-- > 0)) ;  /* wait for /DRDY = 0 */  
      if (!time_out)   
         return ADS1240_TIMEOUT_WARNING;   
   } else {          
      while (!(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ; 		 			/* wait for /DRDY = 1  */       
      while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;  					/* wait for /DRDY = 0 */  
   }


#ifdef ADS1243_SPI_ANALOG

#endif
   return ADS1240_NO_ERROR;	
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
void ADS1243_SendByte(u32 byte_num)
{
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI2, byte_num);	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
  	 SPI_I2S_ReceiveData(SPI2);
#ifdef ADS1243_SPI_ANALOG	//发送一个字节数据，连续写数据时用。
	u8 i = 8;
	 
	while (i--) {
		SPI_SCK(0); 
		SpiDelay(10);

		SPI_DIN(byte_num & 0x80);
		SpiDelay(10);

        SPI_SCK(1);
        byte_num <<= 1;
     }
     SPI_DIN(1);
#endif
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
u32 ADS1243_ReceiveByte(void)
{
	u32 rdata = 0;

/* 使用STM32的SPI口 */
	//while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	rdata = SPI_I2S_ReceiveData(SPI2);	

/* 使用模拟的SIP口 */
#ifdef ADS1243_SPI_ANALOG
	u8 r = 24;

	ADS1243_AssertCS(ADS1243_ENABLE);
	SpiDelay(10);
	ADS1243_WaitDataReady(0);

	while (r--) {
		SPI_SCK(0);
		SpiDelay(10);
		SPI_SCK(1);
		SpiDelay(10);
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)) {
			rdata++;
		}            
		rdata <<= 1;
	}
	ADS1243_AssertCS(ADS1243_DISABLE);
	SpiDelay(10);

#endif

/* 返回读得的值 */
	return rdata;
}

static u8 SPI_ReadWriteByte(u8 data)
{	  /* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	/* Send byte through the SPI2 peripheral */
	SPI_I2S_SendData(SPI2, data);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
  	return SPI_I2S_ReceiveData(SPI2);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
u32  ADS1243_ReadData(u32 Wait_data_ready)
{
	u32 data = 0;
	u8 temp[3] = {0,0,0},temp1[3] = {0x30,0x01,0x03};
	u8 mux_mode = 0xb0;
//	ADS1243_WriteRegister(ADS1243_MUX_REGISTER, 2, &temp1[0]);
    GPIO_ResetBits(GPIOD, GPIO_Pin_11);			/* 选中ADS1243 */  
//	while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;  					/* wait for /DRDY = 0 */ 
//	ADS1243_SendResetCommand(); 
//	GPIO_ResetBits(GPIOD, GPIO_Pin_11);			/* 选中ADS1243 */ 
//	while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;  					/* wait for /DRDY = 0 */  
//	if (Wait_data_ready)   
//		ADS1243_WaitDataReady(0);

#if 0	/* 直接读取转换结果 */
	while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12)));
	SPI_ReadWriteByte(ADS1243_CMD_RDATAC);
	data = SPI_ReadWriteByte(0);
	data = (data << 8) | SPI_ReadWriteByte(0);
	data = (data << 8) | SPI_ReadWriteByte(0);
#else	/* 通过读取寄存器的值得到转换结果 */
	ADS1243_ReadRegister(ADS1243_DOR2_REGISTER, 3, &temp[0]);
//	ADS1243_ReadRegister( ADS1243_MUX_REGISTER,2,&temp1[0]);	
	data = temp[0];
//	printf("%x    ,%x   ,%x\r\n",temp[0],temp[1],temp[2]);
//	 printf("%x    ,%x   ,%x\r\n",temp1[0],temp1[1],temp1[2]);
	data = (data << 8) | temp[1];
	data = (data << 8) | temp[2]; 				
#endif
//	SPI_ReadWriteByte(ADS1243_CMD_STOPC);
//	ADS1243_SendResetCommand();
   	ADS1243_AssertCS(ADS1243_DISABLE);
//	DelayMS(5);
	return (data);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
void ADS1243_ReadRegister(u32 start_address, u32 num_regs, u8 *pData)
{
	int i = 0;
//	u8 data = 0;  

	ADS1243_AssertCS(ADS1243_ENABLE);   							/* assert CS to start transfer */  

  	while ( !(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;  					/* wait for /DRDY = 1 */  
	while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;  					/* wait for /DRDY = 0 */  	
  
	SPI_ReadWriteByte(ADS1243_CMD_RREG | (start_address & 0x0f));	/* send the command byte */   
	while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;  					/* wait for /DRDY = 0 */     	   
	SPI_ReadWriteByte(num_regs - 1);      							/* send the command argument */
	for(i=300;i>0;i--);											            /*延时   必须要*/
	//SpiDelay(cnt++); 
   	    	   
	for (i=0; i<num_regs; i++) {
	/* Loop while DR register in not emplty */	  	 
  		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		 /* Send byte through the SPI2 peripheral */
     	SPI_I2S_SendData(SPI2, 0x05);
		/* Wait to receive a byte */
        while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
		/* achieve the byte read from the SPI bus */						
    	*pData++ = SPI_I2S_ReceiveData(SPI2);   					/* get the register content */ 
	}      	  
	ADS1243_AssertCS(ADS1243_DISABLE); 		   						/* de-assert CS */ 

}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
void ADS1243_WriteRegister(u32 start_address, u32 num_regs, u8 *pData)
{
	int i;     	    
	ADS1243_AssertCS(ADS1243_ENABLE);   							/* assert CS to start transfer */  
	while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;			/* wait for /DRDY = 0 */  		    
	SPI_ReadWriteByte(ADS1243_CMD_WREG | (start_address & 0x0f)); 	/* send the command byte */  
    while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;			/* wait for /DRDY = 0 */   	   
	SPI_ReadWriteByte(num_regs - 1);    							/* send the command argument */        	    
	for (i=0; i<num_regs; i++) 
	{   
		SPI_ReadWriteByte(*(pData++));								/* send the data bytes */ 	 
   	}
   	ADS1243_AssertCS(ADS1243_DISABLE); 		   						/* de-assert CS */

}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
void ADS1243_SendResetCommand(void)
{	  
	ADS1243_AssertCS(ADS1243_ENABLE);  		/* assert CS to start transfe */       	   
	ADS1243_SendByte(ADS1243_CMD_RESET);   	/* send the command byte */   	   
	ADS1243_AssertCS(ADS1243_DISABLE);		/* de-assert CS */ 
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
void ADS1243_SetChannel(u8 mux_code)
{
 	u8 temp = 0x01;


	ADS1243_SendResetCommand();

//	SpiDelay(1000);

	ADS1243_ReadRegister(ADS1243_MUX_REGISTER, 1, &temp);

		
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
void ADS1243_SetGain(u8 gain_code)
{
   u8 temp = 0;   
      
   	/*
 	the gain code is only part of the register, so we have to read it back   
   	and massage the new gain code into it   
   	*/   
 

  
   	/* clear prev gain code; */    
//   	temp &= ~0x07;   
//   	temp |= gain_code & 0x07;   
      
   	/* write the register value containing the new gain code back to the ADS1243 */    
   	ADS1243_WriteRegister(ADS1243_SETUP_REGISTER, 1, &gain_code);
//	SpiDelay(1000);	

   	ADS1243_ReadRegister(ADS1243_SETUP_REGISTER, 1, &temp);
  	printf("SETUP:0x%x\r\n", temp); 
 
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           
*
* Description: 
*
* Arguments  :  
*
* Returns    : 
*                           
*              
*********************************************************************************************************
*/
void ADS1243_SelfCalibrate(void)
{
	;		
}


/*模拟SPI*/
/*SPI send bite*/
void SPI_WriteByte_one(u8 data)
{
 u8 i=8;
 while(i--)
 {
  if(data & 0x80)
  {
   TP_OUT_H;
  }	else{
   TP_OUT_L;
  }
  TP_CLK_H;
  TP_CLK_L;
  data = data << 1;
 }
}
/* SPI recieve bite*/
u8 SPI_ReadByte_one(void)
{
	u8 i = 8;
	u16 temp = 0;
	while(i--)
	{
	TP_CLK_H;
	if(TP_IN){
	temp |= 1<<i;}
	else{
	temp |= 0<<i;}
	TP_CLK_L;
	}
	return temp;
}

/*SPI send CMD*/
u32 SPI_ADC(void)
{
	u32	  data5;
	u8  temp[3]={0,0,0};
	u8  data3[3]={0x1d,0x20,0x30};
	u8  num[3]={1,2,3};
	u8  i=0;
	GPIO_ResetBits(GPIOD, GPIO_Pin_11);     /*选中ADS1243*/
	while ( !(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;  					/* wait for /DRDY = 1 */  
	while ( (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12))) ;  					/* wait for /DRDY = 0 */  
	SPI_WriteByte_one(data3[0]);
	SPI_WriteByte_one(num[2]);
	for(i=100;i>0;i--);
	i = num[2];
	for(;i>0;i--)
	{
	  temp[i-1]=SPI_ReadByte_one();
	}
	GPIO_SetBits(GPIOD, GPIO_Pin_11);     /*不选中ADS1243*/
	data5 = temp[2];
//	printf("%x    ,%x   ,%x\r\n",temp[0],temp[1],temp[2]);
//	 printf("%x    ,%x   ,%x\r\n",temp1[0],temp1[1],temp1[2]);
	data5 = (data5 << 8) | temp[1];
	data5 = (data5 << 8) | temp[0]; 
	return data5;
}
