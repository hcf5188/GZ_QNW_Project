#include "bsp.h"

void TIMOCSet(TIM_TypeDef *TIMSelect,uint16_t CCR_Val,uint8_t channel)
{
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;  
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  
	TIM_OCInitStructure.TIM_Pulse = CCR_Val;  
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	switch(channel)
	{
		case 1:	TIM_OC1Init(TIMSelect, &TIM_OCInitStructure);  
				TIM_OC1PreloadConfig(TIMSelect, TIM_OCPreload_Disable); 
				TIM_ITConfig(TIMSelect,TIM_IT_CC1,ENABLE);break;
		case 2:	TIM_OC2Init(TIMSelect, &TIM_OCInitStructure);  
				TIM_OC2PreloadConfig(TIMSelect, TIM_OCPreload_Disable); 
				TIM_ITConfig(TIMSelect,TIM_IT_CC2,ENABLE);break;
		case 3:	TIM_OC3Init(TIMSelect, &TIM_OCInitStructure);  
				TIM_OC3PreloadConfig(TIMSelect, TIM_OCPreload_Disable); 
				TIM_ITConfig(TIMSelect,TIM_IT_CC3,ENABLE);break;
		case 4:	TIM_OC4Init(TIMSelect, &TIM_OCInitStructure);  
				TIM_OC4PreloadConfig(TIMSelect, TIM_OCPreload_Disable); 
				TIM_ITConfig(TIMSelect,TIM_IT_CC4,ENABLE);break;
	} 
}
void PWMInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
		
	//使能 A B C D E 端口时钟以及TIM1、TIM3时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_AFIO, ENABLE);	 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);//使能TIM1时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);//使能TIM4时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//使能TIM3时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);//使能TIM5时钟
	
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_12|GPIO_Pin_13; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	GPIO_SetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_13);
	
	//PWM 3 - 4
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_6|GPIO_Pin_7; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIO_SetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_7);
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);		//重定向
	//PWM 5 - 8
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_9|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_14; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_SetBits(GPIOE,GPIO_Pin_9|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_14);
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);	//重定向
	// 5V 给电 引脚配置
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_8;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	
	TIM_TimeBaseStructure.TIM_Period = SMALLCycle;	//自动重装载寄存器的值
	TIM_TimeBaseStructure.TIM_Prescaler =0;			//TIMX预分频的值
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	//时钟分割
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//向上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);		//根据以上功能对定时器进行初始化
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);		//根据以上功能对定时器进行初始化
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//选择定时器模式，TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		//输出比较极性低
	TIM_OCInitStructure.TIM_Pulse = SMALLCycle/3;					//占空比 33%
	
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);//根据结构体信息进行初始化  PWM1 - PD12
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);                         
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);//根据结构体信息进行初始化  PWM2 - PD13
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);                         
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);//根据结构体信息进行初始化  PWM3 - PD14
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);                         
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);//根据结构体信息进行初始化  PWM4 - PD15
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);  
	//高级定时器才有的互补输出设置
	TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Reset; 
	TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;
	
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);//根据结构体信息进行初始化  PWM5 - PE9
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);                           
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);//根据结构体信息进行初始化  PWM6 - PE11
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);                           
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);//根据结构体信息进行初始化  PWM7 - PE13
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);                           
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);//根据结构体信息进行初始化  PWM8 - PE14
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);  
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	TIM_ARRPreloadConfig(TIM3,ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	TIM_ARRPreloadConfig(TIM1,ENABLE);
	TIM_Cmd(TIM1, ENABLE);
	
/************************产生高精度的PWM大波形*************************/
	
	TIM_TimeBaseStructure.TIM_Period = 65535;					//65535   us 
	TIM_TimeBaseStructure.TIM_Prescaler = 71;  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;  
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);  
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); 
	
	TIM_Cmd(TIM4, ENABLE); 
	TIM_Cmd(TIM5, ENABLE);
}

// 5ms中断一次
void TIM6_Init(void)
{
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;
	NVIC_InitTypeDef		NVIC_InitStructure; 
	//时钟使能 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); 
	//定时器 TIM6 初始化
	TIM_TimeBaseStructure.TIM_Period    = 999;		//设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler = 71;		//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;      //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;//TIM向上计数模式
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);	//根据指定的参数初始化TIMx的时间基数单位
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);        //使能指定的TIM6中断,允许更新中断
	TIM_Cmd(TIM6, ENABLE);  						//使能TIMx		
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;//开定时器3中断 用于产生大PWM波
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;//开定时器5中断 大占空比的PWM波形
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

