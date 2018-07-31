/*
 * SeedKeyAlgrithem.c
 *
 *  Created on: 2016-11-16
 *      Author: Mahome
 */
//#include "Header.h"
#include "includes.h"

//重汽天然气
int rotateLeft(int a, unsigned int n)//循环左移n位
{
	n %= 32;
	if (n == 0)return a;
	return (a << n) | ((a & 0xFFFFFFFF) >> (32 - n));
}

//重汽天然气
int sub_3F57(ubyte *a1, ubyte *a2, ulong a3)
{
	unsigned int v4; // [esp+4h] [ebp-10h]

	v4 = a3 ^ ((((((a1[3] << 8) | a1[4]) << 8) | a1[5]) << 8) | a1[6]);
	if (a2)
	{
		*a2 = (v4 ^ rotateLeft(v4, 27)) >> 24;
		a2[1] = ((v4 ^ rotateLeft(v4, 27)) & 0xFF0000) >> 16;
		a2[2] = (unsigned short)((v4 ^ (v4 >> 5)) & 0xFF00) >> 8;
		a2[3] = (v4 >> 5) ^ v4;
	}
	return 1;
}

//潍柴
int Seedkey_WC_DCU(ubyte *a1, ulong a2)
{
	signed int i; // [sp+8h] [bp-Ch]
	signed int j; // [sp+8h] [bp-Ch]
	int v5;       // [sp+Ch] [bp-8h]


	v5 = (a1[0] << 24) + (a1[1] << 16) + (a1[2] << 8) + a1[3];
	for (i = 0; i <= 34; i++)
	{
		if (v5 >= 0)
			v5 *= 2;
		else
			v5 = 2 * v5 ^ 0x18DA00F1;
	}
	for (j = 3; j >= 0; j--)
	{
		*(ubyte *)(a2 + j) = v5;
		v5 = (int)(v5 >> 8);
	}
	return 1;
}


unsigned int dword_10024000[4] = {0};

//潍柴锐动力算法
signed int  WCRDL_SeedTokey(ubyte* a1, int a2, ubyte* a3, unsigned int *a4)
{
	unsigned int v5; 	// [sp+0h] [bp-18h]@1
	int v6; 			// [sp+8h] [bp-10h]@2
	int v7; 			// [sp+Ch] [bp-Ch]@1
	unsigned int i; 	// [sp+14h] [bp-4h]@3

	v5 = *(ubyte *)(a1 + 4) + (*(ubyte *)(a1 + 3) << 8) + (*(ubyte *)(a1 + 2) << 16) + (*(ubyte *)(a1 + 1) << 24);
	v7 = dword_10024000[((signed int)*(ubyte *)(a1 + 4) >> 4) & 3];
	if (v7)
	{
		for (i = 0; i < (*(ubyte *)(a1 + 2) & 0xFu) && (v5 & 7) != 5; ++i)
		{
			if (((signed int)*(ubyte *)(a1 + 4) >> 3) & 1)
				v5 = (v5 << 31) | (v5 >> 1);
			else
				v5 = (v5 >> 31) | 2 * v5;
		}
		v6 = v7 ^ v5;
	}
	else
	{
		v6 = *(ubyte *)(a1 + 4) + (*(ubyte *)(a1 + 3) << 8) + (*(ubyte *)(a1 + 2) << 16) + (*(ubyte *)(a1 + 1) << 24);
	}
	*(ubyte *)(a3 + 3) = v6;
	*(ubyte *)(a3 + 2) = (unsigned short)(v6 & 0xFF00) >> 8;
	*(ubyte *)(a3 + 1) = (v6 & 0xFF0000u) >> 16;
	*(ubyte *)a3 = (v6 & 0xFF000000) >> 24;
	*a4 = a2 - 1;
	return 1;
}

ulong CalculateWCDCU(uword EngineModel, ubyte *Seed, ubyte *Key) {
	if(Seed[0] == 1 && Seed[1] == 1) ////潍柴
	{
		ubyte szkey[4] = { 0 };
		Seedkey_WC_DCU(Seed+2,(ulong)szkey);
		memcpy(Key, szkey, 4);
		return 4;
	}
	else if(Seed[0] == 1 && Seed[1] == 2) //重汽天然气
	{
		ubyte szkey[4] = { 0 };
		ubyte szSeed[7] = {0,0,0,Seed[2],Seed[3],Seed[4],Seed[5]};
		sub_3F57(szSeed,szkey,0x339C2AB6);
		memcpy(Key, szkey, 4);
		return 4;
	}
	else if(Seed[0] == 1 && Seed[1] == 3) //潍柴锐动力EDC17C63
		{
			dword_10024000[0] = 0xEDC17C63;
			dword_10024000[1] = 0xEDC17C63;
			dword_10024000[2] = 0xEDC17C63;
			dword_10024000[3] = 0xEDC17C63;
			ubyte szkey[4] = { 0 };
			unsigned int ikeyLen = 0;
			ubyte szSeed[5] = {0x9,Seed[2],Seed[3],Seed[4],Seed[5]};
			WCRDL_SeedTokey(szSeed,5,szkey,&ikeyLen);
			memcpy(Key, szkey, 4);
			return 4;
		}
	else if(Seed[0] == 1 && Seed[1] == 4) //潍柴锐动力EDC17C81
			{
				dword_10024000[0] = 0xEDC17C81;
				dword_10024000[1] = 0xEDC17C81;
				dword_10024000[2] = 0xEDC17C81;
				dword_10024000[3] = 0xEDC17C81;
				ubyte szkey[4] = { 0 };
				unsigned int ikeyLen = 0;
				ubyte szSeed[5] = {0x9,Seed[2],Seed[3],Seed[4],Seed[5]};
				WCRDL_SeedTokey(szSeed,5,szkey,&ikeyLen);
				memcpy(Key, szkey, 4);
				return 4;
			}
	return 0;
}

//艾可蓝ECU后处理算法
ulong CalculateAikelanSeed2Key(ulong Seed)
{
	ulong eax = Seed;
	eax += eax;
	eax += eax;
	eax += eax;
	eax |= 0x3D;
	return eax;
}

ulong CalculateAikelanECU(uword EngineModel, ubyte *Seed, ubyte *Key) {
	ubyte szkey[4] = { 0 };
	ulong uSeed= (Seed[0]<<24)+(Seed[1]<<16)+(Seed[2]<<8)+Seed[3];
	ulong ukey = CalculateAikelanSeed2Key(uSeed);
	szkey[0] = (ubyte)(ukey>>24);
	szkey[1] = (ubyte)(ukey>>16);
	szkey[2] = (ubyte)(ukey>>8);
	szkey[3] = (ubyte)ukey;
	memcpy(Key, szkey, 4);
	return 4;
}


//大陆马牌ECU刷写算法
long CalculateMaPaiSeed2Key(ulong a1, ubyte* byte_100320C3)
{
	ulong v1; // esi
	char v2; // dl
	ulong v3; // ecx
	ulong v4; // ebp
	long v5; // edi
	long v6; // edx
	ulong v7; // ebx
	long v8; // eax
	long v9; // edi
	ulong v10; // edx
	long result; // eax
	long v12; // esi
	ulong v13; // edx
	long v14[4]; // [esp+10h] [ebp-10h]


	v1 = 0x2EBA751F;
	v14[0] = 0x2EBA751F;
	v14[1] = 0x7D6E390F;
	v14[2] = 0x6E2A6272;
	v14[3] = 0xA7B0C413;
	v2 = 32;
	v3 = 0;
	do
	{
		a1 += (v3 + *(v14 + (v3 & 3))) ^ (v1 + (16 * v1 ^ (v1 >> 5)));
		v3 -= 0x61C88647;
		v2--;
		v1 += (v3 + *(v14 + ((v3 >> 11) & 3))) ^ (a1 + (16 * a1 ^ (a1 >> 5)));
	} while (v2 > 0);
	v4 = (uint16_t)(v1 ^ a1) ^ ((v1 ^ a1) >> 16);
	v5 = 0;
	v6 = 4;
	do
	{
		v7 = a1 >> 8 * (3 - v5++);
		v6--;
		byte_100320C3[v5] = v7;
	} while (v6>0);
	v8 = 4;
	v9 = 4;
	do
	{
		v10 = v1 >> 8 * (7 - v8++);
		v9--;
		byte_100320C3[v8] = v10;
	} while (v9>0);
	result = 8;
	v12 = 2;
	do
	{
		v13 = v4 >> 8 * (9 - result++);
		v12--;
		byte_100320C3[result] = v13;
	} while (v12>0);
	return result;
}

ulong CalculateMaPaiECU(uword EngineModel, ubyte *Seed, ubyte *Key) {
	ubyte szkey[11] = { 0 };
	ulong uSeed= (Seed[0]<<8)+Seed[1];
	CalculateMaPaiSeed2Key(uSeed,szkey);
	memcpy(Key, szkey+1, 10);
	return 10;
}


//威孚力达后处理
ulong calcWfld(ulong Seed)
{
		ulong c = 0x11;
		ulong d = 0;
		ulong a = Seed;
		while (c>0)
		{
			if (a<0x8000)
			{
				d = a + a;
				a = (uint16_t)d;
			}
			else
			{
				a += a;
				a ^= 0x3267;
				a = (uint16_t)a;
			}
			c--;
		}
		return a;
}

ulong CalculateWfldACM(uword EngineModel, ubyte *Seed, ubyte *Key) {
	ubyte szkey[3] = { 0 };
	ulong uSeed= (Seed[0]<<8)+Seed[1];
	ulong key = calcWfld(uSeed);
	szkey[0] = (ubyte)(key>>8);
	szkey[1] = (ubyte)key;
	memcpy(Key, szkey, 3);
	return 3;
}

//康明斯后处理算法
ulong  sub_F2F1010(ulong a1, ulong a2)
{
	ulong v2; // esi@1
	ulong result; // eax@1
	ulong v4; // ecx@1
	ulong v5; // edx@1
	int v6; // edi@1

	ulong v7;
	ulong v8;
	ulong v9;
	ulong v10;
	ulong v11;
	ulong v12;

	v2 = a1;
	result = *(ulong *)a2;
	v4 = *(ulong *)(a2 + 4);
	v5 = 0;
	v6 = 32;
	do
	{
		v5 -= 1640531527;
		v7 = (v5 + v4);
		v8 = (*(ulong *)v2 + 16 * v4);
		v9 = (*(ulong *)(v2 + 4) + (v4 >> 5));
		result += v7^v8^v9;
		//result += (v5 + v4) ^ (*(ulong *)v2 + 16 * v4) ^ (*(ulong *)(v2 + 4) + (v4 >> 5));
		v10 = (v5 + result);
		v11 = (*(ulong *)(v2 + 8) + 16 * result);
		v12 = (*(ulong *)(v2 + 12) + (result >> 5));
		v4 += v10^v11^v12;
		//v4 += (v5 + result) ^ (*(ulong *)(v2 + 8) + 16 * result) ^ (*(ulong *)(v2 + 12) + (result >> 5));
		v6--;
	} while (v6>0);
	*(ulong *)a2 = result;
	*(ulong *)(a2 + 4) = v4;
	return result;
}

