
#ifndef __APPTASK_H__
#define __APPTASK_H__

#include "includes.h"


/******************	****    任务优先级     **********************/

#define RS485_TASK_PRIO			22	//485 收发任务
#define START_TASK_PRIO			23	//系统起始任务

/**********************    任务堆栈大小    ********************/

#define START_STK_SIZE			128  //起始 任务堆栈

#define RS485_STK_SIZE			128  // 485 收发任务堆栈大小
/**********************    任务声明    ********************/

void StartTask(void *pdata);		//起始任务声明

void RS485Task(void *pdata);		//RS485 任务处理

/**********************     任务间通信声明        *********************/












#endif 