PWM_Control PWMDat[9];	//8路PWM  + 1路5V控制PWM
DCM_Control DCMDat[4];	//三路DCM控制

void PWMDataInit(void)//PWM引脚初始化
{
	uint8_t i;
	GPIO_InitTypeDef  GPIO_InitStructure;
	for(i = 0;i < 9;i++)
	{
		PWMDat[i].numPWMOC = 20;
		PWMDat[i].ocType = 0;
		PWMDat[i].state	= 0;
		PWMDat[i].level = 0;
		PWMDat[i].dutyCycleBig 	= 0;
		PWMDat[i].dutyCycleSma  = 0;
		PWMDat[i].freBig	= 0;
		PWMDat[i].freSma	= 0;
		PWMDat[i].pinState  = 0;
		PWMDat[i].num       = i;
	}
	for(i=0;i<9;i++)
	{
		if(i<2)
		{
			PWMDat[i].GPIOPWM = GPIOD;	//PWM1 - 4
//			PWMDat[i].TIMsmall = TIM3;	//小定时器是TIM3
			PWMDat[i].pinPWM = 12 + i;	//PWM引脚
			PWMDat[i].OCNumSmall = i+1;	//小频率所在的通道，用于设置小占空比
		}
		else if(i<4)
		{
			PWMDat[i].GPIOPWM = GPIOC;	//PWM1 - 4
			PWMDat[i].TIMsmall = TIM3;	//小定时器是TIM3
			PWMDat[i].pinPWM = 4 + i;	//PWM引脚
			PWMDat[i].OCNumSmall = i - 1;	//小频率所在的通道，用于设置小占空比
		}
		else if(i < 7)
		{
			PWMDat[i].GPIOPWM = GPIOE;	//PWM 5 - 7
			PWMDat[i].TIMsmall = TIM1;
			PWMDat[i].pinPWM = 1 + (i*2);
			PWMDat[i].OCNumSmall = i-3;
		}else if(i == 7)
		{
			PWMDat[i].GPIOPWM = GPIOE;	//PWMDat 8
			PWMDat[i].TIMsmall = TIM1;
			PWMDat[i].pinPWM = 14;
			PWMDat[i].OCNumSmall = 4;
		}
		else if(i == 8)
		{
			PWMDat[i].GPIOPWM = GPIOA;	//PWM 9   5V控制PWM
			PWMDat[i].TIMsmall = NULL;
			PWMDat[i].pinPWM = 8;
		}
	}
	//输入引脚
	PWMDat[0].GPIO_In = GPIOC;//Out1_U
	PWMDat[0].pinIn	  = 15;
	PWMDat[1].GPIO_In = GPIOE;//Out2_U
	PWMDat[1].pinIn	  = 15;
	PWMDat[2].GPIO_In = GPIOE;//Out3_U
	PWMDat[2].pinIn	  = 12;
	PWMDat[3].GPIO_In = GPIOE;//Out4_U
	PWMDat[3].pinIn	  = 10;
	PWMDat[4].GPIO_In = GPIOE;//Out5_U
	PWMDat[4].pinIn	  = 8;
	PWMDat[5].GPIO_In = GPIOE;//Out6_U
	PWMDat[5].pinIn	  = 7;
	PWMDat[6].GPIO_In = GPIOD;//Out7_U
	PWMDat[6].pinIn	  = 4;
	PWMDat[7].GPIO_In = GPIOD;//Out8_U
	PWMDat[7].pinIn	  = 5;
	//将引脚配置为输入
	for(i = 0;i < 8;i++)
	{
		GPIO_InitStructure.GPIO_Pin	= (1<<PWMDat[i].pinIn); 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//输入上拉
		GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
		GPIO_Init(PWMDat[i].GPIO_In,&GPIO_InitStructure);
	}
/*******************  直流电机、步进电机相关参数初始化   *******************/
	for(i = 0;i < 4;i++)
	{
		DCMDat[i].numPWMOC = 20;//默认没有关联任何OC
		DCMDat[i].state = 0;   	//状态
		DCMDat[i].fwd = 0;		//方向
		DCMDat[i].dutyCycle = 0;//大占空比，跟直流电机有关
		DCMDat[i].fre = 0;		//大频率，直流、步进都有用到
	}
	DCMDat[0].GPIO_FWD = GPIOE;
	DCMDat[0].GPIO_REV = GPIOE;
	DCMDat[0].GPIO_Err = GPIOE;
	DCMDat[0].fwd_Pin  = 5;
	DCMDat[0].rev_Pin  = 6;
	DCMDat[0].err_Pin  = 4;
	
	DCMDat[1].GPIO_FWD = GPIOE;
	DCMDat[1].GPIO_REV = GPIOE;
	DCMDat[1].GPIO_Err = GPIOE;
	DCMDat[1].fwd_Pin  = 2;
	DCMDat[1].rev_Pin  = 3;
	DCMDat[1].err_Pin  = 1;
	
	DCMDat[2].GPIO_FWD = GPIOE;
	DCMDat[2].GPIO_REV = GPIOD;
	DCMDat[2].GPIO_Err = GPIOD;
	DCMDat[2].fwd_Pin  = 0;
	DCMDat[2].rev_Pin  = 6;
	DCMDat[2].err_Pin  = 7;
	
	for(i = 0;i < 3;i++)
	{
		GPIO_InitStructure.GPIO_Pin	= (1<<DCMDat[i].fwd_Pin); 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
		GPIO_Init(DCMDat[i].GPIO_FWD,&GPIO_InitStructure);
		GPIO_SetBits(DCMDat[i].GPIO_FWD,(1<<DCMDat[i].fwd_Pin));
		
		GPIO_InitStructure.GPIO_Pin	= (1<<DCMDat[i].rev_Pin);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
		GPIO_Init(DCMDat[i].GPIO_REV,&GPIO_InitStructure);
		GPIO_SetBits(DCMDat[i].GPIO_REV,(1<<DCMDat[i].rev_Pin));
		
		GPIO_InitStructure.GPIO_Pin	= (1<<DCMDat[i].err_Pin);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(DCMDat[i].GPIO_Err,&GPIO_InitStructure);
	}
	DCMDat[2].err_Pin  = 8;

}
TIMEOUTPUT_pwm pwmOC[8];
void TIME_Init(void )
{
	uint8_t i = 0;
	for(i = 0;i<4;i++)
		pwmOC[i].TIMBig = TIM4;
	for(i = 4;i<8;i++)
		pwmOC[i].TIMBig = TIM5;
	pwmOC[0].IT_Source = TIM_IT_CC1;//中断源
	pwmOC[4].IT_Source = TIM_IT_CC1;
	pwmOC[1].IT_Source = TIM_IT_CC2;
	pwmOC[5].IT_Source = TIM_IT_CC2;
	pwmOC[2].IT_Source = TIM_IT_CC3;
	pwmOC[6].IT_Source = TIM_IT_CC3;
	pwmOC[3].IT_Source = TIM_IT_CC4;
	pwmOC[7].IT_Source = TIM_IT_CC4;
	
	pwmOC[0].OCNumBig = 1 ;	//所在定时器的PWM通道
	pwmOC[4].OCNumBig = 1 ;	
	pwmOC[1].OCNumBig = 2 ;	
	pwmOC[5].OCNumBig = 2 ;	
	pwmOC[2].OCNumBig = 3 ;	
	pwmOC[6].OCNumBig = 3 ;
	pwmOC[3].OCNumBig = 4 ;
	pwmOC[7].OCNumBig = 4 ;
	for(i = 0;i<8;i++)
	{
		pwmOC[i].isOPEN		= CLOSE;//此结构体是否被占用
		pwmOC[i].pwmType	= 0;	//波形类型  0默认 1-小占空比 2-大占空比  3-步进电机 4-直流电机
		pwmOC[i].hlSet		= 0;	//在定时器运行中决定引脚的高低电平
		pwmOC[i].hTime		= 0;	//高电平持续时间  PWM波形	(us)
		pwmOC[i].lTime		= 0;	//低电平持续时间			(us)
		pwmOC[i].timeCnt	= 0;	//time计数
		pwmOC[i].stepNum	= 1;	//步号
		pwmOC[i].direct		= 0;	//方向
		pwmOC[i].periodCalc = 0;
		pwmOC[i].pwmControl	= NULL;	//8路PWM	结构体
		pwmOC[i].dcmControl	= NULL;	//直流电机 	结构体
		pwmOC[i].dcmControl1= NULL;	//步进电机 	结构体
		pwmOC[i].dcmControl2= NULL;	//步进电机 	结构体
	}
}