//a1=seed a2 = 4 a3=2 a4=key
ulong GenerateKeyEx(ubyte *a1, ulong a2, ulong a3, ubyte* a4)
{
	ulong v5; // ecx@7
	ulong v6; // ecx@7
	ulong v7; // ecx@7
	ulong v8; // ecx@7
	ulong v9[2]; // [sp+0h] [bp-20h]@1
	//unsigned int v10; // [sp+4h] [bp-1Ch]@1
	long v11[2]; // [sp+8h] [bp-18h]@1
	//void *v12; // [sp+Ch] [bp-14h]@1
	long v13[4]; // [sp+10h] [bp-10h]@4
	//int v14; // [sp+14h] [bp-Ch]@4
	//int v15; // [sp+18h] [bp-8h]@4
	//int v16; // [sp+1Ch] [bp-4h]@4
	long v17; // [sp+28h] [bp+8h]@3
	long v18;
	long v19;
	long v20;
	long v21;

	if (a2 != 4)
		return 3;
	//0xc3791801;
	//v18 = (ubyte)(a1[3]) | (((ubyte)(a1[2]) | (((ubyte)a1[1] | ((ubyte)a1[0] << 8)) << 8)) << 8);
	v17 = (a1[0]<<24);
	v18 = (a1[1]<<16);
	v19 = (a1[2]<<8);
	v20 = (a1[3]);
	v21 = v17+v18+v19+v20;
	v11[0] = v21&0xffffffff;
	v11[1] = 0x260000;
	v9[0] = 0;
	v9[1] = 0;
	if (a3 == 1)
	{
		v13[0] = 2100885492;
		v13[1] = 949146530;
		v13[2] = 1383541836;
		v13[3] = 897274979;
	LABEL_7:
		sub_F2F1010((ulong)&v13, (ulong)&v11[0]);
		sub_F2F1010((ulong)&v13, (ulong)&v9[0]);
		v5 = v9[1];
		*(ubyte *)(a4 + 14) = v9[1]>>8;
		*(ubyte *)(a4 + 15) = v5;
		*(ubyte *)(a4 + 13) = v5 >> 16;
		*(ubyte *)(a4 + 12) = v5>>24;
		v6 = v9[0];
		*(ubyte *)(a4 + 10) = v9[0]>>8;
		*(ubyte *)(a4 + 11) = v6;
		*(ubyte *)(a4 + 9) = v6 >> 16;
		*(ubyte *)(a4 + 8) = v6>>24;
		v7 = (ulong)v11[1];
		*(ubyte *)(a4 + 6) = v7 >> 8;
		*(ubyte *)(a4 + 7) = v7;
		*(ubyte *)(a4 + 5) = v7 >> 16;
		*(ubyte *)(a4 + 4) = v7>>24;
		v8 = v11[0];
		*(ubyte *)(a4 + 2) = v8>>8;
		*(ubyte *)(a4 + 3) = v8;
		*(ubyte *)(a4 + 1) = v8 >> 16;
		*(ubyte *)a4 = v8>>24;
		return 0;
	}
	if (a3 == 2)
	{
		v13[0] = -771705058;
		v13[1] = -1107899287;
		v13[2] = -1181752297;
		v13[3] = 1391079134;
		goto LABEL_7;
	}
	return 2;
}

ulong CalculateCumminsACM(uword EngineModel, ubyte *Seed, ubyte *Key) {
	ubyte szkey[16] = { 0 };
	GenerateKeyEx(Seed,4,2,szkey);
	memcpy(Key, szkey, 0x10);
	return 0x10;
}

//最新康明斯时间戳算法
char  TickCountDecrypt(ulong a1, ulong a2, ulong a3)
{
	char result; // al@5
	char i; // [sp+Ch] [bp-1Ch]@1
	char j; // [sp+Ch] [bp-1Ch]@4
	char k; // [sp+Ch] [bp-1Ch]@7
	char l; // [sp+Ch] [bp-1Ch]@12
	char v8; // [sp+10h] [bp-18h]@1
	char v9; // [sp+14h] [bp-14h]@1
	char v10; // [sp+18h] [bp-10h]@1
	char v11; // [sp+1Ch] [bp-Ch]@10
	char v12; // [sp+20h] [bp-8h]@1
	char v13; // [sp+24h] [bp-4h]@1
	ubyte byte_1023F4D4[] = { 0x04, 0x05, 0x09, 0x03, 0x07, 0x00, 0x02, 0x01, 0x08, 0x06, 0x00, 0x00 };

	*(ubyte *)(a1 + 9) = 0;
	v10 = 0;
	v9 = 0;
	v13 = 0;
	v12 = 0;
	v8 = 9;
	for (i = 0; i < 10; ++i)
		*(ubyte *)(a2 + i) = *(ubyte *)(a1 + (ubyte)byte_1023F4D4[i]);
	for (j = 10;; --j)
	{
		result = j;
		if (j <= 0)
			break;
		*(ubyte *)(a2 + j - 1) ^= *(ubyte *)(a2 + (ubyte)byte_1023F4D4[10 - j]);
	}
	for (k = 0; k < 10; ++k)
	{
		*(ubyte *)(a1 + k) = *(ubyte *)(a2 + k);
		*(ubyte *)(a2 + k) = 0;
		result = k + 1;
	}
	v11 = 0;
	while (v11 < 10)
	{
		for (l = 0; l < 8; ++l)
		{
			if ((*(ubyte *)(a3 + v11) << l) & 0x80)
			{
				*(ubyte *)(a2 + v13) = ((signed int)*(ubyte *)(a1 + v8) >> v9++) & 1 | 2 * *(ubyte *)(a2 + v13);
				if (v9 == 8)
				{
					v9 = 0;
					--v8;
				}
			}
			else
			{
				*(ubyte *)(a2 + v13) = (((*(ubyte *)(a1 + v10) << v12++) & 0x80) != 0) | 2 * *(ubyte *)(a2 + v13);
				if (v12 == 8)
				{
					v12 = 0;
					++v10;
				}
			}
		}
		++v11;
		result = v13++ + 1;
	}
	return result;
}

ulong CalculateCumminsTickCountDecrypt(uword EngineModel, ubyte *TickCount, ubyte *Key) {
	if(TickCount[1] == 0)
	{
		ubyte szArg3[] = { 0x46, 0x58, 0x9A, 0x32, 0x70, 0x87, 0x23, 0x99, 0x85, 0x64, 0x00, 0x00, 0x23, 0x24, 0xC0, 0x11, 0x30, 0x0E, 0x46, 0x22, 0x0B, 0xC8, 0x00, 0x00};
			ubyte szArg2[10] = { 0 };
			ubyte strArg1[] = { 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, TickCount[2], TickCount[3], TickCount[4], TickCount[5], 0x00, 0x00};
			if(TickCount[0] == 0)
			{
				TickCountDecrypt((ulong)strArg1,(ulong)szArg2,(ulong)szArg3);
			}
			else
			{
				TickCountDecrypt((ulong)strArg1,(ulong)szArg2,(ulong)(szArg3+0xC));
			}
			memcpy(Key, szArg2, 0xA);
			return 0xA;
	}
	else if(TickCount[1] == 1)
	{
		ubyte szkey[16] = { 0 };
		GenerateKeyEx(TickCount+2,4,2,szkey);
		memcpy(Key, szkey, 0x10);
		return 0x10;

	}
	else if(TickCount[1] == 2)
	{
		ubyte szkey[16] = { 0 };
		GenerateKeyEx(TickCount+2,4,1,szkey);
		memcpy(Key, szkey, 0x10);
		return 0x10;
	}

}



//最新康明斯算法
ulong dword_10240EB4[] = { 0x40226666, 2, 0xA79BDCFE, 0x0A62A1950, 0x0DF5C7B43, 0x9F874F46, 0x67DBC719, 0x3C9C9E7F
, 0x57AC998A, 0x0E050062B, 0x0, 0x0, 0x0C0, 0x46000000, 0x0E0000, 0x0, 0x0C0, 0x46000000};
ulong dword_10240EB8[] = { 0x2, 0xA79BDCFE, 0x0A62A1950, 0x0DF5C7B43, 0x9F874F46, 0x67DBC719, 0x3C9C9E7F
, 0x57AC998A, 0x0E050062B, 0x0, 0x0, 0x0C0, 0x46000000, 0x0E0000, 0x0, 0x0C0, 0x46000000};
ulong dword_10240EBC[] = { 0xA79BDCFE, 0x0A62A1950, 0x0DF5C7B43, 0x9F874F46, 0x67DBC719, 0x3C9C9E7F
, 0x57AC998A, 0x0E050062B, 0x0, 0x0, 0x0C0, 0x46000000, 0x0E0000, 0x0, 0x0C0, 0x46000000};
ulong  sub_100D60B0(ulong a1)
{
	return 8 * ((ulong)(a1 + 7) >> 3) + 4;
}

ulong  sub_100D60C0(ubyte* a1, ubyte* a2, ulong a3)
{
	ulong result; // eax
	ulong v4; // eax
	ubyte *v5; // ecx

	result = 1;
	if (a1 != a2)
	{
		v4 = 0;
		if (a3)
		{
			v5 = (ubyte *)(a1 + a3 - 1);
			do
				*(ubyte *)(v4++ + a2) = *v5--;
			while (v4 < a3);
		}
		result = 0;
	}
	return result;
}
ulong  sub_100D6120(ubyte* a1, ulong *a2)
{
	ulong v2; // ebx
	ulong v3; // ebp
	ulong v4; // edx
	ulong v5; // ecx
	ulong v6; // edx
	ulong v7; // eax
	ulong v8; // ecx
	ubyte v10[4] = {0}; // [esp+10h] [ebp-1Ch]
	ubyte v11[4] = {0}; // [esp+14h] [ebp-18h]
	ulong v12; // [esp+18h] [ebp-14h]
	ulong v13; // [esp+1Ch] [ebp-10h]
	ulong v14; // [esp+20h] [ebp-Ch]
	ulong v15; // [esp+24h] [ebp-8h]
	//byte v16[4] = { 0 }; // [esp+28h] [ebp-4h]

	v2 = 0;
	v3 = 32;
	sub_100D60C0(a1, v10, 4);
	ubyte *v16 = a1 + 4;
	sub_100D60C0(a1 + 4, v11, 4);
	v4 = *a2;
	v13 = a2[1];
	v5 = a2[2];
	v12 = v4;
	v6 = a2[3];
	v7 = *(ulong*)v10;
	v15 = v5;
	v8 = *(ulong*)v11;
	v14 = v6;
	do
	{
		v2 -= 1640531527;
		v7 += (v8 + v2) ^ (v13 + (v8 >> 5)) ^ (v12 + 16 * v8);
		--v3;
		v8 += (v7 + v2) ^ (v15 + 16 * v7) ^ (v14 + (v7 >> 5));
	} while (v3);
	v11[0] = (ubyte)v8;
	v11[1] = (ubyte)(v8 >> 8);
	v11[2] = (ubyte)(v8 >> 16);
	v11[3] = (ubyte)(v8 >> 24);
	v10[0] = (ubyte)v7;
	v10[1] = (ubyte)(v7 >> 8);
	v10[2] = (ubyte)(v7 >> 16);
	v10[3] = (ubyte)(v7 >> 24);
	sub_100D60C0(v10, a1, 4);
	return sub_100D60C0(v11, v16, 4);
}

ulong sub_100D62D0(ulong* seeds,ubyte *a1, ulong a3, ulong *a4, ulong a5, ulong a6, ulong a7, ulong a8)
{
	ulong v8; // edx
	ulong v9; // ecx
	ulong v10; // edx
	ubyte v11; // zf
	ulong v12; // esi
	ubyte* v13; // edi
	ulong v15[4] = {0}; // [esp+8h] [ebp-10h]


	v8 = a6 ^ seeds[1];
	v15[0] = a5 ^ seeds[0];
	v9 = a7 ^ seeds[2];
	v15[1] = v8;
	v10 = a8 ^ seeds[3];
	v12 = (ulong)(a3 + 7) >> 3;
	v11 = v12 == 0;
	v15[2] = v9;
	v13 = a1;
	v15[3] = v10;
	*a4 = 8 * v12 + 4;
	if (!v11)
	{
		do
		{
			--v12;
			sub_100D6120(v13, v15);
			v13 += 8;
		} while (v12);
	}
	return sub_100D60C0((ubyte*)&a3, v13, 4);
}

