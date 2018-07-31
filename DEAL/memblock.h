

#ifndef __MEMBLOCK_H__
#define __MEMBLOCK_H__

#include "includes.h"

/**********************************************************
*                  �ڴ���س��ýṹ������
***********************************************************/
//��������ֵ
typedef	enum  
{
	OK = 0,
	FAULT,
	OVERFLOW,
	EMPTY
}NORMAL_STATE;

//ѭ�����нṹ
typedef	struct
{
	uint16_t  front;
	uint16_t  rear;
	uint16_t  length;
	uint16_t  count;
	uint8_t	  *data;
}CIR_QUEUE,*pCIR_QUEUE;

//��ջ�ṹ
typedef struct
{
	uint16_t top;
	uint16_t length;
	uint8_t* base;
}STORE, *pSTORE;

typedef struct     //�ڴ��ر���
{
	uint16_t memUsedNum1;    //�ڴ��1��ǰ����ʹ�õ�����
	uint16_t memUsedMax1;    //�ڴ��1��ʷͬһʱ��ʹ����������
	uint8_t memUsedNum2;
	uint8_t memUsedMax2;
	uint8_t memUsedNum3;
	uint8_t memUsedMax3;
	uint8_t memUsedNum4;
	uint8_t memUsedMax4;
	uint8_t memUsedNum5;
	uint8_t memUsedMax5;
	uint8_t memUsedNum6;
	uint8_t memUsedMax6;
	uint8_t memUsedNum7;
	uint8_t memUsedMax7;
}MEM_Check;


//���г�ʼ��
pCIR_QUEUE    Cir_Queue_Init(uint16_t length);
//ɾ������
NORMAL_STATE  CirQ_Delete(pCIR_QUEUE ptr_Q);
//��ն���
NORMAL_STATE  CirQ_Clear(pCIR_QUEUE ptr_Q);
//�õ����б���ֵ������
uint16_t      CirQ_GetLength(pCIR_QUEUE ptr_Q);
//�Ӷ�����ȡ��һ��ֵ
NORMAL_STATE  CirQ_Pop(pCIR_QUEUE ptr_Q,uint8_t *phead);
//�������ѹ��һ��ֵ
NORMAL_STATE  CirQ_OnePush(pCIR_QUEUE ptr_Q,uint8_t dat);
//�������ѹ��ܶ�ֵ
NORMAL_STATE  CirQ_Pushs(pCIR_QUEUE ptr_Q,const uint8_t *pdata,uint16_t length);

pSTORE       Store_Init(uint16_t length);
NORMAL_STATE Store_Delete(pSTORE S);
NORMAL_STATE Store_Clear(pSTORE S);
NORMAL_STATE Store_Pop(uint8_t *pdat,pSTORE S);
uint16_t     Store_Getlength(pSTORE S);
NORMAL_STATE Store_Getdates(uint8_t *pdate,pSTORE S,uint16_t num);

NORMAL_STATE Store_Push(pSTORE S,uint8_t dat);
NORMAL_STATE Store_Pushs(pSTORE S,uint8_t *pdate,uint16_t num);


/**********************************************************
*                  �ڴ�����������
***********************************************************/

#define MEM_16B_ROW		40
#define MEM_16B_COL		18

#define MEM_32B_ROW		20
#define MEM_32B_COL		34

#define MEM_64B_ROW		20
#define MEM_64B_COL		66

#define MEM_128B_ROW	10
#define MEM_128B_COL	130

#define MEM_256B_ROW	10
#define MEM_256B_COL	258

#define MEM_512B_ROW	2
#define MEM_512B_COL	514

#define MEM_1KB_ROW	    2
#define MEM_1KB_COL	    1026

void    *Mem_malloc(uint16_t size);
uint8_t Mem_free(void **ptr);
uint8_t MemBuf_Init(void);






#endif

