#ifndef __GLOBALVAR_H__
#define __GLOBALVAR_H__

#include "includes.h"
#include "memblock.h"

typedef enum
{
	IDLE_MODE = 0,  //系统工作在空闲模式
	BXW_MODE,		//系统工作在便携王模式下
	VCI_MODE,		//系统工作在VCI模式下
	TEST_PORT_SEND_MODE,
	TEST_PORT_RECV_MODE,
}System_Mode;//系统的工作模式


typedef enum
{
	LEDOFF = 0,		//LED 场灭
	LEDON,			//LED 常亮
	LEDBLINK,		//LED 闪烁
}LED_STATE;//系统的工作模式


#pragma pack(1)             //1字节对齐

__packed typedef struct
{
	uint8_t 	isUpdate;  	//是否需要更新
	uint16_t 	softVer;  	//当前的软件版本
	uint16_t	hardVer;	//当前的硬件版
	uint16_t	newPageCnt;	//新程序2K页数量
	uint32_t	fileSize;	//新程序所占字节数
	uint16_t    newSoftCheckSum;//新程序的校验和
	uint16_t    frameCnt;	//包数
}ROM_FLASH_PARAM;

__packed typedef struct
{
	uint8_t  kFastStart;    //K线快速初始化
	uint8_t  vciKId;   		//VCI的K地址
	uint8_t  ecuKId;   		//ECU的K地址
	uint32_t kBaud;    		//K线波特率
	
	uint32_t vciCanId;		//VCI中的CANID
	uint32_t ecuCanId; 		//ECU中的CANID
	uint32_t canBaud;  		//CAN总线的波特率
	uint32_t canFilter;		//CAN总线的过滤ID
	uint32_t canMask;  		//CAN总线的掩码
	
}VCI_RunVar;

__packed typedef struct
{
	uint8_t cmdId;		//指令ID
	uint8_t powCheck;	//供电选择
	uint8_t commMode;	//通讯模式
	uint8_t terminalRes;//终端电阻
}STARTUP;	//初始化信息


__packed typedef struct
{
	uint8_t  cmdId;		//指令
	uint8_t  state;		//状态
	uint8_t  level;		//电平
	uint16_t bigDuty;	//大占空比
	uint16_t bigFre;	//大频率
	uint16_t smallDuty;	//小占空比
	uint16_t smallFre;	//小频率
}PWM_Data;
__packed typedef struct
{
	uint8_t  cmdId;		//指令
	uint8_t  state;		//状态
	uint8_t  fwd;		//电平
	uint16_t duty;	//大占空比
	uint16_t fre;	//大频率
}DCM_Data;


__packed typedef struct
{
	uint8_t	 cmdId;		//指令id
	uint32_t sendId;	//泵ID
	uint32_t receId;	//接收ID
	uint32_t mask;		//过滤CANID 的 掩码mask
	uint8_t  baud;		//波特率
	uint8_t  std_exd;	//标准帧 扩展帧
	uint8_t  data[8];	//发送的数据
	uint32_t canType;
	uint8_t  safeCH;    //0-正常的数据交互  1 - 凯龙安全算法    2 - 艾可蓝安全算法
	uint8_t  safeSeed[4];
	uint32_t flowRecId; //数据流的接收 CANID
}BXWCAN_Data;

__packed typedef struct
{
	uint8_t	 cmdId;		//指令  id
	uint32_t sendId;	//泵 ID
	uint32_t receId;	//接收 ID
	uint32_t mask;		//过滤 CANID 的 掩码 mask
	uint16_t idle;		//波特率
	uint8_t  data[8];	//发送的数据
	
	uint32_t std_exd;	//标准帧 扩展帧
	uint32_t canBaud;	//CAN 波特率
	uint8_t  canRes;	//CAN 匹配电阻
	uint8_t  canCH;		//CAN 通道
	
}VCICAN_Data;

__packed typedef struct
{
	uint8_t  frameHead;		//0x7E 帧头
	uint16_t frameLen; 		//报文长度
	uint8_t  frame1Cnt;		//一级计数
	uint8_t  frame2Cnt;		//二级计数
	uint8_t  frameType;     //报文类型
	uint8_t  serverType;    //服务类型
	uint16_t srcId;         //报文发送端逻辑地址
	uint16_t desId;         //报文接收端逻辑地址
}FrameStruct;