ulong  CalculateCumminsKey(ubyte* a1, ulong a2, ulong a3, ulong* a4, ubyte* a5, ulong a6, ulong a7)
{
	ulong result; // eax
	ulong v8; // dl
	ubyte* v9; // ecx
	ulong v10; // esi
	ulong v11[4] = { 0 }; // [esp+0h] [ebp-40h]
//	ulong v12; // [esp+4h] [ebp-3Ch]
//	ulong v13; // [esp+8h] [ebp-38h]
//	ulong v14; // [esp+Ch] [ebp-34h]

	result = 2;
	if (a7 > 2u)
		return 9;
	if (a1 && a4 && a5)
	{
		if ((ulong)(a6 - 4) > 0xC)
		{
			result = 4;
		}
		else if (sub_100D60B0(a2) <= a3)
		{
			if (a2)
			{
				v8 = a6;
				if (v8 >= 0x10)
				{
					v10 = 16;
				}
				else
				{
					v9 = a5;
					//v11 = -1;
					*(ulong*)(v11+1) = -1;
					*(ulong*)(v11 + 2) = -1;
					*(ulong*)(v11 + 3) = -1;
					v10 = v8;
				}
				sub_100D60C0(v9, (ubyte*)v11, v10);
				sub_100D62D0(
					v11,
					a1,
					a2,
					(ulong *)a4,
					dword_10240EBC[4 * a7 - 4],
					dword_10240EBC[4 * a7 - 3],
					dword_10240EB4[4 * a7],
					dword_10240EB8[4 * a7]);
				result = 0;
			}
			else
			{
				result = 3;
			}
		}
		else
		{
			result = 1;
		}
	}
	return result;
}

ubyte g_byteData2[] = { 0x06, 0x01, 0x00, 0x22, 0x28, 0x01, 0x01, 0x01, 0x00, 0x22, 0x29, 0x06, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x01, 0x00, 0x22, 0x2A, 0x01, 0x02, 0x01, 0x00, 0x22, 0x2B, 0x02, 0x00, 0x0F, 0x01, 0x00, 0x22, 0x2C, 0x04, 0x70, 0x62, 0x02, 0x40, 0x01, 0x00, 0x22, 0x2D, 0x06, 0x00, 0x00, 0x74, 0xF9, 0x2E, 0x26,0,0,0,0,0,0,0,0,0};


ulong CalculateCumminsECUKey2(uword EngineModel, ubyte *Seed, ubyte *Key) {
	ulong uPlainTxtsize = sizeof(g_byteData2);
	ubyte pszKey[100] = { 0 };
	memcpy(pszKey, g_byteData2, uPlainTxtsize);
	char cEncryptLevel = 0;
	cEncryptLevel = Seed[0];
	if (cEncryptLevel == 0) {
		memcpy(Key, g_byteData2, 0x33);
		return 0x33;
	}

	ulong dwkeySize = 0;
	ulong iDataSize = 0x33;
	ulong a3 = 0x6e8;
	char a6 = 4;

	CalculateCumminsKey(pszKey, iDataSize, a3, &dwkeySize, (Seed+2), a6,1);
	memcpy(Key, pszKey, dwkeySize);
	return dwkeySize;
}


/*******************************  for cummins  ********************************** 2017.9.22 严捷 */
ulong sub_75EB46C4(ubyte *a1, ulong a2, ulong a3)
{
	ulong v3; // r1@1
	ubyte *i; // r3@1
	char v5; // t1@3

	v3 = a2 + a3;
	for (i = a1; i - a1 < a3; ++i)
	{
		v5 = *(ubyte *)(v3-- - 1);
		*i = v5;
	}
	return a3;
}
//a2 = 0x33 dataSize a3 = 0x6E8 ? a1数据包包体
ulong Encrypt1939(ubyte* a1, ulong a2, ulong a3, ulong *a4, ulong a5, ulong a6)
{
	ulong *v6; // r6@1
	ulong v7; // r7@1
	ubyte *v10; // r5@1
//	ulong v11; // r0@2
//	ulong v12; // r1@2
	ulong v13; // r4@9
	ulong v14; // r2@9
	ulong v15; // r7@9
	ulong v16; // r3@9
	ulong v17; // r10@10
	ulong v18; // r6@10
	ulong v19; // r9@10
	ubyte *v20; // r8@11
	ulong v21; // r1@11
	ulong v22; // r2@11
	char *v23; // r3@11
	ubyte *v24; // r0@13
	ulong result; // r0@15
	ulong v26; // [sp+4h] [bp-54h]@9
	ulong v27; // [sp+8h] [bp-50h]@9
	ulong v28; // [sp+10h] [bp-48h]@9
	ulong v29; // [sp+14h] [bp-44h]@11
	ulong v30; // [sp+18h] [bp-40h]@11
	ulong v31; // [sp+1Ch] [bp-3Ch]@1
	ulong v32 = 0xFFFFFFFF; // [sp+20h] [bp-38h]@9
	ulong v33 = 0xFFFFFFFF;; // [sp+24h] [bp-34h]@9
	ulong v34 = 0xFFFFFFFF;; // [sp+28h] [bp-30h]@9


	v6 = a4;
	v7 = a2;
	v10 = a1;


	if ((ulong)(a6 - 4) > 0xCu)
	{
		result = 4;
	}
	else if (v10 && a4 && a5)
	{
		if ((ulong)(((v7 + 7) & 0xFFFFFFF8) + 4) > a3)
		{
			result = 1;
		}
		else if (v7)
		{
			sub_75EB46C4((ubyte*)&v31, a5, a6);
			v13 = (ulong)(v7 + 7) >> 3;
			v28 = v7;
			v14 = v32;
			v26 = v33;
			v15 = 8 * v13;
			v16 = v31;
			v27 = v34;
			*v6 = 8 * v13 + 4;
			if (v13)
			{
				v17 = (ulong)v10;
				v18 = v14 ^ 0xA62A1950;
				v19 = v16 ^ 0xA79BDCFE;
				do
				{
					v20 = (ubyte *)(v17 + 4);
					sub_75EB46C4((ubyte*)&v29, v17, 4);
					sub_75EB46C4((ubyte*)&v30, v17 + 4, 4);
					v21 = v30;
					v22 = v29;
					v23 = (char *)-1640531527;
					do
					{
						v22 += (v18 + (v21 >> 5)) ^ ((ulong)v23+v21) ^ (v19 + 16 * v21);
						v21 += ((v27 ^ 0x9F874F46) + (v22 >> 5)) ^ ((v26 ^ 0xDF5C7B43) + 16 * v22) ^ ((ulong)v23+v22);
						v23 -= 1640531527;
					} while (v23 != (char*)0x6526B0D9);
					v30 = v21;
					v24 = (ubyte *)v17;
					v17 += 8;
					v29 = v22;
					sub_75EB46C4(v24, (ulong)&v29, 4);
					sub_75EB46C4(v20, (ulong)&v30, 4);
					--v13;
				} while (v13);
				v10 += v15;
			}
			sub_75EB46C4(v10, (ulong)&v28, 4);
			result = 0;
		}
		else
		{
			result = 3;
		}
	}
	else
	{
		result = 2;
	}
	return result;
}

static const ubyte g_byteData[] = {
					0x06, 0x01, 0x00, 0x22, 0x28, 0x01, 0x01, 0x01, 0x00, 0x22, 0x29, 0x06, 0x41, 0x42, 0x43, 0x44,
					0x45, 0x46, 0x01, 0x00, 0x22, 0x2A, 0x01, 0x02, 0x01, 0x00, 0x22, 0x2B, 0x02, 0x00, 0x0F, 0x01,
					0x00, 0x22, 0x2C, 0x04, 0x70, 0x51, 0x01, 0x51, 0x01, 0x00, 0x22, 0x2D, 0x06, 0x00, 0x00, 0xD2,
					0xB3, 0x4D, 0x1D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
				};

ulong CalculateCumminsECUKey(uword EngineModel, ubyte *Seed, ubyte *Key) {
	ulong uPlainTxtsize = sizeof(g_byteData);
	ubyte pszKey[100] = { 0 };
	memcpy(pszKey, g_byteData, uPlainTxtsize);
	char cEncryptLevel = 0;
	cEncryptLevel = Seed[0];
	if (cEncryptLevel == 0) {
		memcpy(Key, g_byteData, 0x33);
		return 0x33;
	}

	ulong dwkeySize = 0;
	ulong iDataSize = 0x33;
	ulong a3 = 0x6e8;
	char a6 = 4;

	Encrypt1939(pszKey, iDataSize, a3, &dwkeySize, (ulong)(Seed+2), a6);
	memcpy(Key, pszKey, dwkeySize);
	return dwkeySize;
}



/*******************************  for cummins  **********************************/
static const ubyte g_szPlainText[] = { 0x06, 0x01, 0x00, 0x22, 0x28, 0x01, 0x01,
		0x01, 0x00, 0x22, 0x29, 0x06, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x01,
		0x00, 0x22, 0x2A, 0x01, 0x02, 0x01, 0x00, 0x22, 0x2B, 0x02, 0x00, 0x0F,
		0x01, 0x00, 0x22, 0x2C, 0x04, 0x70, 0x60, 0x02, 0x72, 0x01, 0x00, 0x22,
		0x2D, 0x06, 0x00, 0x00, 0x52, 0x71, 0xEE, 0xEC };

static const ulong LevelKey[8] = { 2812009726U, 2787776848U, 3747380035U,
		2676444998U, 1742456601U, 1016897151U, 1470929290U, 3763340843U };

ulong sub_100D6030(ulong a1) {
	return 8 * ((ulong) (a1 + 7) >> 3) + 4;
}

signed int sub_100D6040(ubyte* a1, ubyte* a2, ulong a3) {
	signed int result;
	ulong v4;
	ubyte* v5;

	result = 1;
	if (a2 != a1) {
		v4 = 0;
		if (a3) {
			v5 = a2 + a3 - 1;
			do
				*(ubyte *) (v4++ + a1) = *(ubyte *) v5--;
			while (v4 < a3);
		}
		result = 0;
	}
	return result;
}

signed int sub_100D60A0(ubyte a1[8], ubyte a2[16]) {
	ulong v2;
	ulong v6;
	ulong v7;
	int v8;
	ulong v10;
	ulong v11;
	ulong v12;
	ulong v13;
	ulong v14;
	ulong v16;

	sub_100D6040((ubyte*) &v10, a1, 4u);
	sub_100D6040((ubyte*) &v11, a1 + 4, 4u);
	v6 = v11;
	v7 = v10;
	v12 = *(ulong *) (a2 + 4);
	v16 = *(ulong *) a2;
	v14 = *(ulong *) (a2 + 12);
	v13 = *(ulong *) (a2 + 8);
	v2 = 0;
	v8 = 32;
	do {
		v2 -= 1640531527;
		v7 += (v6 + v2) ^ (v12 + (v6 >> 5)) ^ (v16 + 16 * v6);
		v6 += (v7 + v2) ^ (v14 + (v7 >> 5)) ^ (v13 + 16 * v7);
		--v8;
	} while (v8);
	v11 = v6;
	v10 = v7;
	sub_100D6040(a1, (ubyte*) &v10, 4u);
	return sub_100D6040(a1 + 4, (ubyte*) &v11, 4u);
}

