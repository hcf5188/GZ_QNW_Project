#ifndef __GLOBALVAR_H__
#define __GLOBALVAR_H__

#include "includes.h"
#include "memblock.h"

typedef enum
{
	IDLE_MODE = 0,  //ϵͳ�����ڿ���ģʽ
	BXW_MODE,		//ϵͳ�����ڱ�Я��ģʽ��
	VCI_MODE,		//ϵͳ������VCIģʽ��
	TEST_PORT_SEND_MODE,
	TEST_PORT_RECV_MODE,
}System_Mode;//ϵͳ�Ĺ���ģʽ


typedef enum
{
	LEDOFF = 0,		//LED ����
	LEDON,			//LED ����
	LEDBLINK,		//LED ��˸
}LED_STATE;//ϵͳ�Ĺ���ģʽ


#pragma pack(1)             //1�ֽڶ���

__packed typedef struct
{
	uint8_t 	isUpdate;  	//�Ƿ���Ҫ����
	uint16_t 	softVer;  	//��ǰ������汾
	uint16_t	hardVer;	//��ǰ��Ӳ����
	uint16_t	newPageCnt;	//�³���2Kҳ����
	uint32_t	fileSize;	//�³�����ռ�ֽ���
	uint16_t    newSoftCheckSum;//�³����У���
	uint16_t    frameCnt;	//����
}ROM_FLASH_PARAM;

__packed typedef struct
{
	uint8_t  kFastStart;    //K�߿��ٳ�ʼ��
	uint8_t  vciKId;   		//VCI��K��ַ
	uint8_t  ecuKId;   		//ECU��K��ַ
	uint32_t kBaud;    		//K�߲�����
	
	uint32_t vciCanId;		//VCI�е�CANID
	uint32_t ecuCanId; 		//ECU�е�CANID
	uint32_t canBaud;  		//CAN���ߵĲ�����
	uint32_t canFilter;		//CAN���ߵĹ���ID
	uint32_t canMask;  		//CAN���ߵ�����
	
}VCI_RunVar;

__packed typedef struct
{
	uint8_t cmdId;		//ָ��ID
	uint8_t powCheck;	//����ѡ��
	uint8_t commMode;	//ͨѶģʽ
	uint8_t terminalRes;//�ն˵���
}STARTUP;	//��ʼ����Ϣ


__packed typedef struct
{
	uint8_t  cmdId;		//ָ��
	uint8_t  state;		//״̬
	uint8_t  level;		//��ƽ
	uint16_t bigDuty;	//��ռ�ձ�
	uint16_t bigFre;	//��Ƶ��
	uint16_t smallDuty;	//Сռ�ձ�
	uint16_t smallFre;	//СƵ��
}PWM_Data;
__packed typedef struct
{
	uint8_t  cmdId;		//ָ��
	uint8_t  state;		//״̬
	uint8_t  fwd;		//��ƽ
	uint16_t duty;	//��ռ�ձ�
	uint16_t fre;	//��Ƶ��
}DCM_Data;


__packed typedef struct
{
	uint8_t	 cmdId;		//ָ��id
	uint32_t sendId;	//��ID
	uint32_t receId;	//����ID
	uint32_t mask;		//����CANID �� ����mask
	uint8_t  baud;		//������
	uint8_t  std_exd;	//��׼֡ ��չ֡
	uint8_t  data[8];	//���͵�����
	uint32_t canType;
	uint8_t  safeCH;    //0-���������ݽ���  1 - ������ȫ�㷨    2 - ��������ȫ�㷨
	uint8_t  safeSeed[4];
	uint32_t flowRecId; //�������Ľ��� CANID
}BXWCAN_Data;

__packed typedef struct
{
	uint8_t	 cmdId;		//ָ��  id
	uint32_t sendId;	//�� ID
	uint32_t receId;	//���� ID
	uint32_t mask;		//���� CANID �� ���� mask
	uint16_t idle;		//������
	uint8_t  data[8];	//���͵�����
	
	uint32_t std_exd;	//��׼֡ ��չ֡
	uint32_t canBaud;	//CAN ������
	uint8_t  canRes;	//CAN ƥ�����
	uint8_t  canCH;		//CAN ͨ��
	
}VCICAN_Data;

__packed typedef struct
{
	uint8_t  frameHead;		//0x7E ֡ͷ
	uint16_t frameLen; 		//���ĳ���
	uint8_t  frame1Cnt;		//һ������
	uint8_t  frame2Cnt;		//��������
	uint8_t  frameType;     //��������
	uint8_t  serverType;    //��������
	uint16_t srcId;         //���ķ��Ͷ��߼���ַ
	uint16_t desId;         //���Ľ��ն��߼���ַ
}FrameStruct;

