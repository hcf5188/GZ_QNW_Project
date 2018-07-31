#include "bsp.h"


uint16_t datADC[ADBUFFSIZE] = {0,};

void TIM2_Configuration(void)
{ 
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
	TIM_OCInitTypeDef TIM_OCInitStructure; 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE); 

	TIM_TimeBaseStructure.TIM_Period 	= 999;		//����ֵ  10ms����һ��ADת��
	TIM_TimeBaseStructure.TIM_Prescaler = 71;		//��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;	//ģʽ
	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;//���ϼ���
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
	DMA_InitTypeDef  DMA_InitStructure;				// ע��ADCΪ12λģ��ת������ֻ��ADCConvertedValue�ĵ�12λ��Ч
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//ʹ��DMAʱ��

	DMA_DeInit(DMA1_Channel1);						//����DMA1�ĵ�һͨ�� 
	DMA_InitStructure.DMA_PeripheralBaseAddr =  ADC1_DR_Address;	//DMA��Ӧ���������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&datADC[0];			//�ڴ�洢����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;				//DMA��ת��ģʽΪSRCģʽ����������Ƶ��ڴ�
	DMA_InitStructure.DMA_BufferSize = ADBUFFSIZE;					//DMA�����С��10��
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//����һ�����ݺ��豸��ַ��ֹ����	 
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;			//�رս���һ�����ݺ�Ŀ���ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//�����������ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//DMA�������ݳߴ磬HalfWord����Ϊ16λ			
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;					//ѭ��ת��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;			//DMA���ȼ���
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;					//M2Mģʽ����
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC, ENABLE);//ʹ�ܴ�������ж�
	
	DMA_Cmd(DMA1_Channel1, ENABLE);	

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); 
}
void ADC_GPIO_Configuration(void)        //ADC IO����
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
	ADC_GPIO_Configuration();			//AD IO��������
	TIM2_Configuration(); 				//��ʱ�� 2 ����
	ADC_DMA_Config();					//ADC_DMA����
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//������ת��ģʽ ADC_DUALMOD[3:0]=0000;
	ADC_InitStructure.ADC_ScanConvMode =ENABLE;		  //��ɨ��ģʽ ��ʮ��ͨ��
	ADC_InitStructure.ADC_ContinuousConvMode =DISABLE;//�ر�����ת��ģʽ ����ֻҪ����һ�Σ�
	//������ת���ͻ�����ͣЪ������CONT��0����������һ���Ժ��ADC���Ͳ�����TIM2_CC2��������	
	
	ADC_InitStructure.ADC_ExternalTrigConv =ADC_ExternalTrigConv_T2_CC2;//���ת��ģʽ
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				//���뷽ʽ,ADCΪ12λ�У��Ҷ��뷽ʽ ADC_ALIGN=0;
	ADC_InitStructure.ADC_NbrOfChannel = ADBUFFSIZE;					//����ͨ������1��  ADC_SQR1[23:20]=0000;
																		//ADC_SQR1[23:20] ����ͨ����Ŀ��ѡ��
	ADC_Init(ADC1, &ADC_InitStructure);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);		//����ʱ��(12MHz),��RCC���滹Ӧ����APB2=AHBʱ��72MHz
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1 ,ADC_SampleTime_13Cycles5);//����5
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2 ,ADC_SampleTime_13Cycles5);//����6
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3 ,ADC_SampleTime_13Cycles5);//����7
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4 ,ADC_SampleTime_13Cycles5);//����8
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 5 ,ADC_SampleTime_13Cycles5);//Uin_1  ��ѹ
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 6 ,ADC_SampleTime_13Cycles5);//Uin_2  ��ѹ
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10,7 ,ADC_SampleTime_13Cycles5);//����1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11,8 ,ADC_SampleTime_13Cycles5);//����2
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12,9 ,ADC_SampleTime_13Cycles5);//����3
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13,10,ADC_SampleTime_13Cycles5);//����4
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14,11,ADC_SampleTime_13Cycles5);//ϵͳ��ѹ
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15,12,ADC_SampleTime_13Cycles5);//ϵͳ����
	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);	//�����ⲿ����ģʽʹ�ܣ�������ⲿ����ʵ�����������ADCģ����ⲿ
	ADC_DMACmd(ADC1, ENABLE);   
	ADC_Cmd(ADC1, ENABLE);					//ADC���ʹ��  ADC_ADON=1
	ADC_ResetCalibration(ADC1);				//����У׼
	while(ADC_GetResetCalibrationStatus(ADC1));//�ȴ�����У׼���
	ADC_StartCalibration(ADC1);				//��ʼУ׼	ADC_RSTCAL=1; ��ʼ��У׼�Ĵ���
	while(ADC_GetCalibrationStatus(ADC1));	//�ȴ�У׼���  ADC_CAL=0; 
	TIM_Cmd(TIM2, ENABLE);					//�����򿪶�ʱ��ʹ��
	DMA_Cmd(DMA1_Channel1,ENABLE);			//ʹ��DMA  
}

uint8_t ad_flag = 0;
double sysCURR = 0;

uint8_t * ptrADC;
extern OS_EVENT * ADCResultQ;				//�洢ADCת���Ľ��
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
	
	OSIntExit();  //�жϷ��������ϵͳ�����������
}