signed int sub_100D6250(ubyte* a1, ubyte* a2, int a3, ulong* a4, ulong a5,
		ulong a6, ulong a7, ulong a8) {
	int v12;
	ubyte* v13;
	ulong seedEncode[4];

	seedEncode[0] = a5 ^ *(ulong *) a1;
	seedEncode[1] = a6 ^ *(ulong *) (a1 + 4);
	seedEncode[2] = a7 ^ *(ulong *) (a1 + 8);
	seedEncode[3] = a8 ^ *(ulong *) (a1 + 12);
	v12 = (a3 + 7) >> 3;
	v13 = a2;
	*(ulong *) a4 = 8 * v12 + 4;
	if (v12) {
		do {
			--v12;
			sub_100D60A0(v13, (ubyte*) &seedEncode[0]);
			v13 += 8;
		} while (v12);
	}
	return sub_100D6040(v13, (ubyte*) &a3, 4u);
}

signed int sub_100D6380(ubyte* a1, int a2, ulong a3, ulong* a4, ubyte* a5,
		ubyte a6, ubyte a7) {
	signed int result;

	ulong v10;
	ulong seed16[4];

	result = 2;
	if (a7 <= 2u) {
		if (a1 && a4 && a5) {
			if ((ubyte) (a6 - 4) > 0xCu) {
				result = 4;
			} else if (sub_100D6030(a2) <= a3) {
				if (a2) {
					if (a6 >= 0x10u) {
						v10 = 16;
					} else {
						seed16[0] = -1;
						seed16[1] = -1;
						seed16[2] = -1;
						seed16[3] = -1;
						v10 = a6;
					}
					sub_100D6040((ubyte*) &seed16[0], a5, v10);
					sub_100D6250((ubyte*) &seed16[0], a1, a2, a4,
							LevelKey[4 * a7 - 4], LevelKey[4 * a7 - 3],
							LevelKey[4 * a7 - 2], LevelKey[4 * a7 - 1]);
					result = 0;
				} else {
					result = 3;
				}
			} else {
				result = 1;
			}
		}
	} else {
		result = 9;
	}
	return result;
}
/*******************************  for cummins  **********************************/

/*******************************  for weichai  **********************************/
int GetKey_WeiChai_nSeednKey_N1(ubyte pszSeed[4], ulong uSeedSize
								 , ubyte pszKey[4], ulong uKeyBufferSize
								 , ulong* pKeyOutSize);
int GetKey_WeiChai_nSeednKey_N2(ubyte pszSeed[4], ulong uSeedSize
								 , ubyte pszKey[4], ulong uKeyBufferSize
								 , ulong* pKeyOutSize);
int GetKey_WeiChai_nSeednKey_N3(ubyte pszSeed[4], ulong uSeedSize
								 , ubyte pszKey[4], ulong uKeyBufferSize
								 , ulong* pKeyOutSize);
int GetKey_WeiChai_nSeednKey_N4(ubyte pszSeed[4], ulong uSeedSize
								 , ubyte pszKey[4], ulong uKeyBufferSize
								 , ulong* pKeyOutSize);
int GetKey_WeiChai_nSeednKey_N5(ubyte pszSeed[4], ulong uSeedSize
								 , ubyte pszKey[4], ulong uKeyBufferSize
								 , ulong* pKeyOutSize);

static const ulong g_tableCcittWISE[] = {
		//00010000
		0xBEF7E8E8, 0x0A8085DA, 0xA2F542F8, 0x0D581051, 0x9B50F84F,
        0xA00C1AF5, 0x4BB6667E, 0xA2E2CE74, 0xA7BC2A41, 0x062C8A78,
        0x4347CDE5, 0xE2F3B84E, 0x55450DCF, 0x4FC7CD1A, 0x196163E1,
        0xFB92B64D, 0x40E68513, 0xCA656142, 0xF8576AF6, 0x8FB12CB6,
        0x91DD2170, 0x49F23F69, 0xB615D19C, 0x0A68EE30, 0x9852C8DD,
        0xA76C9CD1, 0xA553FA68, 0x789DCA9F, 0xE500E4D1, 0xB3C54150,
        0x8AF9D36D, 0x9F202191, 0x8504BC80, 0x48D4ACB3, 0x29B9246D,
        0x6C15F640, 0x5807D489, 0x54FCE9C3, 0x304C3709, 0x8B9B10C1,
        0x6BA61D7C, 0xC5E6E965, 0x6B32E7B2, 0xD824F19B, 0x2AFAC18C,
        0xF26A9EB4, 0x34E1E105, 0xF9198D23, 0x24D1E3B5, 0x1D6BAA25,
        0xABC44BFC, 0x71D347C4, 0xF9267C78, 0xF04E65C5, 0x1E7870C1,
        0x146D3B6B, 0xBD3392BB, 0x413E19CF, 0xE93674A8, 0x9D7C8C17,
        0x4A6C3D55, 0xFD2118C6, 0x590AAC3A, 0x8A3226E5, 0x08FE9CC4,
        0x5A81E4E8, 0xA315A783, 0xB4E4C924, 0x42671923, 0xBAFD74AF,
        0xC2EC1638, 0x2EF7B9CD, 0xF13DB362, 0x725434BE, 0x218DDB9B,
        0x79884EB8, 0x23C33F86, 0xC9176556, 0xE529FB16, 0x73D1D1B4,
        0x5E256E00, 0x9A3C33BD, 0x9EC61D18, 0x0169EBA5, 0x9E35630A,
        0x14575962, 0x2FFDBBB9, 0x8EDD2A48, 0x4C6BC857, 0xB2CF823D,
        0xCA38FA07, 0xD3AD0382, 0xF8B3D7A8, 0x4B462BE4, 0xAEFF08BE,
        0x242F0E89, 0x853699C1, 0xA4B3E4CC, 0x9AACED5D, 0x92EAA272,
        0x4C811FE5, 0x439051CB, 0x0E01C13B, 0x89D7F793, 0x3CC43F5B,
        0x63F30180, 0x1AF58FE6, 0x725F53AB, 0x27658324, 0xB38672B7,
        0x5A1BA41B, 0x161E1BE6, 0x93B301A6, 0x6AF56F70, 0xF5ED547D,
        0xFAD42EDD, 0xB884B6C9, 0xBE9D80C9, 0x64DB2043, 0x58A2C0EB,
        0x96D33A08, 0x9AAEFE83, 0x208FFDCC, 0xD9AC7717, 0x8C04D351,
        0x20720C6D, 0x4F34FCB6, 0xF1C03F46, 0x9717F432, 0x55F5C9C5,
        0x9E65D616, 0x9333B1BE, 0xB8BF1A3E, 0xE135EB10, 0x2A14B03C,
        0xD0DEDBAC, 0x5AF2F0AB, 0xE746B175, 0x8F46AF42, 0x791F65F1,
        0xC608DB4B, 0x2592C16B, 0xA6928E49, 0xFA7A5C34, 0x75325F94,
        0x85E8F5BB, 0x940C6E50, 0xD0F1BC0B, 0x2EF173E9, 0xC2AAA564,
        0xA93E819E, 0xF26EB87F, 0x9E1A0D2F, 0x0C224B1B, 0x2D880580,
        0x1342C2DD, 0xB0BE881C, 0xA842938C, 0x32ABA16B, 0x4F2B165B,
        0x9FC94C61, 0x3ECE9176, 0xCF737401, 0x2955DCBC, 0xF33B2488,
        0xE9050A8A, 0x5807B6DF, 0x5FFF1358, 0x5C024E34, 0x68D85E72,
        0x079AA7FB, 0x762BFA54, 0x08CF4BCB, 0x8A3CAA1A, 0x4213BE86,
        0x0BE735C5, 0x55C34384, 0xE68308D4, 0xE6D9F495, 0x6731F592,
        0xB9B8E4A2, 0x17CBF798, 0x70C4B648, 0xA45BE10F, 0xECB72175,
        0x73F1812E, 0xA366E7AA, 0x9D657F2B, 0x4753F0E7, 0x053255C7,
        0x8BA4FF5B, 0xAE87E012, 0x9CCBA47E, 0x149311DE, 0x8C4F07E5,
        0xB98360A9, 0x87B96244, 0x09C9E847, 0x57A64879, 0x2AA3E879,
        0xE5267CBE, 0x5A9D180C, 0x8D0B6B7B, 0xFF494622, 0x0043FF12,
        0x2469A57B, 0x5F39C125, 0xFC91645D, 0x6CFCCC71, 0xE80886BD,
        0x14585B9C, 0xB19C6F7C, 0xCE5FDDA4, 0xBC05E74F, 0x01DC612A,
        0xD8AFC3D5, 0x78C20A3A, 0xE0C89975, 0x07628B2E, 0x6530CC64,
        0xC709969B, 0x3A1A398F, 0x4FD6D77A, 0x985B7621, 0xE40D965B,
        0xFF172F06, 0x894E485B, 0x4B8A82B4, 0x1AB3C33C, 0xEBCFA293,
        0x8C2104E0, 0xC83E98E8, 0xDD7C38AF, 0xCAB1F7A6, 0xC07945E8,
        0xD054B639, 0xC05CF355, 0xE36742F0, 0xAFE36EDE, 0xA89ECD05,
        0xE400D3C5, 0xDEB85695, 0x323854AC, 0x4CADD974, 0x59C3F4C1,
        0x249F0966, 0x8888BD45, 0xC881B36D, 0x82377EE2, 0xF8A4D2B0,
        0xAB5D6721, 0xA5FC3CFC, 0x61BBF0A5, 0xF1B9956A, 0x9CCACBFE,
        0x4B456571,
		//00012800
		0x024D27A0, 0xDF168C58, 0x080CA67C, 0xAA6EED2C, 0x98F6021F,
		0x385C3F38, 0x56549E2E, 0x57EDC246, 0x6848325F, 0xBB6B5875,
		0x06703CF8, 0x0DCB0041, 0x4F77C1C4, 0xA754C44E, 0x7A299333,
		0x1C952AE8, 0xC1F1AF5D, 0x52828866, 0x0FDE1248, 0x7CC00CFC,
		0x021C23BB, 0x57D8AD77, 0x07E08841, 0x81832EE3, 0x8087F914,
		0xCA1BEF3E, 0xF7069257, 0xAFB068F7, 0x8EFCD41A, 0xE6611276,
		0x9E30D383, 0xEDE3D5CC, 0xD57DD2FF, 0x14DE3B78, 0xC8D0C4B4,
		0x249C0596, 0x56493068, 0xD75E71E8, 0x4DC02270, 0x7A3D6E5B,
		0x85C4402B, 0x72B23D17, 0x2655AE64, 0xC00DEB56, 0x5D176CE7,
		0x9E413962, 0xB5A2F80E, 0x361E230D, 0xE7508033, 0x4A67299C,
		0xC8470F4A, 0xE2339123, 0x68197141, 0x2E457E66, 0x30AF3AFB,
		0x8EF33548, 0xE2B9275F, 0xFAFFF742, 0xC48079E9, 0x022EB618,
		0xC138D633, 0x6C735B68, 0x0C6D7B65, 0x969F2FCC, 0xB0FD3ED6,
		0x49E966C4, 0x237DDA09, 0xAA0967B8, 0x1A1BDD52, 0x0558AB1A,
		0xE9BFFDB3, 0x0933C77E, 0x66DAFB65, 0xD02C552D, 0x80F9DE4C,
		0x0667F2D0, 0x2D2492E1, 0x455FF082, 0x4B840B2A, 0x496AC8E5,
		0x67379797, 0x5EECF19C, 0xB48C1B39, 0x7D98F66B, 0xCA0533BC,
		0x87C387BA, 0xFB180E56, 0xD3B65933, 0x51C67180, 0xEC26989F,
		0x218933DC, 0xD9537C8D, 0x860D125E, 0xF1464694, 0x2E42B722,
		0x0246CE6C, 0x536E2FE5, 0x41454F67, 0x5135C946, 0xAA14CFAF,
		0xFF8F7282, 0xF78D28DD, 0x0C783645, 0xEBC6B86B, 0x39387982,
		0x1B8564CA, 0xA30A647E, 0xD4D8B45B, 0xFAA2A700, 0x9E2C236E,
		0x7F40153D, 0x40D1C897, 0xAAED4F3E, 0xD4385138, 0x986A7FB0,
		0xBFDE5209, 0xE3BCA0D0, 0x0FB49FF5, 0xDD7DAA54, 0x10474A90,
		0xF742E795, 0xDD558DD0, 0x21362CA2, 0x67DF7BB6, 0xBA1C0E87,
		0x9B263325, 0xC6CD6E43, 0x77128A57, 0xA0440CDC, 0xA8D77163,
		0x0A882079, 0xBB094418, 0x746C8328, 0xD9F5F9DB, 0xAA701678,
		0xB4AD1A10, 0x31E42635, 0xDA3D93A2, 0x09B46146, 0xEC1175AD,
		0x94F92BCF, 0x32063CEC, 0x22C64BE8, 0x25FD718A, 0x7EBF6EB4,
		0x202F518C, 0x5CB54A88, 0x1CC38031, 0xE2C73EB2, 0x058A5EC6,
		0xEA9D39E5, 0x6ED862FF, 0xFA6B70F1, 0x5D24324B, 0xFE54C0D4,
		0x0ECF2224, 0x7E3210E0, 0x7AA462EF, 0x4D9D300B, 0x5884AE8D,
		0x08BB1371, 0xAE77FCBE, 0x74360466, 0x185FA08C, 0xDA1C0290,
		0x7EA553BE, 0x414F87E0, 0xFBBBE197, 0x4C1674C0, 0xEEE483BC,
		0x53AFC73D, 0x8CD48B85, 0xE85FFA65, 0x750612E6, 0x060458E2,
		0xAA004A92, 0x0984E4CE, 0xE9C19434, 0x0C2F1385, 0xB17FB42D,
		0x8DB0A784, 0xF5D1E8F0, 0x16FEA426, 0x98189E53, 0x46C0744F,
		0x8380D3F9, 0xE2D3CBAD, 0xEFE6C262, 0xC1F721FD, 0xCADEFF88,
		0x4C6DE362, 0x2B371181, 0x389CCE71, 0x9E24E91D, 0x0FD239D2,
		0xCE230805, 0x5ACFD02A, 0x6F045C53, 0x603D9D18, 0x521AD2A7,
		0xAEAC8D04, 0xD3758053, 0x49A9637E, 0xAE3A239B, 0x520C12C4,
		0x59D86A65, 0xF0B86988, 0xEC889745, 0x00B0B665, 0x6317D95D,
		0x48FF6B1B, 0xF4CB0907, 0x522E3F5D, 0xE67DDB88, 0xECC94E61,
		0xD26BF050, 0x7341B347, 0x7EC34C0B, 0x2AB22B97, 0x4346679A,
		0x3F5C40DB, 0x2D509FF5, 0x8612EACE, 0x9FC5A226, 0x496471CF,
		0xA71548F1, 0x8E060A58, 0xB83B3B12, 0xFB9CC705, 0x9382D221,
		0x0234C357, 0xC0EBEBB7, 0x64AA167B, 0x538DDA4F, 0x854B8F12,
		0xE4D5D002, 0x2599B63C, 0xEA09FED5, 0xEB6A4BA3, 0xE0C1344C,
		0xD33C2624, 0x638150C8, 0x21087A1A, 0xC2D233EF, 0x91681E48,
		0xE0674C50, 0x32012DE1, 0xCDB56C3A, 0x2F81C34C, 0x74101C8C,
		0x33C2A01F, 0xD17365F9, 0xFAA690B7, 0xD1C1C904, 0xBEA89421,
		0xAA8555C7
	};
