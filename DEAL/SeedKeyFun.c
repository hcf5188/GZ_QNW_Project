#include "includes.h"

extern ulong CalculateKey(uword EngineModel, ulong Seed);
extern ulong CalculateKeyEx(uword EngineModel, ubyte *Seed, ubyte *Key);
extern ulong CalcKeyForWeichai(uword EngineModel, ulong Seed);
extern ulong CalculateCumminsECUKey(uword EngineModel, ubyte *Seed, ubyte *Key);
extern ulong CalculateCumminsECUKey2(uword EngineModel, ubyte *Seed, ubyte *Key);
extern ulong CalculateCumminsTickCountDecrypt(uword EngineModel, ubyte *TickCount, ubyte *Key);
extern ulong CalculateCumminsACM(uword EngineModel, ubyte *Seed, ubyte *Key);
extern ulong CalculateWfldACM(uword EngineModel, ubyte *Seed, ubyte *Key);
extern ulong CalculateMaPaiECU(uword EngineModel, ubyte *Seed, ubyte *Key);
extern ulong CalculateAikelanECU(uword EngineModel, ubyte *Seed, ubyte *Key);
extern ulong CalculateWCDCU(uword EngineModel, ubyte *Seed, ubyte *Key);
extern ulong CalculateKeyG5(uword EngineModel, ulong Seed);

extern Str_Encrypt Eyt;  				//过安全算法相关结构体

