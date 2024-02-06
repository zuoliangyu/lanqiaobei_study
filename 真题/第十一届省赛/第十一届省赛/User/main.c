#include "main.h"
/* ���������� */
uchar Key_Slow_Down;								 // ��������ר�ñ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
uchar Seg_Pos;										 // �����ɨ��ר�ñ���
uint Seg_Slow_Down;									 // ����ܼ���ר�ñ���
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������

/* ������ʾ */
uchar Seg_show_mode; // 0���� 1���� 2����

uint old_vol;			  // ��һ�εĵ�ѹֵ
uchar vol_demo;			  // �ο���ѹ0-50
unsigned long count_down; // �½��ؼ���
bit count_down_flag;	  // �½��ؼ�����־λ
/* ʱ�� */
uint time_5s;

uchar error_count;
void init_Seg()
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		Seg_Buf[i] = 10;
		Seg_Point[i] = 0;
	}
}
/* ���̴����� */
void Key_Proc()
{
	static uchar Key_Val, Key_Down, Key_Old, Key_Up; // ����ר�ñ���
	if (Key_Slow_Down)
		return;
	Key_Slow_Down = 1; // ���̼��ٳ���

	Key_Val = Key_Read();					  // ʵʱ��ȡ����ֵ
	Key_Down = Key_Val & (Key_Old ^ Key_Val); // ��׽�����½���
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // ��׽�����Ͻ���
	Key_Old = Key_Val;						  // ����ɨ�����
	switch (Key_Down)
	{
	case 12:
		if (Seg_show_mode == 0)
		{
			EEPROM_Write(&vol_demo, 0, 1);
		}
		Seg_show_mode = (++Seg_show_mode) % 3;
		error_count = 0;
		break;
	case 16:
		if (Seg_show_mode == 1)
			vol_demo = (vol_demo + 5 > 50 ? 0 : vol_demo + 5);
		error_count = 0;
		break;
	case 17:
		if (Seg_show_mode == 1)
			vol_demo = (vol_demo - 5 < 0 ? 50 : vol_demo - 5);
		error_count = 0;
		break;
	case 13:
		if (Seg_show_mode == 2)
			count_down = 0;
		error_count = 0;
		break;
	default:
		error_count = (++error_count >= 3) ? 3 : error_count;
		break;
	}
}

/* ��Ϣ������ */
void Seg_Proc()
{
	uint real_V;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	switch (Seg_show_mode)
	{
	case 0:
		/* ������ʾ */
		Seg_Buf[0] = 11;				   // U
		real_V = Ad_Read(0x03) * 100 / 51; // 0-255->0-500
		// ����⵽�½��ص�ʱ�����+1
		if (old_vol > vol_demo * 10 && real_V < vol_demo * 10)

			count_down++;
		if (real_V < vol_demo * 10)
			count_down_flag = 1;
		else
			count_down_flag = 0;
		old_vol = real_V;
		Seg_Buf[5] = real_V / 100 % 10; // ��λ
		Seg_Buf[6] = real_V % 100 / 10; // ʮλ
		Seg_Buf[7] = real_V % 10;		// ��λ
		Seg_Point[5] = 1;
		break;

	case 1:
		/*��������*/
		Seg_Buf[0] = 12;				  // P
		Seg_Buf[5] = vol_demo / 100 % 10; // ��λ
		Seg_Buf[6] = vol_demo / 10 % 10;  // ʮλ
		Seg_Buf[7] = vol_demo % 10;		  // ��λ
		Seg_Point[5] = 1;
		break;
	case 2:
		/*��������*/
		Seg_Buf[0] = 13; // N
		Seg_Buf[1] = count_down / 10000000 % 10;
		Seg_Buf[2] = count_down / 1000000 % 10;
		Seg_Buf[3] = count_down / 100000 % 10;
		Seg_Buf[4] = count_down / 10000 % 10;
		Seg_Buf[5] = count_down / 1000 % 10;
		Seg_Buf[6] = count_down / 100 % 10;
		Seg_Buf[7] = count_down % 10;
		break;
	}
}

/* ������ʾ���� */
void Led_Proc()
{
	ucLed[0] = (time_5s >= 5000);
	ucLed[1] = (count_down % 2);
	ucLed[2] = (error_count >= 3);
}

/* ��ʱ��0�жϳ�ʼ������ */
void Timer0Init(void) // 1����@12.000MHz
{
	AUXR &= 0x7F; // ��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0; // ���ö�ʱ��ģʽ
	TL0 = 0x18;	  // ���ö�ʱ��ʼֵ
	TH0 = 0xFC;	  // ���ö�ʱ��ʼֵ
	TF0 = 0;	  // ���TF0��־
	TR0 = 1;	  // ��ʱ��0��ʼ��ʱ
	ET0 = 1;	  // ��ʱ���ж�0��
	EA = 1;		  // ���жϴ�
}

/* ��ʱ��0�жϷ����� */
void Timer0Server() interrupt 1
{
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // ���̼���ר��
	if (++Seg_Slow_Down == 500)
		Seg_Slow_Down = 0; // ����ܼ���ר��
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // �������ʾר��
	if (count_down_flag)
	{
		if (++time_5s >= 5000)
			time_5s = 5000;
	}
	else
		time_5s = 0;
	Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

void Delay750ms() //@12MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 35;
	j = 51;
	k = 182;
	do
	{
		do
		{
			while (--k)
				;
		} while (--j);
	} while (--i);
}

/* Main */
void main()
{
	// ������¶ȶ�ȡ�Ļ�
	rd_temperature();
	Delay750ms();

	System_Init();
	Timer0Init();
	EEPROM_Read(&vol_demo, 0, 1);
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}