static const ulong *g_pCcitt_WISE_27_01 = &g_tableCcittWISE[0];
static const ulong *g_pCcitt_WISE_27_03 = &g_tableCcittWISE[256];

static const ulong g_tableOldCcittWISE[] = {
		//00010000
		0x01FFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x50368356,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xABCDDCBA,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0x12345678, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0x11223344, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF,
		//00012800
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x90343413,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xAAFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xC8FFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x02085309,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0x00787655, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0x03490245, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xCCFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF
	};
static const ulong *g_pCcitt_WISE_27_01_Old = &g_tableOldCcittWISE[0];
static const ulong *g_pCcitt_WISE_27_03_Old = &g_tableOldCcittWISE[256];

static const ulong g_tableCcittWNG[] = {
		//00010000
		0x682C9993, 0xEFDC0595, 0x04654DB9, 0xC2EAAC6F, 0x40EB3838,
		0x9245DEB7, 0x05256A20, 0xA0CEAAC1, 0xEA35CF1E, 0x0050672E,
		0x694B58BD, 0xEDB1CB46, 0x01F6EBA9, 0xF1889193, 0x49F9DB62,
		0x9EDE4133, 0x57E2A185, 0x1B1AFDB9, 0x08457653, 0x48996EF5,
		0xEE6C7347, 0x7679C62F, 0x002B7F72, 0x0F447F65, 0xDC54E4AE,
		0x85B25887, 0xD3ED2F1D, 0x640FDBB4, 0x4120A67F, 0x9191F17A,
		0x42DBF84F, 0xFF416419, 0x7B504D64, 0x07CB2257, 0xFFED7F51,
		0x7C7955C2, 0xE41F85F7, 0x0355CAA8, 0x8C7DC9E6, 0x3DC81A80,
		0x300BFB81, 0x41F20CBD, 0xCC25C8E2, 0xFA4123D1, 0xE91BD2A5,
		0x148B028B, 0x3C844B36, 0xB147D358, 0x15B6D267, 0xC5F36B9B,
		0xF9E6AE64, 0x2B451ED9, 0x3A6EE08F, 0xFDCADB58, 0xF7FC620F,
		0xD8B73F96, 0xF41D5122, 0x1506D751, 0x62B0AE45, 0x22EC4DEB,
		0xC9D467F6, 0xCEDBE920, 0xB695FF2B, 0x5F9A4737, 0x5AE3C82D,
		0x3E994FD4, 0x6679A6AB, 0x865C6C89, 0x9B0FD48E, 0x589FC480,
		0x3F29048B, 0xE49212BC, 0x5D2DB4A6, 0xE240B98D, 0xEE7DC4D0,
		0xE71709FE, 0x647A6175, 0x723F8BB3, 0x086EDA7F, 0xDA9BE3CC,
		0x09F87EE2, 0x6CDCCB69, 0x512369A4, 0x2E37ED8D, 0x8AF06A02,
		0xE1753058, 0x9390FA89, 0x7F3268F5, 0x46A99ACE, 0xECD8D66E,
		0xC3D89184, 0xF21B2504, 0x52750866, 0xA09BB90F, 0x0F739B21,
		0xD0515D62, 0x562345F9, 0x589883E5, 0x45A3C4CF, 0x0838AF89,
		0xD19F80C5, 0xA0460AA7, 0x534B9E18, 0x95F518D5, 0x88649524,
		0x08006609, 0x7463BD56, 0xD07B03F8, 0x2B14CC7E, 0xEBFA4744,
		0x33046C01, 0x62F58F2B, 0x6BF4420C, 0x5BCC3E86, 0xE7C0C73E,
		0xD1B012ED, 0xED452069, 0x651D1013, 0x141BC7E6, 0x52D55BA4,
		0xFF1E23A0, 0xD67D136C, 0x2FB105A7, 0xC56E8372, 0xF3CF2B0A,
		0xF6E24689, 0x3A267BCD, 0xD6872394, 0x8CE00015, 0x4642E354,
		0xD95946F7, 0xBC5E4F13, 0x893AA108, 0xA5AAEA23, 0xBEC042E0,
		0x4178E46F, 0xF3E47608, 0x3CF1098C, 0xF8D8318F, 0x238BF8A9,
		0x62336C1B, 0x5B75A07C, 0x66570F63, 0x66605BEF, 0x0CC42343,
		0x4721022D, 0xF99F1C4E, 0xC3F43BF1, 0x401B53A9, 0x2B4C4622,
		0x97B568B3, 0x7384B024, 0x4189F320, 0x8B047D1A, 0x0F3678CE,
		0x89710A3A, 0xA97B13AB, 0x4975426D, 0x6B38ACD2, 0xC4D22FD3,
		0x0118325C, 0x8452533A, 0xB557FF87, 0x7191B08D, 0x0AB8886A,
		0x766C7599, 0x832061DD, 0x96DD8683, 0xB6019B5C, 0xA331FC8E,
		0xC7C1A2B1, 0x11E4253E, 0xD961CA0F, 0x1D97A7E4, 0x5DB76AAD,
		0x4D35297F, 0x6680A460, 0xC71F44FE, 0x3003B9BA, 0x41FD29CD,
		0x1A1DC91F, 0xE0C02F07, 0xA98FE4B7, 0xDB50F638, 0x00D06D9A,
		0xF9D06019, 0xB46F62D6, 0xEEB1DE8B, 0xF2AA26D6, 0x0289A487,
		0x5CCD80FA, 0xEEFF5E3F, 0xB74D09BB, 0xE703B8F9, 0x14343C50,
		0x7DD7A56D, 0xA2214D5A, 0x4088CC97, 0x1A257576, 0x4335E43A,
		0x8D5CB3DF, 0x611EBA95, 0x9A50E062, 0x3409916D, 0x8625E659,
		0x207AF5C6, 0x950D9D05, 0xEA564947, 0x4F4D55ED, 0x53673BD7,
		0xD4E7F3E5, 0xB9F1560B, 0x1DC03DD9, 0x64F6E850, 0xA9A1CD85,
		0x6C0B1868, 0x5455A60D, 0xBF4BA18F, 0xAE82CDC7, 0xDC45678D,
		0x59E8EFE7, 0x0820B69D, 0xE33C741E, 0x895ABCC9, 0xF1B5E905,
		0xBA269897, 0xB3C8A3D3, 0xC88D2949, 0x957EBF7C, 0x245E498D,
		0xE1240B83, 0xD2B23CBD, 0x54E29EB5, 0xC4218965, 0x360832CE,
		0x5BF772FA, 0x8DE37F8B, 0x41B3737B, 0xC4E6A2A2, 0x19DB8F96,
		0xD20B4981, 0xE6AA4783, 0xAB15576F, 0xB7B0A358, 0x9C875E0E,
		0x77DDABF5, 0x88818D4A, 0x900C0A65, 0x97FA71F8, 0xFE61E67C,
		0x213DEC95, 0xF6C4462E, 0x5A1518D9, 0x22170ED3, 0xE5E40D5C,
		0xFB6BB7FC
	};
static const ulong *g_pCcitt_WNG_27_01 = &g_tableCcittWNG[0];

enum SEED_KEY_MODE{
	WISE_27_01,
	WISE_27_03,
	WISE_27_01_OLD,
	WISE_27_03_OLD,
	WNG_27_01
};

int SecretAccess1(enum SEED_KEY_MODE modeSK, const ubyte* seed, ulong uSeedSize, ubyte* key, ulong* pKeyOutSize);
int BuildKey(enum SEED_KEY_MODE modeSK, const ubyte* pszSeed, ulong uSeedSize, ubyte* pszKey, ulong* pKeyOutSize);