//得到种子、索引出mask,为计算Key做准备
void RcvSeed_CatchMask(FrameStruct* ptrFrame)
{
	uint8_t * ptrData = NULL;
	uint8_t cmdId = 0;
	uint16_t datLen = 0;
	strL2B L2B;
	strW2B W2B;
	
	ptrData = (uint8_t *)ptrFrame;
	datLen = ptrData[12];
	datLen = (datLen<<8) + ptrData[11];
	cmdId  = ptrData[13];
	
	switch(cmdId)
	{
		case 0x01:	break;
		case 0x02:	break;
		
		case 0x11:	
			W2B.B[0] = ptrData[14];
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			L2B.B[0] = ptrData[16];
			L2B.B[1] = ptrData[17];
			L2B.B[2] = ptrData[18];
			L2B.B[3] = ptrData[19];
			Eyt.Seed = L2B.L;		//得到种子
			Eyt.Key = CalculateKey(Eyt.Model,Eyt.Seed);
			ptrData = Mem_malloc(10);
			ptrData[0] = 5;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.Key,4);
			DateToSend(ptrData,7,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x12:	
			W2B.B[0] = ptrData[14];
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			memcpy(Eyt.SeedEx, &ptrData[16], sizeof(Eyt.SeedEx));
			Eyt.KeySize = CalculateKeyEx(Eyt.Model, Eyt.SeedEx, Eyt.KeyEx );
			ptrData = Mem_malloc(Eyt.KeySize + 3);
			ptrData[0] = Eyt.KeySize + 1;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.KeyEx,Eyt.KeySize);
			DateToSend(ptrData,Eyt.KeySize + 3,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x13:	
			W2B.B[0] = ptrData[14];
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			L2B.B[0] = ptrData[16];
			L2B.B[1] = ptrData[17];
			L2B.B[2] = ptrData[18];
			L2B.B[3] = ptrData[19];
			Eyt.Seed = L2B.L;		//得到种子
			Eyt.Key = CalcKeyForWeichai(Eyt.Model, Eyt.Seed );
			ptrData = Mem_malloc(10);
			ptrData[0] = 5;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.Key,4);
			DateToSend(ptrData,7,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x14:	
			W2B.B[0] = ptrData[14];
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			memcpy(Eyt.SeedEx, &ptrData[16], sizeof(Eyt.SeedEx));
			Eyt.KeySize = CalculateCumminsECUKey(Eyt.Model, Eyt.SeedEx, Eyt.KeyEx );
			ptrData = Mem_malloc(Eyt.KeySize + 3);
			ptrData[0] = Eyt.KeySize + 1;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.KeyEx,Eyt.KeySize);
			DateToSend(ptrData,Eyt.KeySize + 3,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x15:	
			W2B.B[0] = ptrData[14];
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			memcpy(Eyt.SeedEx, &ptrData[16], sizeof(Eyt.SeedEx));
			Eyt.KeySize = CalculateCumminsECUKey2(Eyt.Model, Eyt.SeedEx, Eyt.KeyEx );
			ptrData = Mem_malloc(Eyt.KeySize + 3);
			ptrData[0] = Eyt.KeySize + 1;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.KeyEx,Eyt.KeySize);
			DateToSend(ptrData,Eyt.KeySize + 3,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x16:	
			W2B.B[0] = ptrData[14];
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			memcpy(Eyt.SeedEx, &ptrData[16], sizeof(Eyt.SeedEx));
			Eyt.KeySize = CalculateCumminsTickCountDecrypt(Eyt.Model, Eyt.SeedEx, Eyt.KeyEx );
			ptrData = Mem_malloc(Eyt.KeySize + 3);
			ptrData[0] = Eyt.KeySize + 1;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.KeyEx,Eyt.KeySize);
			DateToSend(ptrData,Eyt.KeySize + 3,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x17:	
			W2B.B[0] = ptrData[14];
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			memcpy(Eyt.SeedEx, &ptrData[16], sizeof(Eyt.SeedEx));
			Eyt.KeySize = CalculateCumminsACM(Eyt.Model, Eyt.SeedEx, Eyt.KeyEx );
			ptrData = Mem_malloc(Eyt.KeySize + 3);
			ptrData[0] = Eyt.KeySize + 1;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.KeyEx,Eyt.KeySize);
			DateToSend(ptrData,Eyt.KeySize + 3,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x18:	
			W2B.B[0] = ptrData[14];
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			memcpy(Eyt.SeedEx, &ptrData[16], sizeof(Eyt.SeedEx));
			Eyt.KeySize = CalculateWfldACM(Eyt.Model, Eyt.SeedEx, Eyt.KeyEx );
			ptrData = Mem_malloc(Eyt.KeySize + 3);
			ptrData[0] = Eyt.KeySize + 1;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.KeyEx,Eyt.KeySize);
			DateToSend(ptrData,Eyt.KeySize + 3,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x19:		
			W2B.B[0] = ptrData[14];
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			memcpy(Eyt.SeedEx, &ptrData[16], sizeof(Eyt.SeedEx));
			Eyt.KeySize = CalculateMaPaiECU(Eyt.Model, Eyt.SeedEx, Eyt.KeyEx );
			ptrData = Mem_malloc(Eyt.KeySize + 3);
			ptrData[0] = Eyt.KeySize + 1;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.KeyEx,Eyt.KeySize);
			DateToSend(ptrData,Eyt.KeySize + 3,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x1A:	
			W2B.B[0] = ptrData[14];//同样适用于 后处理 艾可蓝 安全算法
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			memcpy(Eyt.SeedEx, &ptrData[16], sizeof(Eyt.SeedEx));
			Eyt.KeySize = CalculateAikelanECU(Eyt.Model, Eyt.SeedEx, Eyt.KeyEx );
			ptrData = Mem_malloc(Eyt.KeySize + 3);
			ptrData[0] = Eyt.KeySize + 1;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.KeyEx,Eyt.KeySize);
			DateToSend(ptrData,Eyt.KeySize + 3,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x1B:	
			W2B.B[0] = ptrData[14];
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			memcpy(Eyt.SeedEx, &ptrData[16], sizeof(Eyt.SeedEx));
			Eyt.KeySize = CalculateWCDCU(Eyt.Model, Eyt.SeedEx, Eyt.KeyEx );
			ptrData = Mem_malloc(Eyt.KeySize + 3);
			ptrData[0] = Eyt.KeySize + 1;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.KeyEx,Eyt.KeySize);
			DateToSend(ptrData,Eyt.KeySize + 3,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x1C:
			W2B.B[0] = ptrData[14];
			W2B.B[1] = ptrData[15];
			Eyt.Model = W2B.W;		//得到模式
			L2B.B[0] = ptrData[16];
			L2B.B[1] = ptrData[17];
			L2B.B[2] = ptrData[18];
			L2B.B[3] = ptrData[19];
			Eyt.Seed = L2B.L;		//得到种子
			Eyt.Key = CalculateKeyG5(Eyt.Model,Eyt.Seed);
			ptrData = Mem_malloc(10);
			ptrData[0] = 5;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.Key,4);
			DateToSend(ptrData,7,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		case 0x1D:	
			Eyt.Key = 0x20180521;
			ptrData = Mem_malloc(10);
			ptrData[0] = 5;
			ptrData[1] = 0x00;
			ptrData[2] = cmdId + 0x40;
			memcpy(&ptrData[3],&Eyt.Key,4);
			DateToSend(ptrData,7,0x00,0x0C);//回复上位机Key
			Mem_free((void **)&ptrData);
			break;
		
		default: break;
	}
}
















