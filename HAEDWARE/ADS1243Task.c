#include "sys.h"
#include "bsp.h"
#include "includes.h"

u32 adsNow[8];		//保存最新采集的值
u32 ads1Result[8];	//滤波后的值
void ADS1243_Task1(void *pdata)
{
	u8 i = 0;
	u8 ads1_flag = 0;
	SPI3_ConfigInit();
	ADS1243_Init1();
	
	OSTimeDlyHMSM(0,0,0,500);
	while(1)
	{
		adsNow[i] = ADS1243_ReadDataC(i);
		if(ads1_flag < 8)//保存第一次采集值
		{
			memcpy(ads1Result,adsNow,8);
			ads1_flag++;
		}else
		{
			switch(i)//滤波
			{
//				case 0:ads1Result[0]=(ads1Result[0]>>1)+(adsNow[0]>>1);
//					varBXWRun.Vol[5] = (u32)((double)ads1Result[0]*2500.0*11/16777215);   //临时添加测试使用
//					break;
//					
//				case 1:ads1Result[1]=(ads1Result[1]>>1)+(adsNow[1]>>1);
//					varBXWRun.Vol[4] = (u32)((double)ads1Result[1]*2500.0*11/16777215);   //临时添加测试使用
//					break;
//					
//				case 2:ads1Result[2]=(ads1Result[2]>>1)+(adsNow[2]>>1);//RT2
//					varBXWRun.resistor[1] = (u32)((double)ads1Result[2]*10000.0*2.5/(3.3*16777215 - 2.5*(double)ads1Result[2]));
//					break;
//					
//				case 3:ads1Result[3]=(ads1Result[3]>>1)+(adsNow[3]>>1);//RT1
//					varBXWRun.resistor[0] = (u32)((double)ads1Result[3]*10000.0*2.5/(3.3*16777215 - 2.5*(double)ads1Result[3]));
//					break;
//					
//				case 4:ads1Result[4]=(ads1Result[4]>>1)+(adsNow[4]>>1);
//				    varBXWRun.Vol[2] = (u32)((double)ads1Result[4]*2500.0*11/16777215);   //临时添加测试使用
//					break;
//					
//				case 5:ads1Result[5]=(ads1Result[5]>>1)+(adsNow[5]>>1);
//				    varBXWRun.Vol[3] = (u32)((double)ads1Result[5]*2500.0*11/16777215);   //临时添加测试使用
//					break;
//					
//				case 6:ads1Result[6]=(ads1Result[6]>>1)+(adsNow[6]>>1);//RT4
//					varBXWRun.resistor[3] = (u32)((double)ads1Result[6]*100000.0*2.5/(3.3*16777215 - 2.5*(double)ads1Result[6]));
//					break;
//					
//				case 7:ads1Result[7]=(ads1Result[7]>>1)+(adsNow[7]>>1);//RT3
//					varBXWRun.resistor[2] = (u32)((double)ads1Result[7]*100000.0*2.5/(3.3*16777215 - 2.5*(double)ads1Result[7]));
//					break;

				case 0:ads1Result[0]=(ads1Result[0]>>1)+(adsNow[0]>>1);
					varBXWRun.Vol[5] = (u32)((double)ads1Result[0]*2500.0*13/16777215);   //临时添加测试使用
					break;
					
				case 1:ads1Result[1]=(ads1Result[1]>>1)+(adsNow[1]>>1);
					varBXWRun.Vol[4] = (u32)((double)ads1Result[1]*2500.0*13/16777215);   //临时添加测试使用
					break;
					
				case 2:ads1Result[2]=(ads1Result[2]>>1)+(adsNow[2]>>1);//RT2
					varBXWRun.resistor[1] = (u32)((double)adsNow[2]*10000.0*2.5/(3.3*16777215 - 2.5*(double)ads1Result[2]));
					break;
					
				case 3:ads1Result[3]=(ads1Result[3]>>1)+(adsNow[3]>>1);//RT1
					varBXWRun.resistor[0] = (u32)((double)adsNow[3]*10000.0*2.5/(3.3*16777215 - 2.5*(double)ads1Result[3]));
					break;
					
				case 4:ads1Result[4]=(ads1Result[4]>>1)+(adsNow[4]>>1);
				    varBXWRun.Vol[2] = (u32)((double)ads1Result[4]*2500.0*13/16777215);   //临时添加测试使用
					break;
					
				case 5:ads1Result[5]=(ads1Result[5]>>1)+(adsNow[5]>>1);
				    varBXWRun.Vol[3] = (u32)((double)ads1Result[5]*2500.0*13/16777215);   //临时添加测试使用
					break;
					
				case 6:ads1Result[6]=(ads1Result[6]>>1)+(adsNow[6]>>1);//RT4
					varBXWRun.resistor[3] = (u32)((double)adsNow[6]*120000.0*2.5/(3.3*16777215 - 2.5*(double)ads1Result[6]));
					break;
					
				case 7:ads1Result[7]=(ads1Result[7]>>1)+(adsNow[7]>>1);//RT3
					varBXWRun.resistor[2] = (u32)((double)adsNow[7]*120000.0*2.5/(3.3*16777215 - 2.5*(double)ads1Result[7]));
					break;		
				default:break;
			}
		}
		i++;
		if(i>=8)
			i=0;	
	}
}