int GetKey_WeiChai_nSeednKey_N1(ubyte pszSeed[4], ulong uSeedSize
							 , ubyte pszKey[4], ulong uKeyBufferSize
							 , ulong* pKeyOutSize)
{
    int bReturn;

    if(!pszSeed || uSeedSize < 1
            || !pszKey || uKeyBufferSize < 4 || uKeyBufferSize < uSeedSize
            || !pKeyOutSize)
    {
        return 0;
    }

    *pKeyOutSize = 0;
	bReturn = SecretAccess1(WISE_27_03, pszSeed, uSeedSize, pszKey, pKeyOutSize);

    return bReturn;
}



int GetKey_WeiChai_nSeednKey_N2(ubyte pszSeed[4], ulong uSeedSize
							 , ubyte pszKey[4], ulong uKeyBufferSize
							 , ulong* pKeyOutSize)
{
    int bReturn;

    if(!pszSeed || uSeedSize < 1
            || !pszKey || uKeyBufferSize < 4 || uKeyBufferSize < uSeedSize
            || !pKeyOutSize)
    {
        return 0;
    }

    *pKeyOutSize = 0;
    bReturn = SecretAccess1(WISE_27_01, pszSeed, uSeedSize, pszKey, pKeyOutSize);

    return bReturn;
}


int GetKey_WeiChai_nSeednKey_N3(ubyte pszSeed[4], ulong uSeedSize
							 , ubyte pszKey[4], ulong uKeyBufferSize
							 , ulong* pKeyOutSize)
{
    int bReturn;

    if(!pszSeed || uSeedSize < 1
            || !pszKey || uKeyBufferSize < 4 || uKeyBufferSize < uSeedSize
            || !pKeyOutSize)
    {
        return 0;
    }

    *pKeyOutSize = 0;
    bReturn = BuildKey(WISE_27_03_OLD, pszSeed, uSeedSize, pszKey, pKeyOutSize);

    return bReturn;
}


int GetKey_WeiChai_nSeednKey_N4(ubyte pszSeed[4], ulong uSeedSize
							 , ubyte pszKey[4], ulong uKeyBufferSize
							 , ulong* pKeyOutSize)
{
    int bReturn;

    if(!pszSeed || uSeedSize < 1
            || !pszKey || uKeyBufferSize < 4 || uKeyBufferSize < uSeedSize
            || !pKeyOutSize)
    {
        return 0;
    }

    *pKeyOutSize = 0;
    bReturn = BuildKey(WISE_27_01_OLD, pszSeed, uSeedSize, pszKey, pKeyOutSize);

    return bReturn;
}


int GetKey_WeiChai_nSeednKey_N5(ubyte pszSeed[4], ulong uSeedSize
							, ubyte pszKey[4], ulong uKeyBufferSize
							, ulong* pKeyOutSize)
{
    int bReturn;

    if(!pszSeed || uSeedSize < 1
            || !pszKey || uKeyBufferSize < 4 || uKeyBufferSize < uSeedSize
            || !pKeyOutSize)
    {
        return 0;
    }

    *pKeyOutSize = 0;
    bReturn = SecretAccess1(WNG_27_01, pszSeed, uSeedSize, pszKey, pKeyOutSize);

    return bReturn;
}

const ulong * GetSeedKeyData(enum SEED_KEY_MODE modeSK)
{
	switch(modeSK)
	{
	case WISE_27_01:
		return g_pCcitt_WISE_27_01;
	case WISE_27_03:
		return g_pCcitt_WISE_27_03;
	case WISE_27_01_OLD:
		return g_pCcitt_WISE_27_01_Old;
	case WISE_27_03_OLD:
		return g_pCcitt_WISE_27_03_Old;
	case WNG_27_01:
		return g_pCcitt_WNG_27_01;
	default:
		break;
	}

	return NULL;
}

int SeedToKey(enum SEED_KEY_MODE modeSK, const ubyte* pszSeed, ulong uSeedSize, ulong * dwKey)
{
	ulong i;
	const ulong * pCcittDataBegin;

	pCcittDataBegin = GetSeedKeyData(modeSK);
	if(pCcittDataBegin)
	{
        for (i = 0; i < uSeedSize; i++)
        {
            *dwKey = ((*dwKey) >> 16 ^ pCcittDataBegin[(int)(((*dwKey) ^ (int)pszSeed[i]) & 0xFF)]);
        }
		return 1;
	}
	return 0;
}

int BuildKey(enum SEED_KEY_MODE modeSK, const ubyte* pszSeed, ulong uSeedSize, ubyte* pszKey, ulong* pKeyOutSize)
{
	ulong k;
	int j;
	ulong dwKey;

	dwKey = 0;
	if(SeedToKey(modeSK, pszSeed, uSeedSize, &dwKey))
	{
		j = 0;
		for (k = uSeedSize; k > 0; k--)
		{
			pszKey[j] = (ubyte)(dwKey >> (k - 1) * 8 & 0xFF);
			j++;
		}
		*pKeyOutSize = uSeedSize;
		return 1;
	}
	return 0;
}

int SecretAccess1(enum SEED_KEY_MODE modeSK, const ubyte* seed, ulong uSeedSize, ubyte* key, ulong* pKeyOutSize)
{
	switch(uSeedSize)
	{
	case 2:
		if (seed[0] == 0x12 && seed[1] == 0x34)
		{
			key[0] = 0xED;
			key[1] = 0xCB;
			*pKeyOutSize = 2;
			return 1;
		}
		return 0;
	case 4:
		if (seed[0] == 0x00 && seed[1] == 0x00 && seed[2] == 0x00 && seed[3] == 0x00)
		{
			key[0] = 0x00;
			key[1] = 0x00;
			key[2] = 0x00;
			key[3] = 0x00;
			*pKeyOutSize = 4;
			return 1;
		}
		else if (seed[0] == 0x00 && seed[1] == 0x01 && seed[2] == 0x02 && seed[3] == 0x03)
		{
			key[0] = 0x03;
			key[1] = 0x02;
			key[2] = 0x01;
			key[3] = 0x00;
			*pKeyOutSize = 4;
			return 1;
		}
		break;
	default:
		break;
	}

	return BuildKey(modeSK, seed, uSeedSize, key, pKeyOutSize);
}
/*******************************  for weichai  **********************************/

