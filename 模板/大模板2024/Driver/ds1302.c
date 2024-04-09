#include "ds1302.h"
#include <intrins.h>

sbit SCK = P1 ^ 7;
sbit SDA = P2 ^ 3;
sbit RST = P1 ^ 3;
/*	# 	DS1302����Ƭ��˵��
	1. 	���ļ������ṩ���������빩����ѡ����ɳ�����Ʋο���
	2. 	����ѡ�ֿ������б�д��ش�����Ըô���Ϊ������������ѡ��Ƭ�����͡������ٶȺ�����
		�жԵ�Ƭ��ʱ��Ƶ�ʵ�Ҫ�󣬽��д�����Ժ��޸ġ�
*/

//
void Write_Ds1302(unsigned char temp)
{
	unsigned char i;
	for (i = 0; i < 8; i++)
	{
		SCK = 0;
		SDA = temp & 0x01;
		temp >>= 1;
		SCK = 1;
	}
}

//
void Write_Ds1302_Byte(unsigned char address, unsigned char dat)
{
	RST = 0;
	_nop_();
	SCK = 0;
	_nop_();
	RST = 1;
	_nop_();
	Write_Ds1302(address);
	Write_Ds1302(dat);
	RST = 0;
}

//
unsigned char Read_Ds1302_Byte(unsigned char address)
{
	unsigned char i, temp = 0x00;
	RST = 0;
	_nop_();
	SCK = 0;
	_nop_();
	RST = 1;
	_nop_();
	Write_Ds1302(address);
	for (i = 0; i < 8; i++)
	{
		SCK = 0;
		temp >>= 1;
		if (SDA)
			temp |= 0x80;
		SCK = 1;
	}
	RST = 0;
	_nop_();
	SCK = 0;
	_nop_();
	SCK = 1;
	_nop_();
	SDA = 0;
	_nop_();
	SDA = 1;
	_nop_();
	return (temp);
}

void Set_Rtc(unsigned char *ucRtc)
{
	unsigned char i;
	Write_Ds1302_Byte(0x8e, 0x00);
	for (i = 0; i < 3; i++)
		Write_Ds1302_Byte(0x84 - 2 * i, ucRtc[i] / 10 % 10 << 4 | ucRtc[i] % 10);
	Write_Ds1302_Byte(0x8e, 0x80);
}

void Read_Rtc(unsigned char *ucRtc)
{
	unsigned char i;
	unsigned temp;
	for (i = 0; i < 3; i++)
	{
		temp = Read_Ds1302_Byte(0x85 - 2 * i);
		ucRtc[i] = temp / 16 * 10 + temp % 16;
	}
}