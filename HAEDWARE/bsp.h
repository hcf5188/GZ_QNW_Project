/************************************************************
*公司名称:             上海星融汽车科技
*文件名称:      bsp.h
*作者姓名:      洪春峰  QQ:1341189542
*文件内容:      各种外设、定时器等的初始化
*编写日期:       2018-4-11
*************************************************************/



#ifndef __BSP_H__
#define __BSP_H__
#include "memblock.h"
#include "sys.h"
#include "includes.h"
////////               常用数据定义              ///////////////

#define SYSTEMTIME   72000000		//系统时钟

////// ***************  函数声明   **************//////////////

void SystemBspInit(void);               //系统外设初始化
void SysTickInit(void);                 //系统滴答初始化

void RS485_UartConfig(u32 baud);        //485 通讯初始化

void ALL_GPIO_Init(void);				//配置所有相关输出的IO
void Reset_AllGPIO(void);				//复位所有的IO
/*************************   全局变量   ******************************/

#endif