static uint32_t maskList[][4] = {{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0xD9503B38,0,0,0},
		{0xD9503B38,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0x079EA519,0,0,0},
		{0x079EA519,0,0,0},
		{0x079EA519,0,0,0},
		{0x6B2D7E84,0x02267E84,0x47217E84,0x45247E84},
		{0x6B2D7E84,0x02267E84,0x47217E84,0x45247E84},
		{0x6B2D7E84,0x02267E84,0x47217E84,0x45247E84},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x3D5B8329,0,0,0},
		{0xD9503B38,0,0,0},
		{0x326e6970,0,0,0},
		{0x326e6970,0,0,0},
		{0x326e6970,0,0,0},
		{0x30279812,0,0,0},
		{0x30279812,0,0,0},
		{0x30279812,0,0,0},
		{0xD9503B38,0,0,0},
		{0xD9503B38,0,0,0},
		{0xD9503B38,0,0,0},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x38192ED2,0,0,0},
		{0x38192ED2,0,0,0},
		{0x38192ED2,0,0,0},
		{0x9C516C1B,0,0,0},
		{0x9C516C1B,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0x4E5E5762,0,0,0},
		{0x4E5E5762,0,0,0},
		{0x4E5E5762,0,0,0},
		{0x245EFC5C,0,0,0},
		{0x245EFC5C,0,0,0},
		{0x245EFC5C,0,0,0},
		{0x61798024,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x55130714,0,0,0},
		{0x55130714,0,0,0},
		{0x55130714,0,0,0},
		{0x55130714,0,0,0},
		{0x55130714,0,0,0},
		{0x55130714,0,0,0},
		{0x106B8225,0,0,0},
		{0x106B8225,0,0,0},
		{0x106B8225,0,0,0},
		{0x106B8225,0,0,0},
		{0x106B8225,0,0,0},
		{0x106B8225,0,0,0},
		{0x8A836B58,0,0,0},
		{0x8A836B58,0,0,0},
		{0x8A836B58,0,0,0},
		{0x8A836B58,0,0,0},
		{0x8A836B58,0,0,0},
		{0x8A836B58,0,0,0},
		{0xF9465626,0,0,0},
		{0xF9465626,0,0,0},
		{0xF9465626,0,0,0},
		{0x1143813E,0,0,0},
		{0x1143813E,0,0,0},
		{0x1143813E,0,0,0},
		{0x54130914,0,0,0},
		{0x54130914,0,0,0},
		{0x54130914,0,0,0},
		{0x079EA519,0,0,0},
		{0x079EA519,0,0,0},
		{0x079EA519,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x9374EFF4,0,0,0},
		{0x9374EFF4,0,0,0},
		{0x9374EFF4,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0x1D4A3579,0,0,0},
		{0x1D4A3579,0,0,0},
		{0x1D4A3579,0,0,0},
		{0x1D4A3579,0,0,0},
		{0x1D4A3579,0,0,0},
		{0x1D4A3579,0,0,0},
		{0x52130404,0,0,0},
		{0x52130404,0,0,0},
		{0x52130404,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0xD9503B38,0,0,0},
		{0xD9503B38,0,0,0},
		{0xD9503B38,0,0,0},
		{0x9C516C1B,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x9C516C1B,0,0,0},
		{0x61798024,0,0,0},
		{1,1,1,1},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x83191226,0,0,0},
		{0xD9503B38,0,0,0},
		{0xD9503B38,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x83191226,0,0,0},
		{0x4E5E5762,0,0,0},
		{0x079EA519,0,0,0},
		{0x1143813E,0,0,0},
		{0x55130714,0,0,0},
		{0x1D4A3579,0,0,0},
		{0x1143813E,0,0,0},
		{0x9C516C1B,0,0,0},
		{0x9C516C1B,0,0,0},
		{0x9C516C1B,0,0,0},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x504BA019,0x0648A019,0x4741A019,0x4544A019},
		{0x504BA019,0x0648A019,0x4741A019,0x4544A019},
		{0x504BA019,0x0648A019,0x4741A019,0x4544A019},
		{0x19A04B50,0x19A04806,0x19A04147,0x19A04445},
		{0x19A04B50,0x19A04806,0x19A04147,0x19A04445},
		{0x19A04B50,0x19A04806,0x19A04147,0x19A04445},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x2AFA756F,0x2AFA7800,0x2AFA7147,0x2AFA7445},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0x6B2D7E84,0x02267E84,0x47217E84,0x45247E84},
		{0x6B2D7E84,0x02267E84,0x47217E84,0x45247E84},
		{0x6B2D7E84,0x02267E84,0x47217E84,0x45247E84},
		{0x8A967163,0x00917163,0x47917163,0x45947163},
		{0x8A967163,0x00917163,0x47917163,0x45947163},
		{0x8A967163,0x00917163,0x47917163,0x45947163},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x1881B150,0x1881B405,0x1881B147,0x1881B445},
		{0x1881B150,0x1881B405,0x1881B147,0x1881B445},
		{0x1881B150,0x1881B405,0x1881B147,0x1881B445},
		{0xD4CF9578,0xD4CF9404,0xD4CF9147,0xD4CF4445},
		{0xD4CF9578,0xD4CF9404,0xD4CF9147,0xD4CF4445},
		{0xD4CF9578,0xD4CF9404,0xD4CF9147,0xD4CF4445},
		{0xE56B8237,0xE56B8408,0xE56B8147,0xE56B8445},
		{0xE56B8237,0xE56B8408,0xE56B8147,0xE56B8445},
		{0xE56B8237,0xE56B8408,0xE56B8147,0xE56B8445},
		{0xE56B8237,0xE56B8408,0xE56B8147,0xE56B8445},
		{0xE56B8237,0xE56B8408,0xE56B8147,0xE56B8445},
		{0xE56B8237,0xE56B8408,0xE56B8147,0xE56B8445},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x58130808,0,0,0},
		{0x58130808,0,0,0},
		{0x58130808,0,0,0},
		{0x52130404,0,0,0},
		{0x52130404,0,0,0},
		{0x52130404,0,0,0},
		{0x326e6970,0,0,0},
		{0x326e6970,0,0,0},
		{0x326e6970,0,0,0},
		{0x30279812,0,0,0},
		{0x30279812,0,0,0},
		{0x30279812,0,0,0},
		{0x106B8225,0,0,0},
		{0x106B8225,0,0,0},
		{0x106B8225,0,0,0},
		{0x54130914,0,0,0},
		{0x54130914,0,0,0},
		{0x54130914,0,0,0},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x30279812,0,0,0},
		{0x30279812,0,0,0},
		{0x30279812,0,0,0},
		{0x106B8225,0,0,0},
		{0x106B8225,0,0,0},
		{0x106B8225,0,0,0},
		{0x1143813E,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x74665A43,0,0,0},
		{0xD54E4180,0,0,0},
		{0x74665A43,0,0,0},
		{0x61798024,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0x74665A43,0,0,0},
		{0x74665A43,0,0,0},
		{0x61798024,0,0,0},
		{0x61798024,0,0,0},
		{0x4E5E5762,0,0,0},
		{0x4E5E5762,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x106B8225,0,0,0},
		{0xD9503B38,0,0,0},
		{0xD9503B38,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0x1143813E,0,0,0},
		{0x61798024,0,0,0},
		{0x74665A43,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0x8A836B58,0,0,0},
		{0x61798024,0,0,0},
		{0x74665A43,0,0,0},
		{0xD54E4180,0,0,0},
		{0xD54E4180,0,0,0},
		{0x8A836B58,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x245EFC5C,0,0,0},
		{0x10C54A3D,0x10C54007,0x10C54147,0x10C54445},
		{0x245EFC5C,0,0,0},
		{0xD9503B38,0,0,0},
		{0x83191226,0,0,0},
		{0xD9503B38,0,0,0},
		{0xD9503B38,0,0,0},
		{0xDA503B38,0,0,0},
		{0xA766135C,0,0,0},
		{0xA766135C,0,0,0},
		{0x77566633,0,0,0},
		{0x78566633,0,0,0},
		{0x79566633,0,0,0},
		{0x7A566633,0,0,0},
		{0x7B566633,0,0,0},
		{0x7C566633,0,0,0},
		{0x9A7856A4,0,0,0},
		{0x02050450,0x08080001,0x47414944,0x45444E00},
		{0x3D4AC510,0x0740C510,0x4741C510,0x4544C510},
		{0x1E38B579,0x0638B579,0x4731B579,0x4534B579},
		{0x8A836B58,0,0,0},
		{0x02050450,0x08080001,0x47414944,0x45444E00},
		{0x02050450,0x00080001,0x47414944,0x45444E00},
		{0x8795FCD4,0x8795F404,0x8795F147,0x87954445},
		{0x6B2D7E84,0x02267E84,0x47217E84,0x45247E84},
		{0x81492A5E,0x03462A5E,0x47412A5E,0x45442A5E},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0x504BA019,0x0648A019,0x4741A019,0x4544A019},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x8795FCD4,0x8795F404,0x8795F147,0x87954445},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x50B18118,0x05B48118,0x47B18118,0x45B48118},
		{0x77566633,0,0,0},
		{0x6F75FA2A,0x0078FA2A,0x4771FA2A,0x4574FA2A},
		{0x504BA019,0x0648A019,0x4741A019,0x4544A019},
		{0x03000801,0,0,0},
		{0x03000801,0,0,0},
		{0x03000801,0,0,0},
		{0x58130808,0,0,0},
		{0x58130808,0,0,0},
		{0x58130808,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x312C1B7C,0,0,0},
		{0x322C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x83191226,0,0,0},
		{0x83191226,0,0,0},
		{0x83191226,0,0,0},
		{0x07000650,0x03030001,0x47414944,0x45444E00},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x504BA019,0x0648A019,0x4741A019,0x4544A019},
		{0xD4CF9578,0xD4CF9404,0xD4CF9147,0xD4CF4445},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0xFFFFFFFF,0,0,0},
		{0x78563412,0,0,0},
		{0x9A785694,0,0,0},
		{0x9A785634,0,0,0},
		{0x9A785614,0,0,0},
		{0x9A785624,0,0,0},
		{0x9A785644,0,0,0},
		{0x9A785654,0,0,0},
		{0x9A785664,0,0,0},
		{0x9A785674,0,0,0},
		{0x9A785684,0,0,0},
		{1,1,1,1},
		{0xB197B197,0,0,0},
		{0xDB559AEA,0,0,0},
		{0x9C516C1B,0,0,0},
		{0x9C516C1B,0,0,0},
		{0x43204320,0,0,0},
		{0x302C1B7C,0,0,0},
		{0xBBAA2211,0,0,0},
		{0xAAAAAAAA,0,0,0},
		{0x2AFA756F,0x2AFA7800,0x2AFA7147,0x2AFA7445},
		{0x2AFA756F,0x2AFA7800,0x2AFA7147,0x2AFA7445},
		{0x2AFA756F,0x2AFA7800,0x2AFA7147,0x2AFA7445},
		{0x2AFA756F,0x2AFA7800,0x2AFA7147,0x2AFA7445},
		{0x2AFA756F,0x2AFA7800,0x2AFA7147,0x2AFA7445},
		{0x2AFA756F,0x2AFA7800,0x2AFA7147,0x2AFA7445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x99797601,0x08587601,0x47515621,0x45446611},
		{0x2C21160B,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x9C516C1B,0,0,0},
		{0x302C1B7C,0,0,0},
		{0xBBBBBBBB,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x4E5E5762,0,0,0},
		{0x245EFC5C,0,0,0},
		{0x61798024,0,0,0},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x2C21160B,0,0,0},
		{0x265646F9,0x26564704,0x26564147,0x26564445},
		{0x265646F9,0x26564704,0x26564147,0x26564445},
		{0x265646F9,0x26564704,0x26564147,0x26564445},
		{0x84BA3027,0x14B73027,0x58B23027,0x56B53027},
		{0x84BA3027,0x14B73027,0x58B23027,0x56B53027},
		{0x84BA3027,0x14B73027,0x58B23027,0x56B53027},
		{0x84BA3027,0x14B73027,0x58B23027,0x56B53027},
		{0x84BA3027,0x14B73027,0x58B23027,0x56B53027},
		{0x84BA3027,0x14B73027,0x58B23027,0x56B53027},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x02050450,0x08080001,0x47414944,0x45444E00},
		{0x6B2D7E84,0x02267E84,0x47217E84,0x45247E84},
		{0xFFFFFFFF,0,0,0},
		{0xD4CF9578,0xD4CF9404,0xD4CF9147,0xD4CF4445},
		{0xE56B8237,0xE56B8408,0xE56B8147,0xE56B8445},
		{0xE56B8237,0xE56B8408,0xE56B8147,0xE56B8445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x55130714,0,0,0},
		{0x4B1C6340,0x4B1C6207,0x4B1C6147,0x4B1C6445},
		{0x4B1C6340,0x4B1C6207,0x4B1C6147,0x4B1C6445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x55130714,0,0,0},
		{0x99797602,0x08587601,0x47515621,0x45446611},
		{0x99796602,0x08587601,0x47515621,0x45446611},
		{0x99797603,0x08587601,0x47515621,0x45446611},
		{0x99796604,0x08587601,0x47515621,0x45446611},
		{0x9C516C1B,0,0,0},
		{0x9C516C1C,0,0,0},
		{0xD4CF9578,0xD4CF9404,0xD4CF9147,0xD4CF4445},
		{0x8795FCD4,0x8795F404,0x8795F147,0x87954445},
		{0x8795FCD4,0x8795F404,0x8795F147,0x87954445},
		{0x9C516C1B,0,0,0},
		{0x302C1B7C,0,0,0},
		{0xD9503B38,0,0,0},
		{0x3D5B8329,0,0,0},
		{0x3D5B8399,0,0,0},
		{0x3D5B8389,0,0,0},
		{0xD9503B38,0,0,0},
		{0xD9503B38,0,0,0},
		{0x7B566634,0,0,0},
		{0x7B566635,0,0,0},
		{0x7B566636,0,0,0},
		{0x50060007,0x01000904,0x44494147,0x004E4445},
		{0x27FC931A,0x27FC9407,0x27FC9147,0x27FC9445},
		{0x7B566637,0,0,0},
		{0x59C81BA0,0x59C81906,0x59C81147,0x59C81445},
		{0x59C81BA0,0x59C81906,0x59C81147,0x59C81445},
		{0x59C81BA0,0x59C81906,0x59C81147,0x59C81445},
		{0x302C1B7C,0,0,0},
		{0x74665A43,0,0,0},
		{0x9C516C2C,0,0,0},
		{0x079EA529,0,0,0},
		{0x079EA539,0,0,0},
		{0x9C516C3C,0,0,0},
		{0x9C516C4C,0,0,0},
		{0x1143813E,0,0,0},
		{0x61798024,0,0,0},
		{0xD54E4180,0,0,0},
		{0x61798024,0,0,0},
		{0x01767999,0x01765808,0x21565147,0x11664445},
		{0x1143813E,0,0,0},
		{0xD54E4180,0,0,0},
		{0x302C1B7C,0,0,0},
		{0x55130714,0,0,0},
		{0x83191226,0,0,0},
		{0x4E5E5762,0,0,0},
		{0x8795FCD4,0x8795F404,0x8795F147,0x87954445},
		{0x8795FCD4,0x8795F404,0x8795F147,0x87954445},
		{0xD4CF9578,0xD4CF9404,0xD4CF9147,0xD4CF4445},
		{0xD4CF9578,0xD4CF9404,0xD4CF9147,0xD4CF4445},
		{0xD5CF9578},
		{0x37826BA5,0,0,0},
		{0x37826BB5,0,0,0},
		{0x37826BC5,0,0,0},
		{0x37826BD5,0,0,0},
		{0x37826BE5,0,0,0},
		{0x37826BF1,0,0,0},
		{0x37826BF2,0,0,0},
		{0x37826BF3,0,0,0},
		{0x6B9C3E0D,0,0,0},
		{0x50060007,0x01000806,0x44494147,0x004E4445},
		{0x37826BF4,0,0,0},
		{0x245EFC5C,0,0,0},
		{0x37826BE6,0,0,0},
		{0x245EFC5C,0,0,0},
		{0x41534B4D,0,0,0},
		{0x41534B4D,0,0,0},
		{0x41534B4D,0,0,0},
		{0x245EFC5C,0,0,0},
		{0x8A836B58,0,0,0},
		{0x9C516C5C,0,0,0},
		{0x9C516C6C,0,0,0},
		{0x9C516C7C,0,0,0},
		{0x9C516C8C,0,0,0},
		{0x9C516C9C,0,0,0},
		{0x9C516CAC,0,0,0},
		{0x9C516CBC,0,0,0},
		{0x9C516CCC,0,0,0},
		{0x9C516CDC,0,0,0},
		{0x9C516CEC,0,0,0},
		{0x9C516CFC,0,0,0},
		{0x9C516CAB,0,0,0},
		{0x9C516CBB,0,0,0},
		{0x9C516CCB,0,0,0},
		{0x079EA519,0,0,0},
		{0x265646F9,0x26564704,0x26564147,0x26564445},
		{0x265646F9,0x26564704,0x26564147,0x26564445},
		{0x265646F9,0x26564704,0x26564147,0x26564445},
		{0x302C1B7C,0,0,0},
		{0x78948798,0,0,0},
		{0x78948798,0,0,0},
		{0x78948798,0,0,0},
		{0xF9465626,0x04475626,0x47415626,0x45445626},
		{0xF9465626,0,0,0},
		{0xFFFFFFFF,0,0,0},
		{0x1143813E,0,0,0},
		{0x1143813E,0,0,0},
		{0x9C516CAC,0,0,0}};



