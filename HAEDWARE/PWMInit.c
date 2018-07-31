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
		
	//ʹ�� A B C D E �˿�ʱ���Լ�TIM1��TIM3ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_AFIO, ENABLE);	 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);//ʹ��TIM1ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);//ʹ��TIM4ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//ʹ��TIM3ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);//ʹ��TIM5ʱ��
	
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
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);		//�ض���
	//PWM 5 - 8
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_9|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_14; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_SetBits(GPIOE,GPIO_Pin_9|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_14);
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);	//�ض���
	// 5V ���� ��������
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_8;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	
	TIM_TimeBaseStructure.TIM_Period = SMALLCycle;	//�Զ���װ�ؼĴ�����ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =0;			//TIMXԤ��Ƶ��ֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;	//ʱ�ӷָ�
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//���ϼ���
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);		//�������Ϲ��ܶԶ�ʱ�����г�ʼ��
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);		//�������Ϲ��ܶԶ�ʱ�����г�ʼ��
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//ѡ��ʱ��ģʽ��TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		//����Ƚϼ��Ե�
	TIM_OCInitStructure.TIM_Pulse = SMALLCycle/3;					//ռ�ձ� 33%
	
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);//���ݽṹ����Ϣ���г�ʼ��  PWM1 - PD12
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);                         
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);//���ݽṹ����Ϣ���г�ʼ��  PWM2 - PD13
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);                         
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);//���ݽṹ����Ϣ���г�ʼ��  PWM3 - PD14
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);                         
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);//���ݽṹ����Ϣ���г�ʼ��  PWM4 - PD15
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);  
	//�߼���ʱ�����еĻ����������
	TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Reset; 
	TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;
	
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);//���ݽṹ����Ϣ���г�ʼ��  PWM5 - PE9
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);                           
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);//���ݽṹ����Ϣ���г�ʼ��  PWM6 - PE11
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);                           
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);//���ݽṹ����Ϣ���г�ʼ��  PWM7 - PE13
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);                           
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);//���ݽṹ����Ϣ���г�ʼ��  PWM8 - PE14
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);  
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	TIM_ARRPreloadConfig(TIM3,ENABLE);
	TIM_Cmd(TIM3, ENABLE);
	TIM_ARRPreloadConfig(TIM1,ENABLE);
	TIM_Cmd(TIM1, ENABLE);
	
/************************�����߾��ȵ�PWM����*************************/
	
	TIM_TimeBaseStructure.TIM_Period = 65535;					//65535   us 
	TIM_TimeBaseStructure.TIM_Prescaler = 71;  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;  
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);  
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); 
	
	TIM_Cmd(TIM4, ENABLE); 
	TIM_Cmd(TIM5, ENABLE);
}

// 5ms�ж�һ��
void TIM6_Init(void)
{
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;
	NVIC_InitTypeDef		NVIC_InitStructure; 
	//ʱ��ʹ�� 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); 
	//��ʱ�� TIM6 ��ʼ��
	TIM_TimeBaseStructure.TIM_Period    = 999;		//��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler = 71;		//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;      //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;//TIM���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);	//����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);        //ʹ��ָ����TIM6�ж�,��������ж�
	TIM_Cmd(TIM6, ENABLE);  						//ʹ��TIMx		
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;//����ʱ��3�ж� ���ڲ�����PWM��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;//����ʱ��5�ж� ��ռ�ձȵ�PWM����
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

PWM_Control PWMDat[9];	//8·PWM  + 1·5V����PWM
DCM_Control DCMDat[4];	//��·DCM����

void PWMDataInit(void)//PWM���ų�ʼ��
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
//			PWMDat[i].TIMsmall = TIM3;	//С��ʱ����TIM3
			PWMDat[i].pinPWM = 12 + i;	//PWM����
			PWMDat[i].OCNumSmall = i+1;	//СƵ�����ڵ�ͨ������������Сռ�ձ�
		}
		else if(i<4)
		{
			PWMDat[i].GPIOPWM = GPIOC;	//PWM1 - 4
			PWMDat[i].TIMsmall = TIM3;	//С��ʱ����TIM3
			PWMDat[i].pinPWM = 4 + i;	//PWM����
			PWMDat[i].OCNumSmall = i - 1;	//СƵ�����ڵ�ͨ������������Сռ�ձ�
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
			PWMDat[i].GPIOPWM = GPIOA;	//PWM 9   5V����PWM
			PWMDat[i].TIMsmall = NULL;
			PWMDat[i].pinPWM = 8;
		}
	}
	//��������
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
	//����������Ϊ����
	for(i = 0;i < 8;i++)
	{
		GPIO_InitStructure.GPIO_Pin	= (1<<PWMDat[i].pinIn); 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//��������
		GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
		GPIO_Init(PWMDat[i].GPIO_In,&GPIO_InitStructure);
	}