__packed typedef struct
{
	uint8_t  dcmDer;		//DCM ״̬����
	uint16_t  dcmCyc;		//DCM ռ�ձ�
	uint16_t dcmElec;		//DCM ����
}DCMStruct;

__packed typedef struct
{
	uint16_t cmd1Len;		//cmd1����
	uint8_t  cmd1Id;
	uint16_t sysVol;		//ϵͳ��ѹ
	uint16_t sysCurr;       //ϵͳ����
	uint8_t  stateFlag1;	//״̬λ
	uint8_t  stateFlag2;
	uint8_t  stateFlag3;
	uint16_t barometric;    //����ѹ��
	uint16_t humidity;      //ʪ��
	uint16_t weizhi;        //δ֪
	
	uint16_t cmd2Len;		//cmd2����
	uint8_t  cmd2Id;
	uint32_t resistor[4];	//����1
	
	uint16_t cmd3Len;		//cmd3����
	uint8_t  cmd3Id;			
	uint16_t Vol[6];		//��ѹ
	
	uint16_t cmd4Len;		//cmd4����
	uint8_t  cmd4Id;
	uint32_t fre[2];		//Ƶ��

	uint16_t cmd5Len;		//cmd5����
	uint8_t  cmd5Id;
	uint8_t  StatePWM[9];	//PWM1 ״̬
	
	uint16_t cmd6Len;		//cmd6����
	uint8_t  cmd6Id;
	uint16_t dutyCyclePWM[9];//PWM ռ�ձ�
	
	uint16_t cmd7Len;		//cmd7����
	uint8_t  cmd7Id;
	uint16_t elecPWM[9];    //����

	uint16_t cmd8Len;		//cmd8����
	uint8_t  cmd8Id;
	DCMStruct dcm[3];
	
//	uint16_t cmd9Len;		//cmd9 ����
//	uint8_t  cmd9Id;
//	uint32_t revCANID;		//���յ� CANID
//	uint8_t  datCAN[8];		// CAN ����
}BXWAllData;


__packed typedef struct
{
	System_Mode sysMode;	//ϵͳ����ģʽ
	STARTUP     startUp;	//��ʼ����Ϣ  ���硢ͨѶ���ն˵���
	
	uint16_t dat595;	//���� 595 �Ĳ���
	LED_STATE	ledFlag[6];	//LED ״̬ 0-��  1-���� 2-��˸
	uint8_t     ledBlinTime[6];	//LED ��˸��ʱ����
	uint8_t		ledTimeCnt[6];	//LED ʱ���ʱ
	uint8_t		ledHL[6];		//LED �ߵ͵�ƽ������LED��˸
	uint8_t		timeLed;		//LED ���ڵ�ʱ��
	uint8_t     wirelessMode;	//����ģʽ  0 - ����   1 - WIFI
	
	uint8_t 	cmdIdK;			//���յ���K   sid
	uint8_t 	cmdIdCAN;		//���յ���CAN sid
	uint32_t    receCANID;      //���յ�CANID
	uint32_t    sendCANID;		//���͵�CANID
	uint8_t     manyPackFlag;	//�����־��CLOSE - 30ָ����Ч   OPEN - 30ָ����Ч
	uint8_t     manyPackFlagDown;//����·�
	uint8_t     isVCIchangeCANTrue;//VCI �л�CANָ���Ƿ���Ч 0 - ��Ч��1-��Ч
	
}SYSTEM_Var;


#pragma pack () 
#define	ubyte uint8_t
#define uword uint16_t
#define ulong uint32_t
typedef union strL2B
{
	ulong L;
	ubyte B[4];
}strL2B;
typedef union strW2B
{
	uword W;
	ubyte B[2];
}strW2B;
typedef struct 
{
	ubyte FucStat;				//������һ������
	ulong UniqueChipID[4];
	uword BMIValue;

	ulong Seed;			//���� ����λ����ȡ
	ulong Key;			//��Կ �����󷢸���λ��
	uword Model;		//ģʽ ����΢����ȡ  ������Ϊ  mask ������

	//extension cummins  ��չ����˹
	ulong KeySize;		//��Կ��С
	ubyte SeedEx[6];	//��չ����
	ubyte KeyEx[100];	//��չ����Կ
}Str_Encrypt;

#define CMD_DBG

#define UPDATAPARA_ADDR 	0x8004000     //����������ַ   UPDATAPARA_ADDR
#define APP_ADDR        	0x8004800     //APP���е�ַ
#define NEWSOFT_ADDR    	0x8020000     //�³���ı����ַ
#define PARAM_ADDR      	0x20000800    //������������RAM��ַ
#define FLASH_PAGE_SIZE    ((u16)0x800)   //һҳFlash��С 2KB

/**********         ȫ�ֱ���         ***********************/

void GloVar_Init(void);					//ȫ�ֱ�����ʼ��

#endif