extern TIMEOUTPUT_pwm lowPWMOC[8];	
extern uint16_t elecBuff[8][20]; 
uint16_t  elecMax = 0;	
uint8_t deviceState[8] = {0,0,0,0,0,0,0,0};
//PWM处理任务     超时关闭等功能
void PWMTask(void *pdata)
{
	uint8_t  i = 0,j = 0;
	uint8_t  ledPWMFlag = 0;
	uint16_t duty;
	static u8 execTimes = 0;
	
	TIME_Init();	//数据结构初始化
	PWMDataInit();	//控制 PWM 的数据初始化
	
	while(1)		//监控 PWM 引脚状态、电流状态、步进电机的软启动、软停止、5秒没信息，回复PWM引脚不动
	{
		ledPWMFlag = 0;
		OSTimeDlyHMSM(0,0,0,50);
		execTimes ++;
		for(i = 0;i < 8;i++)
		{
			if(pwmOC[i].isOPEN != OPEN)//此通道是关闭的
				continue;
			ledPWMFlag = 1;
			pwmOC[i].timeWork ++;
			if(pwmOC[i].timeWork >= 60)//3s没数据下发，则关闭此通道
			{
				pwmOC[i].isOPEN = CLOSE;
				continue;
			}
			if(pwmOC[i].pwmType == STEP_MOTOR)//步进电机调速
			{
				if(pwmOC[i].periodCalc > pwmOC[i].period)
					pwmOC[i].periodCalc -= pwmOC[i].hTime;
			}
		}
		for(i = 0;i < 8;i++)
		{
			if(lowPWMOC[i].isOPEN != OPEN)//此通道是开启的
				continue;
			ledPWMFlag = 1;
			lowPWMOC[i].timeWork ++;
			if(lowPWMOC[i].timeWork > 100)//5s没数据下发，则关闭此通道
			{
				lowPWMOC[i].isOPEN = CLOSE;
				continue;
			}
		}
		for(i = 0;i < 8;i++)
		{
			elecMax = 0;
			for(j = 0;j<20;j++)	//将最近二十次的最大值提取出来
				if(elecMax< elecBuff[i][j])
					elecMax = elecBuff[i][j];
			if(i<6)//得到上报电流
			{
				if(PWMDat[i].level != 0)//停止后不供电
				{	//低速，小占空比  
					if(PWMDat[i].dutyCycleBig >990)
						duty = 990;
					else if(PWMDat[i].dutyCycleBig < 10)
						duty = 10;
					else
						duty = PWMDat[i].dutyCycleBig;
					if(PWMDat[i].ocType == 2)
					{
						if(lowPWMOC[PWMDat[i].numPWMOC].pwmType == SMALL_PWM)
						{
							varBXWRun.elecPWM[i]=(uint16_t)(((double)2.5*duty * elecMax)/4096);
						}
						else
						{
							varBXWRun.elecPWM[i]=(uint16_t)(((double)2.5*(1000 - duty) * elecMax)/4096);
						}
					}
					else
						varBXWRun.elecPWM[i]=(uint16_t)(((double)2.5*(1000 - duty) * elecMax)/4096);
				}
					
				else//长供电
					varBXWRun.elecPWM[i]=(uint16_t)(((double)2.5 * 1000 * elecMax)/4096);
			}
			else
			{
				if(PWMDat[i].dutyCycleBig >990)
					duty = 990;
				else if(PWMDat[i].dutyCycleBig < 10)
					duty = 10;
				else
					duty = PWMDat[i].dutyCycleBig;
				varBXWRun.elecPWM[i]=(uint16_t)(((double)5 * (1000 - duty) * elecMax)/4096);
			}
				
			if(PWMDat[i].numPWMOC > 9)		//此路 PWM 波没有工作
			{
//				varBXWRun.elecPWM[i] = 0;	//电流清零	
				PWMDat[i].pinState = GPIO_ReadInputDataBit(PWMDat[i].GPIO_In,(1<<PWMDat[i].pinIn));
				varBXWRun.StatePWM[i] = GPIO_ReadInputDataBit(PWMDat[i].GPIO_In,(1<<PWMDat[i].pinIn));	
				continue;
			}//工作的时候，不检查设备有没有在线
			if(varBXWRun.elecPWM[i] < 20)		//电流不能小于20
			{
				deviceState[i] ++;
				if(deviceState[i] >= 25&&execTimes >= 25)
					varBXWRun.StatePWM[i] = 1;	//不在线
			}else
			{
				varBXWRun.StatePWM[i] = 0;		//在线
			}
		}
		if(execTimes >= 25)
		{
			execTimes = 0;
			for( i=0;i<8;i++)
				deviceState[i] = 0;
		}
			
		if(ledPWMFlag == 1)
			varOperaRun.ledFlag[5] = LEDON;	//有输出PWM LED 状态标志，常亮
		else
			varOperaRun.ledFlag[5] = LEDOFF;//PWM LED 状态标志，常灭
	}
}