__packed typedef struct
{
	uint8_t  dcmDer;		//DCM 状态方向
	uint16_t  dcmCyc;		//DCM 占空比
	uint16_t dcmElec;		//DCM 电流
}DCMStruct;

__packed typedef struct
{
	uint16_t cmd1Len;		//cmd1长度
	uint8_t  cmd1Id;
	uint16_t sysVol;		//系统电压
	uint16_t sysCurr;       //系统电流
	uint8_t  stateFlag1;	//状态位
	uint8_t  stateFlag2;
	uint8_t  stateFlag3;
	uint16_t barometric;    //大气压力
	uint16_t humidity;      //湿度
	uint16_t weizhi;        //未知
	
	uint16_t cmd2Len;		//cmd2长度
	uint8_t  cmd2Id;
	uint32_t resistor[4];	//电阻1
	
	uint16_t cmd3Len;		//cmd3长度
	uint8_t  cmd3Id;			
	uint16_t Vol[6];		//电压
	
	uint16_t cmd4Len;		//cmd4长度
	uint8_t  cmd4Id;
	uint32_t fre[2];		//频率

	uint16_t cmd5Len;		//cmd5长度
	uint8_t  cmd5Id;
	uint8_t  StatePWM[9];	//PWM1 状态
	
	uint16_t cmd6Len;		//cmd6长度
	uint8_t  cmd6Id;
	uint16_t dutyCyclePWM[9];//PWM 占空比
	
	uint16_t cmd7Len;		//cmd7长度
	uint8_t  cmd7Id;
	uint16_t elecPWM[9];    //电流

	uint16_t cmd8Len;		//cmd8长度
	uint8_t  cmd8Id;
	DCMStruct dcm[3];
	
//	uint16_t cmd9Len;		//cmd9 长度
//	uint8_t  cmd9Id;
//	uint32_t revCANID;		//接收的 CANID
//	uint8_t  datCAN[8];		// CAN 数据
}BXWAllData;


__packed typedef struct
{
	System_Mode sysMode;	//系统工作模式
	STARTUP     startUp;	//初始化信息  供电、通讯、终端电阻
	
	uint16_t dat595;	//送入 595 的参数
	LED_STATE	ledFlag[6];	//LED 状态 0-灭  1-常亮 2-闪烁
	uint8_t     ledBlinTime[6];	//LED 闪烁的时间间隔
	uint8_t		ledTimeCnt[6];	//LED 时间计时
	uint8_t		ledHL[6];		//LED 高低电平，用于LED闪烁
	uint8_t		timeLed;		//LED 所在的时间
	uint8_t     wirelessMode;	//无线模式  0 - 蓝牙   1 - WIFI
	
	uint8_t 	cmdIdK;			//接收到的K   sid
	uint8_t 	cmdIdCAN;		//接收到的CAN sid
	uint32_t    receCANID;      //接收的CANID
	uint32_t    sendCANID;		//发送的CANID
	uint8_t     manyPackFlag;	//多包标志，CLOSE - 30指令无效   OPEN - 30指令有效
	uint8_t     manyPackFlagDown;//多包下发
	uint8_t     isVCIchangeCANTrue;//VCI 切换CAN指令是否有效 0 - 无效，1-有效
	
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
	ubyte FucStat;				//调用哪一个程序
	ulong UniqueChipID[4];
	uword BMIValue;

	ulong Seed;			//种子 从上位机获取
	ulong Key;			//秘钥 解析后发给上位机
	uword Model;		//模式 从上微机获取  用来作为  mask 的索引

	//extension cummins  扩展康明斯
	ulong KeySize;		//秘钥大小
	ubyte SeedEx[6];	//扩展种子
	ubyte KeyEx[100];	//扩展的秘钥
}Str_Encrypt;

#define CMD_DBG

#define UPDATAPARA_ADDR 	0x8004000     //升级参数地址   UPDATAPARA_ADDR
#define APP_ADDR        	0x8004800     //APP运行地址
#define NEWSOFT_ADDR    	0x8020000     //新程序的保存地址
#define PARAM_ADDR      	0x20000800    //程序升级参数RAM地址
#define FLASH_PAGE_SIZE    ((u16)0x800)   //一页Flash大小 2KB

/**********         全局变量         ***********************/

void GloVar_Init(void);					//全局变量初始化

#endif