/*******************  ֱ����������������ز�����ʼ��   *******************/
	for(i = 0;i < 4;i++)
	{
		DCMDat[i].numPWMOC = 20;//Ĭ��û�й����κ�OC
		DCMDat[i].state = 0;   	//״̬
		DCMDat[i].fwd = 0;		//����
		DCMDat[i].dutyCycle = 0;//��ռ�ձȣ���ֱ������й�
		DCMDat[i].fre = 0;		//��Ƶ�ʣ�ֱ�������������õ�
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
	pwmOC[0].IT_Source = TIM_IT_CC1;//�ж�Դ
	pwmOC[4].IT_Source = TIM_IT_CC1;
	pwmOC[1].IT_Source = TIM_IT_CC2;
	pwmOC[5].IT_Source = TIM_IT_CC2;
	pwmOC[2].IT_Source = TIM_IT_CC3;
	pwmOC[6].IT_Source = TIM_IT_CC3;
	pwmOC[3].IT_Source = TIM_IT_CC4;
	pwmOC[7].IT_Source = TIM_IT_CC4;
	
	pwmOC[0].OCNumBig = 1 ;	//���ڶ�ʱ����PWMͨ��
	pwmOC[4].OCNumBig = 1 ;	
	pwmOC[1].OCNumBig = 2 ;	
	pwmOC[5].OCNumBig = 2 ;	
	pwmOC[2].OCNumBig = 3 ;	
	pwmOC[6].OCNumBig = 3 ;
	pwmOC[3].OCNumBig = 4 ;
	pwmOC[7].OCNumBig = 4 ;
	for(i = 0;i<8;i++)
	{
		pwmOC[i].isOPEN		= CLOSE;//�˽ṹ���Ƿ�ռ��
		pwmOC[i].pwmType	= 0;	//��������  0Ĭ�� 1-Сռ�ձ� 2-��ռ�ձ�  3-������� 4-ֱ�����
		pwmOC[i].hlSet		= 0;	//�ڶ�ʱ�������о������ŵĸߵ͵�ƽ
		pwmOC[i].hTime		= 0;	//�ߵ�ƽ����ʱ��  PWM����	(us)
		pwmOC[i].lTime		= 0;	//�͵�ƽ����ʱ��			(us)
		pwmOC[i].timeCnt	= 0;	//time����
		pwmOC[i].stepNum	= 1;	//����
		pwmOC[i].direct		= 0;	//����
		pwmOC[i].periodCalc = 0;
		pwmOC[i].pwmControl	= NULL;	//8·PWM	�ṹ��
		pwmOC[i].dcmControl	= NULL;	//ֱ����� 	�ṹ��
		pwmOC[i].dcmControl1= NULL;	//������� 	�ṹ��
		pwmOC[i].dcmControl2= NULL;	//������� 	�ṹ��
	}
}

extern TIMEOUTPUT_pwm lowPWMOC[8];	
extern uint16_t elecBuff[8][20]; 
uint16_t  elecMax = 0;	
uint8_t deviceState[8] = {0,0,0,0,0,0,0,0};
//PWM��������     ��ʱ�رյȹ���
void PWMTask(void *pdata)
{
	uint8_t  i = 0,j = 0;
	uint8_t  ledPWMFlag = 0;
	uint16_t duty;
	static u8 execTimes = 0;
	
	TIME_Init();	//���ݽṹ��ʼ��
	PWMDataInit();	//���� PWM �����ݳ�ʼ��
	
	while(1)		//��� PWM ����״̬������״̬���������������������ֹͣ��5��û��Ϣ���ظ�PWM���Ų���
	{
		ledPWMFlag = 0;
		OSTimeDlyHMSM(0,0,0,50);
		execTimes ++;
		for(i = 0;i < 8;i++)
		{
			if(pwmOC[i].isOPEN != OPEN)//��ͨ���ǹرյ�
				continue;
			ledPWMFlag = 1;
			pwmOC[i].timeWork ++;
			if(pwmOC[i].timeWork >= 60)//3sû�����·�����رմ�ͨ��
			{
				pwmOC[i].isOPEN = CLOSE;
				continue;
			}
			if(pwmOC[i].pwmType == STEP_MOTOR)//�����������
			{
				if(pwmOC[i].periodCalc > pwmOC[i].period)
					pwmOC[i].periodCalc -= pwmOC[i].hTime;
			}
		}
		for(i = 0;i < 8;i++)
		{
			if(lowPWMOC[i].isOPEN != OPEN)//��ͨ���ǿ�����
				continue;
			ledPWMFlag = 1;
			lowPWMOC[i].timeWork ++;
			if(lowPWMOC[i].timeWork > 100)//5sû�����·�����رմ�ͨ��
			{
				lowPWMOC[i].isOPEN = CLOSE;
				continue;
			}
		}
		for(i = 0;i < 8;i++)
		{
			elecMax = 0;
			for(j = 0;j<20;j++)	//�������ʮ�ε����ֵ��ȡ����
				if(elecMax< elecBuff[i][j])
					elecMax = elecBuff[i][j];
			if(i<6)//�õ��ϱ�����
			{
				if(PWMDat[i].level != 0)//ֹͣ�󲻹���
				{	//���٣�Сռ�ձ�  
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
					
				else//������
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
				
			if(PWMDat[i].numPWMOC > 9)		//��· PWM ��û�й���
			{
//				varBXWRun.elecPWM[i] = 0;	//��������	
				PWMDat[i].pinState = GPIO_ReadInputDataBit(PWMDat[i].GPIO_In,(1<<PWMDat[i].pinIn));
				varBXWRun.StatePWM[i] = GPIO_ReadInputDataBit(PWMDat[i].GPIO_In,(1<<PWMDat[i].pinIn));	
				continue;
			}//������ʱ�򣬲�����豸��û������
			if(varBXWRun.elecPWM[i] < 20)		//��������С��20
			{
				deviceState[i] ++;
				if(deviceState[i] >= 25&&execTimes >= 25)
					varBXWRun.StatePWM[i] = 1;	//������
			}else
			{
				varBXWRun.StatePWM[i] = 0;		//����
			}
		}
		if(execTimes >= 25)
		{
			execTimes = 0;
			for( i=0;i<8;i++)
				deviceState[i] = 0;
		}
			
		if(ledPWMFlag == 1)
			varOperaRun.ledFlag[5] = LEDON;	//�����PWM LED ״̬��־������
		else
			varOperaRun.ledFlag[5] = LEDOFF;//PWM LED ״̬��־������
	}
}































