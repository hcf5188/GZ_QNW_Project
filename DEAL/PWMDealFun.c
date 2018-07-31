#include "bsp.h"


extern PWM_Control PWMDat[9];		//8路PWM
extern TIMEOUTPUT_pwm pwmOC[8];		//8路大占空比大频率发生器
TIMEOUTPUT_pwm lowPWMOC[8];			//低速PWM
extern void TIMOCSet(TIM_TypeDef *TIMSelect,uint16_t CCR_Val,uint8_t channel);
//9路PWM控制
void BXW_PWMDataDeal(PWM_Data * ptr,uint8_t PWMId)
{
	uint32_t period;//
	uint16_t duty;
	uint8_t i;
	uint8_t type= 0;
	GPIO_InitTypeDef  GPIO_InitStr;

	PWMDat[PWMId].state        = ptr->state;
	PWMDat[PWMId].level		   = ptr->level;
	PWMDat[PWMId].dutyCycleSma = ptr->smallDuty;
	PWMDat[PWMId].freSma       = ptr->smallFre;
	PWMDat[PWMId].dutyCycleBig = ptr->bigDuty;
	PWMDat[PWMId].freBig       = ptr->bigFre;
	if(PWMId == 8)//5V 电路
	{
		if(PWMDat[8].state == 1)
			GPIO_ResetBits(PWMDat[8].GPIOPWM,(1<<PWMDat[8].pinPWM ));
		else
			GPIO_SetBits(PWMDat[8].GPIOPWM,(1<<PWMDat[8].pinPWM ));
			return;
	}
	if(PWMDat[PWMId].state == 0)
	{
		switch(PWMDat[PWMId].ocType)//此引脚所在的OC类型
		{		
			case 0 :return;//没有关联任何
			//高速
			case 1 :pwmOC[PWMDat[PWMId].numPWMOC].isOPEN = CLOSE;
				break;
			//低速  
			case 2 :lowPWMOC[PWMDat[PWMId].numPWMOC].isOPEN = CLOSE;//在TIM6中会关闭此通道
				break;
			default:
				break;
		}
		varBXWRun.dutyCyclePWM[PWMId] = 0;	//要发送给平板的占空比
//		PWMDat[PWMId].ocType = 0;			//OC类型为空
		return;
	}
	varBXWRun.dutyCyclePWM[PWMId] = ptr->bigDuty;//要发送给平板的占空比
	if((ptr->bigDuty > 5)&&(ptr->bigDuty < 995))
	{
		period = 10000000.0 / PWMDat[PWMId].freBig;
		if(period > 65500)//大频率 低速 并且是PWM通道，不是5V控制PWM
		{
			for(i=0;i<8;i++)
			{		
				if((lowPWMOC[i].isOPEN == OPEN)||(lowPWMOC[i].pwmType != 0))
					continue;
				else 
					break;
			}
			if(PWMDat[PWMId].numPWMOC > 9)
			{	
				if(PWMDat[PWMId].dutyCycleSma >= 1000)//只有大占空比
				{
					PWMDat[PWMId].dutyCycleSma = 1000;
					type =BIG_PWM;			
					GPIO_InitStr.GPIO_Mode = GPIO_Mode_Out_PP;
					GPIO_InitStr.GPIO_Pin = (u16)(1<<PWMDat[PWMId].pinPWM);
					GPIO_InitStr.GPIO_Speed	= GPIO_Speed_50MHz;
					GPIO_Init(PWMDat[PWMId].GPIOPWM,&GPIO_InitStr);
				}
				else// 既有大占空比，又有小占空比
				{
					// 小占空比
					duty = (uint16_t)((double)SMALLCycle*PWMDat[PWMId].dutyCycleSma/1000.0);
					switch(PWMDat[PWMId].OCNumSmall)//设置占空比
					{
						case 1:TIM_SetCompare1(PWMDat[PWMId].TIMsmall,duty);break;
						case 2:TIM_SetCompare2(PWMDat[PWMId].TIMsmall,duty);break;
						case 3:TIM_SetCompare3(PWMDat[PWMId].TIMsmall,duty);break;
						case 4:TIM_SetCompare4(PWMDat[PWMId].TIMsmall,duty);break;
					}
					type = SMALL_PWM;
					GPIO_InitStr.GPIO_Pin = (1<<PWMDat[PWMId].pinPWM);
					GPIO_InitStr.GPIO_Mode = GPIO_Mode_AF_PP;
					GPIO_InitStr.GPIO_Speed	= GPIO_Speed_50MHz;
					GPIO_Init(PWMDat[PWMId].GPIOPWM,&GPIO_InitStr);
				}
				duty = 4;
				PWMDat[PWMId].numPWMOC = i;	//索引到对应的 lowPWMOC 的编号
				PWMDat[PWMId].ocType   = 2;	//低速 PWM
				
				lowPWMOC[i].timeCnt = 0;
				lowPWMOC[i].pwmControl = &PWMDat[PWMId];
				
				TIM_Cmd(TIM6,DISABLE);		//关闭定时器6中断
				lowPWMOC[i].isOPEN = OPEN;
				lowPWMOC[i].pwmType = type;
				TIM_Cmd(TIM6,ENABLE);		//开启定时器6中断
			}
			lowPWMOC[PWMDat[PWMId].numPWMOC].timeWork = 0;	//有数据过来
			lowPWMOC[PWMDat[PWMId].numPWMOC].period = 10000.0 / PWMDat[PWMId].freBig;
			lowPWMOC[PWMDat[PWMId].numPWMOC].hTime  = (uint16_t)((lowPWMOC[PWMDat[PWMId].numPWMOC].period*PWMDat[PWMId].dutyCycleBig)/1000.0);
		}
		else if(period <= 65500)			//只有大频率，高速 高精度
		{
			for(i = 0;i<8;i++)
			{
				if((pwmOC[i].isOPEN == OPEN)||(pwmOC[i].pwmType!=0))
					continue;
				else
					break;
			}
			if(PWMDat[PWMId].numPWMOC > 9)	//还没绑定PWM
			{
				PWMDat[PWMId].numPWMOC	= i;
				PWMDat[PWMId].ocType    = 1;//高速 PWM
				
				pwmOC[i].isOPEN 		= OPEN;
				pwmOC[i].pwmType 		= BIG_PWM;
				pwmOC[i].pwmControl     = &PWMDat[PWMId];
				
				GPIO_InitStr.GPIO_Pin   = 1<<PWMDat[PWMId].pinPWM;
				GPIO_InitStr.GPIO_Mode  = GPIO_Mode_Out_PP;
				GPIO_InitStr.GPIO_Speed	= GPIO_Speed_50MHz;
				GPIO_Init(PWMDat[PWMId].GPIOPWM,&GPIO_InitStr);
				
				pwmOC[i].period = 10000000.0 / PWMDat[PWMId].freBig;
				pwmOC[i].hTime = (uint16_t)((pwmOC[i].period*PWMDat[PWMId].dutyCycleBig)/1000.0);
				TIMOCSet(pwmOC[i].TIMBig,pwmOC[i].hTime,pwmOC[i].OCNumBig);//开启中断
			}
			pwmOC[PWMDat[PWMId].numPWMOC].timeWork = 0;//有数据过来
			//周期
			pwmOC[PWMDat[PWMId].numPWMOC].period = 10000000.0 / PWMDat[PWMId].freBig;
			//高电平时间
			pwmOC[PWMDat[PWMId].numPWMOC].hTime = (uint16_t)((pwmOC[PWMDat[PWMId].numPWMOC].period*PWMDat[PWMId].dutyCycleBig)/1000.0);
			//低电平时间
			pwmOC[PWMDat[PWMId].numPWMOC].lTime = pwmOC[PWMDat[PWMId].numPWMOC].period -pwmOC[PWMDat[PWMId].numPWMOC].hTime;
		}
	}else if(ptr->bigDuty <= 5)
	{
		GPIO_InitStr.GPIO_Pin   = 1<<PWMDat[PWMId].pinPWM;	// PWM  OUT1 I 
		GPIO_InitStr.GPIO_Mode  = GPIO_Mode_Out_PP;
		GPIO_InitStr.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(PWMDat[PWMId].GPIOPWM,&GPIO_InitStr);
		GPIO_ResetBits(PWMDat[PWMId].GPIOPWM,1<<PWMDat[PWMId].pinPWM);
	}else if(ptr->bigDuty >= 995)
	{
		GPIO_InitStr.GPIO_Pin   = 1<<PWMDat[PWMId].pinPWM;	// PWM  OUT1 I 
		GPIO_InitStr.GPIO_Mode  = GPIO_Mode_Out_PP;
		GPIO_InitStr.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(PWMDat[PWMId].GPIOPWM,&GPIO_InitStr);
		GPIO_SetBits(PWMDat[PWMId].GPIOPWM,1<<PWMDat[PWMId].pinPWM);
	}
}
//直流电机控制  步进电机控制
extern DCM_Control DCMDat[4];
void BXW_DCMDataDeal(DCM_Data * ptr,uint8_t PWMId)
{
	uint8_t  i;
	if(ptr->cmdId == 0x11)//接线错误
		DCMDat[PWMId].stepErr = 1;
	else//接线正确
		DCMDat[PWMId].stepErr = 0;
		
	DCMDat[PWMId].state = ptr->state;	//状态
	DCMDat[PWMId].fwd   = ptr->fwd;		//方向
	DCMDat[PWMId].dutyCycle = ptr->duty;
	DCMDat[PWMId].fre		= ptr->fre;
	if(PWMId < 3)						//只针对直流电机
	{
		varBXWRun.dcm[PWMId].dcmDer = ptr->state;
		varBXWRun.dcm[PWMId].dcmCyc = ptr->duty;
	}

	if(DCMDat[PWMId].state == 0)		//停止PWM波形
	{
		if(DCMDat[PWMId].numPWMOC > 9)	//已经停止波形
			return;
		pwmOC[DCMDat[PWMId].numPWMOC].isOPEN = CLOSE;
//		DCMDat[PWMId].numPWMOC = 20;	//没有任何索引
		return;
	}
	if(PWMId < 3)//直流电机控制
	{
		for(i = 0;i<8;i++)
		{
			if((pwmOC[i].isOPEN == OPEN)&&(pwmOC[i].pwmType!=0))
				continue;
			else
				break;
		}
		if(DCMDat[PWMId].numPWMOC > 9)			//还没绑定PWM
		{
			DCMDat[PWMId].numPWMOC	= i;		
			pwmOC[i].isOPEN 		= OPEN;		
			pwmOC[i].pwmType 		= DC_MOTOR;	//直流电机模式
			pwmOC[i].dcmControl     = &DCMDat[PWMId];
			
			pwmOC[i].period = 10000000.0 / DCMDat[PWMId].fre;
			
			if(pwmOC[i].period>65500)
				pwmOC[i].period = 65500;
				
			pwmOC[i].hTime = (uint16_t)((pwmOC[i].period * DCMDat[PWMId].dutyCycle)/1000.0);
			TIMOCSet(pwmOC[i].TIMBig,pwmOC[i].hTime,pwmOC[i].OCNumBig);//开启中断
		}
		pwmOC[DCMDat[PWMId].numPWMOC].timeWork = 0;//有数据过来
		pwmOC[DCMDat[PWMId].numPWMOC].period = 10000000.0 / DCMDat[PWMId].fre;
		if(pwmOC[DCMDat[PWMId].numPWMOC].period > 65500)
				pwmOC[DCMDat[PWMId].numPWMOC].period = 65500;
		pwmOC[DCMDat[PWMId].numPWMOC].hTime = (uint16_t)((pwmOC[DCMDat[PWMId].numPWMOC].period * DCMDat[PWMId].dutyCycle)/1000.0);
		pwmOC[DCMDat[PWMId].numPWMOC].lTime = pwmOC[DCMDat[PWMId].numPWMOC].period - pwmOC[DCMDat[PWMId].numPWMOC].hTime;
	}
	else if(PWMId == 3)//步进电机控制
	{
		for(i = 0;i < 8;i++)
		{
			if((pwmOC[i].isOPEN == OPEN)&&(pwmOC[i].pwmType!=0))
				continue;
			else
				break;
		}
		if(DCMDat[PWMId].numPWMOC > 9)			//还没绑定PWM
		{
			DCMDat[PWMId].numPWMOC	= i;		
			pwmOC[i].isOPEN 		= OPEN;		
			pwmOC[i].pwmType 		= STEP_MOTOR;	//直流电机模式
			pwmOC[i].dcmControl     = &DCMDat[PWMId];
			
			pwmOC[i].dcmControl1    = &DCMDat[0];//A相
			pwmOC[i].dcmControl2    = &DCMDat[1];//B相
			pwmOC[i].stepNum	 	= 1; //步号为0
			
			pwmOC[i].period = 10000000.0 / DCMDat[PWMId].fre;
			if(DCMDat[PWMId].fwd == 1)
				pwmOC[i].direct = 1;
			else 
				pwmOC[i].direct = 0;
			if(pwmOC[i].period > STEP_START_FRE)//步进电机初始频率
				pwmOC[i].period = STEP_START_FRE;
			else
				pwmOC[DCMDat[PWMId].numPWMOC].hTime = (STEP_START_FRE - pwmOC[i].period)/50;
			
			pwmOC[DCMDat[PWMId].numPWMOC].periodCalc = STEP_START_FRE;//起始频率是50Hz

			TIMOCSet(pwmOC[i].TIMBig,pwmOC[i].periodCalc,pwmOC[i].OCNumBig);//开启中断
		}
		pwmOC[DCMDat[PWMId].numPWMOC].timeWork = 0;//有数据过来 清零
	}
}



