void ADS1243_Task2(void *pdata)
{
	
}
float elecSys;
uint16_t elecIndex = 0;
uint16_t elecBuff[8][20]; 
extern OS_EVENT * ADCResultQ;				//存储ADC转换的结果
uint16_t mcuAdc[ADBUFFSIZE];
float  turnBuf = 0;
u16 sysCurrBuff[100];
float sysC;
void ADC_Task(void *pdata)
{
	uint8_t err,i;
	uint8_t ad_flag = 0;	                                                                                                                              
	uint16_t * ptrADC;
	static u8 sysCurrIndex = 0 ;
	
	while(1)
	{
		ptrADC = (uint16_t *)OSQPend(ADCResultQ,0,&err);
		if(ad_flag == 0)
		{
			memcpy(mcuAdc,ptrADC,ADBUFFSIZE);
			ad_flag = 1;
		}else						//滤波算法
		{
			for(i = 0;i < ADBUFFSIZE;i++)
//				mcuAdc[i] = (uint16_t)((mcuAdc[i]*0.9) + (ptrADC[i]*0.1));//滤波
				mcuAdc[i] = ptrADC[i];//不滤波
			if(elecIndex >= 20)
				elecIndex = 0;
			for(i = 0;i < 10;i++)	//得出电流实际值
			{
				if(i<4)				//电流 5 6 7 8
					elecBuff[4+i][elecIndex] = mcuAdc[i];
				else if(i > 5)		//电流 1 2 3 4 
					elecBuff[i-6][elecIndex] = mcuAdc[i];
			} 
			elecIndex ++;
			
			turnBuf = mcuAdc[4];	// Uin_1  电压
			varBXWRun.Vol[0]	= (u16)(((float)turnBuf*13.0*2500)/4095.0);
			turnBuf = mcuAdc[5];	// Uin_2  电压
			varBXWRun.Vol[1]	= (u16)(((float)turnBuf*13.0*2500)/4095.0);
			
			if(mcuAdc[10]<3800)		//电压测量 滤掉漂移电压
			{
				turnBuf = mcuAdc[10];// 系统电压
				varBXWRun.sysVol	= (u16)(((float)turnBuf*13.0*2500)/4095.0);
			}
//			turnBuf = mcuAdc[11];	// 系统电流
			if(mcuAdc[11]<3500)	//电流测量  滤掉漂移电流
				sysCurrBuff[sysCurrIndex++] = mcuAdc[11];
			if(sysCurrIndex>=100)
			{
				sysC = 0;
				sysCurrIndex = 0;
				for(i=0;i<100;i++)
					sysC += sysCurrBuff[i];
				sysC = sysC / 100.0;
				varBXWRun.sysCurr = (u16)((sysC*2500.0 - 4096*1650)/(4096*0.11));
			}
//			elecSys = turnBuf1;
//			elecSys=elecSys;
//			varBXWRun.sysCurr	= (u16)((turnBuf1*2500.0 - 4096*1650)/(4096*0.11));
		}
		Mem_free((void **)&ptrADC);
	}
}








