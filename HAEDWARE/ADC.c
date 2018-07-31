#include "bsp.h"


uint16_t datADC[ADBUFFSIZE] = {0,};

void TIM2_Configuration(void)
{ 
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
	TIM_OCInitTypeDef TIM_OCInitStructure; 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE); 

	TIM_TimeBaseStructure.TIM_Period 	= 999;		//计数值  10ms触发一次AD转换
	TIM_TimeBaseStructure.TIM_Prescaler = 71;		//分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;	//模式
	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;//向上计数
	TIM_TimeBaseInit(TIM2, & TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//TIM_OutputState_Disable; 
	TIM_OCInitStructure.TIM_Pulse = 1;					
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OC2Init(TIM2, & TIM_OCInitStructure);

}
//ADC_DMA init
#define ADC1_DR_Address			((u32)0x4001244C)
void ADC_DMA_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	DMA_InitTypeDef  DMA_InitStructure;				// 注：ADC为12位模数转换器，只有ADCConvertedValue的低12位有效
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//使能DMA时钟

	DMA_DeInit(DMA1_Channel1);						//开启DMA1的第一通道 
	DMA_InitStructure.DMA_PeripheralBaseAddr =  ADC1_DR_Address;	//DMA对应的外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&datADC[0];			//内存存储基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;				//DMA的转换模式为SRC模式，由外设搬移到内存
	DMA_InitStructure.DMA_BufferSize = ADBUFFSIZE;					//DMA缓存大小，10个
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//接收一次数据后，设备地址禁止后移	 
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;			//关闭接收一次数据后，目标内存地址后移
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//定义外设数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//DMA搬移数据尺寸，HalfWord就是为16位			
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;					//循环转换模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;			//DMA优先级高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;					//M2M模式禁用
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC, ENABLE);//使能传输完成中断
	
	DMA_Cmd(DMA1_Channel1, ENABLE);	

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
}
void ADC_GPIO_Configuration(void)        //ADC IO配置
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);                   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;     
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
	//U_in1   U_in2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}
void ADC_DMA_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	ADC_GPIO_Configuration();			//AD IO引脚配置
	TIM2_Configuration(); 				//定时器 2 配置
	ADC_DMA_Config();					//ADC_DMA配置
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//独立的转换模式 ADC_DUALMOD[3:0]=0000;
	ADC_InitStructure.ADC_ScanConvMode =ENABLE;		  //打开扫描模式 有十个通道
	ADC_InitStructure.ADC_ContinuousConvMode =DISABLE;//关闭连续转换模式 否则只要触发一次，
	//后续的转换就会永不停歇（除非CONT清0），这样第一次以后的ADC，就不是由TIM2_CC2来触发了	
	
	ADC_InitStructure.ADC_ExternalTrigConv =ADC_ExternalTrigConv_T2_CC2;//软件转换模式
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				//对齐方式,ADC为12位中，右对齐方式 ADC_ALIGN=0;
	ADC_InitStructure.ADC_NbrOfChannel = ADBUFFSIZE;					//开启通道数，1个  ADC_SQR1[23:20]=0000;
																		//ADC_SQR1[23:20] 设置通道数目的选择
	ADC_Init(ADC1, &ADC_InitStructure);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);		//配置时钟(12MHz),在RCC里面还应配置APB2=AHB时钟72MHz
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1 ,ADC_SampleTime_13Cycles5);//电流5
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2 ,ADC_SampleTime_13Cycles5);//电流6
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3 ,ADC_SampleTime_13Cycles5);//电流7
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4 ,ADC_SampleTime_13Cycles5);//电流8
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 5 ,ADC_SampleTime_13Cycles5);//Uin_1  电压
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 6 ,ADC_SampleTime_13Cycles5);//Uin_2  电压
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10,7 ,ADC_SampleTime_13Cycles5);//电流1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11,8 ,ADC_SampleTime_13Cycles5);//电流2
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12,9 ,ADC_SampleTime_13Cycles5);//电流3
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13,10,ADC_SampleTime_13Cycles5);//电流4
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14,11,ADC_SampleTime_13Cycles5);//系统电压
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15,12,ADC_SampleTime_13Cycles5);//系统电流
	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);	//设置外部触发模式使能（这个“外部“其实仅仅是相对于ADC模块的外部
	ADC_DMACmd(ADC1, ENABLE);   
	ADC_Cmd(ADC1, ENABLE);					//ADC命令，使能  ADC_ADON=1
	ADC_ResetCalibration(ADC1);				//重新校准
	while(ADC_GetResetCalibrationStatus(ADC1));//等待重新校准完成
	ADC_StartCalibration(ADC1);				//开始校准	ADC_RSTCAL=1; 初始化校准寄存器
	while(ADC_GetCalibrationStatus(ADC1));	//等待校准完成  ADC_CAL=0; 
	TIM_Cmd(TIM2, ENABLE);					//最后面打开定时器使能
	DMA_Cmd(DMA1_Channel1,ENABLE);			//使能DMA  
}

uint8_t ad_flag = 0;
double sysCURR = 0;

uint8_t * ptrADC;
extern OS_EVENT * ADCResultQ;				//存储ADC转换的结果
void  DMA1_Channel1_IRQHandler(void)
{
	OSIntEnter();
	
	if(DMA_GetITStatus(DMA1_IT_TC1)!=RESET)
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1);
		
		ptrADC = Mem_malloc(sizeof(datADC));
		if(ptrADC != NULL)
		{
			memcpy((uint8_t  *)ptrADC,(uint8_t  *)datADC,2*ADBUFFSIZE);
			if(OSQPost(ADCResultQ,ptrADC) != OS_ERR_NONE)
				Mem_free((void**)&ptrADC);
		}
	}
	
	OSIntExit();  //中断服务结束，系统进行任务调度
}