ulong SeedKey(ulong Seed, ulong Mask) {
	ulong Key = 0;
	Key = Seed + Mask;
	return Key;

}

uint32_t calMaskIndex(uint32_t seed) {
	strL2B L2Bseed;
	L2Bseed.L = seed;
	return (L2Bseed.B[3] >> 4) & 0x3;
}

//国5算法
unsigned int __ROR4__(unsigned int value, int shift) {
	shift &= 0x1f; // mod 32
	return shift == 0? value
		: (value >> shift) | (value << (32 - shift));
}

char *revers_array(char *p, int size)
{
  char *i;
  char tmp;

  if ( size > 0 )
  {
    for ( i = &p[size - 1]; p < i; --i )
    {
      tmp = *p;
      *p = *i;
      *i = tmp;
      ++p;
    }
  }
  return p;
}

/*
 * seed is an in/out argument of 4 bytes.
 * return: 4, as the length in bytes.
 */
int security_calculate(ubyte* seed,uint32_t* Mask) {
  unsigned char *v1; // r4@1
  ulong v2; // r1@1
  ulong v3; // r2@1
  unsigned int v4; // r6@1
  ulong v5; // r3@1
  ulong v6; // r1@1
  ulong v7; // r2@4
  ulong v9; // r2@10
  uint32_t v10[4]; // [sp+4h] [bp-2Ch]@1
  ulong v11; // [sp+14h] [bp-1Ch]@7


  v1 = seed;
  /*v10[0] = 24541593;
  v10[1] = 24533000;
  v10[2] = 559305031;
  v10[3] = 291914821;*/
  v10[0] = Mask[0];
  v10[1] = Mask[1];
  v10[2] = Mask[2];
  v10[3] = Mask[3];
  v2 = seed[1];
  v3 = seed[3];
  v4 = (unsigned int)(v3 << 26) >> 30;
  v5 = (seed[0] << 24) | v3 | (v2 << 16) | (seed[2] << 8);
  v6 = v2 & 0xF;
  if ( v3 & 8 )
  {
    if ( v6 && (v3 & 7) != 5 )
    {
      v7 = 0;
      do
      {
        v7 = (v7 + 1) & 0xFF;
        v5 = __ROR4__(v5, 1);
      }
      while ( v7 != v6 && (v5 & 7) != 5 );
    }
  }
  else if ( v6 && (v3 & 7) != 5 )
  {
    v9 = 0;
    do
    {
      v9 = (v9 + 1) & 0xFF;
      v5 = __ROR4__(v5, 31);
    }
    while ( v9 != v6 && (v5 & 7) != 5 );
  }
  v11 = v5 ^ v10[v4];
  revers_array((char *)&v11, 4);
  memcpy(v1, &v11, 4u);
  return 4;
}

ulong CalculateKeyG5(uword EngineModel, ulong Seed) {
	ulong Key = 0;

	uint32_t size = sizeof(maskList) / 0xF;
	if (EngineModel > 0 && EngineModel <= size) {
		uint32_t idx = EngineModel - 1;
		uint32_t *mask = maskList[idx];

		if (mask[0] == 1 && mask[1] == 1 && mask[2] == 1 && mask[3] == 1) { //反转seed为key
			strL2B L2Bseed;
			strL2B L2Btemp;
			L2Btemp.L = Seed;
			L2Bseed.B[0] = L2Btemp.B[3];
			L2Bseed.B[1] = L2Btemp.B[2];
			L2Bseed.B[2] = L2Btemp.B[1];
			L2Bseed.B[3] = L2Btemp.B[0];
			Key = L2Bseed.L;
		} else if (mask[1] == 0 && mask[2] == 0 && mask[3] == 0) {
			if (mask[0] == 0) { //无mask
				Key = 0xAAAAAAAA;
			} else { //单mask
				//Key = genKeyCRIN(Seed, mask[0]);
			}
		} else { //多mask
			//uint32_t offset = calMaskIndex(Seed);
			ubyte seed0 = (ubyte)Seed;
			ubyte seed1 = (ubyte)(Seed>>8);
			ubyte seed2 = (ubyte)(Seed>>16);
			ubyte seed3 = (ubyte)(Seed>>24);
			ubyte bSeed[] = {seed0,seed1,seed2,seed3};
			security_calculate(bSeed, mask);
			Key = (bSeed[3]<<24)+(bSeed[2]<<16)+(bSeed[1]<<8)+bSeed[0];
			return Key;
		}
	}

	return Key;
}

/**
 * BOSCH大车安全算法
 */
uint32_t genKeyCRIN(uint32_t seed, uint32_t mask) {
	strL2B L2Bseed;
	strL2B L2Bmask;
	L2Bseed.L = seed;
	L2Bmask.L = mask;

	for (int i = 35; i > 0; i--) {
		uint8_t b = 0;

		for (int j = 3; j >= 0; j--) {
			if (b) {
				b = ((L2Bseed.B[j] & 0x80) != 0);
				L2Bseed.B[j] = ((L2Bseed.B[j] << 1) + 1);
			} else {
				b = ((L2Bseed.B[j] & 0x80) != 0);
				L2Bseed.B[j] = ((L2Bseed.B[j] << 1));
			}
		}

		if (b) {
			for (int j = 0; j < 4; j++) {
				L2Bseed.B[j] = (L2Bseed.B[j] ^ L2Bmask.B[j]);
			}
		}
	}

	return L2Bseed.L;
}

/**
 * BOSCH小车安全算法
 */
uint32_t genKeyCRI(uint32_t seed, uint32_t mask) {
	strL2B L2Bseed;
	strL2B L2Bmask;
	L2Bseed.L = seed;
	L2Bmask.L = mask;

	int count = L2Bseed.B[1] & 0xF;
	int dir = L2Bseed.B[3] >> 3 & 0x1;

	for (int i = 0; i < count; i++) {
		if (1 == dir) {
			uint8_t bitOfRight = L2Bseed.B[3] & 0x1;
			for (int n = 3; n > 0; --n) {
				uint8_t bitOfPrevByte = L2Bseed.B[n - 1] & 0x1;
				L2Bseed.B[n] = (L2Bseed.B[n] >> 1) | (bitOfPrevByte << 7);
			}
			L2Bseed.B[0] = (L2Bseed.B[0] >> 1) | (bitOfRight << 7);
		} else {
			uint8_t bitOfLeft = (L2Bseed.B[0] >> 7) & 0x1;
			for (int n = 0; n < 3; n++) {
				uint8_t bitOfNextByte = (L2Bseed.B[n + 1] >> 7) & 0x1;
				L2Bseed.B[n] = (L2Bseed.B[n] << 1) | bitOfNextByte;
			}
			L2Bseed.B[3] = (L2Bseed.B[3] << 1) | bitOfLeft;
		}

		uint8_t bitOfRight = L2Bseed.B[3] & 0x7;
		if (bitOfRight == 5) {
			break;
		}
	}

	for (int j = 0; j < 4; j++) {
		L2Bseed.B[j] = (L2Bseed.B[j] ^ L2Bmask.B[j]);
	}

	return L2Bseed.L;
}



ulong CalculateKeyEx(uword EngineModel, ubyte *Seed, ubyte *Key) {
	ulong uPlainTxtsize = sizeof(g_szPlainText);
	ubyte pszKey[100] = { 0 };
	ulong KeyOutSize = 0;
	memcpy(pszKey, g_szPlainText, uPlainTxtsize);
	char cEncryptLevel = 0;
	ulong uSeedSize = 4;
	cEncryptLevel = Seed[0];
	if (cEncryptLevel == 0) {
		memcpy(Key, g_szPlainText, uPlainTxtsize);
		return uPlainTxtsize;
	}

	sub_100D6380(pszKey, uPlainTxtsize, sizeof(pszKey), &KeyOutSize, Seed + 2,
			uSeedSize, cEncryptLevel);
	memcpy(Key, pszKey, KeyOutSize);

	return KeyOutSize;
}

ulong CalculateKey(uword EngineModel, ulong Seed) {
	ulong Key = 0;

	uint32_t size = sizeof(maskList) / 0xF;
	if (EngineModel > 0 && EngineModel <= size) {
		uint32_t idx = EngineModel - 1;
		uint32_t *mask = maskList[idx];

		if (mask[0] == 1 && mask[1] == 1 && mask[2] == 1 && mask[3] == 1)
		{ //反转seed为key
			strL2B L2Bseed;
			strL2B L2Btemp;
			L2Btemp.L = Seed;
			L2Bseed.B[0] = L2Btemp.B[3];
			L2Bseed.B[1] = L2Btemp.B[2];
			L2Bseed.B[2] = L2Btemp.B[1];
			L2Bseed.B[3] = L2Btemp.B[0];
			Key = L2Bseed.L;
		} else if (mask[1] == 0 && mask[2] == 0 && mask[3] == 0)
		{
			if (mask[0] == 0)
			{ //无mask
				Key = 0xAAAAAAAA;
			}else
			{ 	//单mask
				Key = genKeyCRIN(Seed, mask[0]);
			}
		}else
		{ 				//多mask
			uint32_t offset = calMaskIndex(Seed);
			Key = genKeyCRI(Seed,mask[offset]);
		}
	}

	return Key;
}

ulong CalcKeyForWeichai(uword EngineModel, ulong Seed) {
	ulong Key = 0;
	strL2B L2Bkey;
	ulong uSeedSize = sizeof(Seed);
	ubyte pszKey[4] = {0};
	ulong uKeySize = 4;
	ulong pKeyOutSize = 0;

	strL2B L2Bseed;
	L2Bseed.B[0] = (ubyte)((Seed) & 0xFF );
	L2Bseed.B[1] = (ubyte)((Seed >> 8) & 0xFF );
	L2Bseed.B[2] = (ubyte)((Seed >> 16) & 0xFF );
	L2Bseed.B[3] = (ubyte)((Seed >> 24) & 0xFF );

	switch(EngineModel){
		case 1:
			GetKey_WeiChai_nSeednKey_N1(L2Bseed.B,uSeedSize,pszKey,uKeySize,&pKeyOutSize);
			break;
		case 2:
			GetKey_WeiChai_nSeednKey_N2(L2Bseed.B,uSeedSize,pszKey,uKeySize,&pKeyOutSize);
			break;
		case 3:
			GetKey_WeiChai_nSeednKey_N3(L2Bseed.B,uSeedSize,pszKey,uKeySize,&pKeyOutSize);
			break;
		case 4:
			GetKey_WeiChai_nSeednKey_N4(L2Bseed.B,uSeedSize,pszKey,uKeySize,&pKeyOutSize);
			break;
		case 5:
			GetKey_WeiChai_nSeednKey_N5(L2Bseed.B,uSeedSize,pszKey,uKeySize,&pKeyOutSize);
			break;
		default:
			pszKey[0] = 0x11;
			pszKey[1] = 0x11;
			pszKey[2] = 0x11;
			pszKey[3] = 0x11;
			break;
	}

	L2Bkey.B[0] = pszKey[0];
	L2Bkey.B[1] = pszKey[1];
	L2Bkey.B[2] = pszKey[2];
	L2Bkey.B[3] = pszKey[3];
	Key = L2Bkey.L;

	return Key;
}
