
#ifndef __APPTASK_H__
#define __APPTASK_H__

#include "includes.h"


/******************	****    �������ȼ�     **********************/

#define RS485_TASK_PRIO			22	//485 �շ�����
#define START_TASK_PRIO			23	//ϵͳ��ʼ����

/**********************    �����ջ��С    ********************/

#define START_STK_SIZE			128  //��ʼ �����ջ

#define RS485_STK_SIZE			128  // 485 �շ������ջ��С
/**********************    ��������    ********************/

void StartTask(void *pdata);		//��ʼ��������

void RS485Task(void *pdata);		//RS485 ������

/**********************     �����ͨ������        *********************/












#endif 












