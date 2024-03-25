#include "main.h"
/* ���������� */
uchar Key_Slow_Down;								 // ��������ר�ñ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
uchar Seg_Pos;										 // �����ɨ��ר�ñ���
uint Seg_Slow_Down;									 // ����ܼ���ר�ñ���
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������
uchar ucRtc[3] = {0x16, 0x59, 0x50};				 // ʱ����
/* ���� */
uint light_100x;
uint temperature_10x;
uchar para_hour = 17;		 // ʱ�����
uchar para_temperature = 25; // �¶Ȳ���
uchar para_led = 4;

/* �������ʾģʽ */
uchar Seg_show_mode;  // 0���ݽ��� 1��������
uchar data_show_mode; // 0ʱ�� 1�¶� 2����
uchar para_show_mode; // 0ʱ�� 1�¶� 2ָʾ��

/* ʱ�� */
uint time_dark_3s;
uint time_light_3s;
bit dark_flag; // 0�� 1��
void init_Seg(uchar start, uchar end)
{
	uchar i;
	for (i = start; i < end; i++)
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
	if (Key_Down == 4)
	{
		init_Seg(0, 8);
		Seg_show_mode = (++Seg_show_mode) % 2;
		data_show_mode = para_show_mode = 0;
	}
	if (Key_Down == 5)
	{
		if (Seg_show_mode == 0)
		{
			init_Seg(0, 8);
			data_show_mode = (++data_show_mode) % 3;
		}
		else
		{
			init_Seg(2, 8);
			para_show_mode = (++para_show_mode) % 3;
		}
	}
	if (Seg_show_mode == 1)
	{
		if (Key_Down == 8)
		{
			switch (para_show_mode)
			{
			case 0:
				para_hour = (--para_hour >= 0) ? para_hour : 0;
				break;
			case 1:
				para_temperature = (--para_temperature >= 0) ? para_temperature : 0;
				break;
			case 2:
				para_led = (--para_led >= 4) ? para_led : 4;
			}
		}
		if (Key_Down == 9)
		{
			switch (para_show_mode)
			{
			case 0:
				para_hour = (++para_hour <= 23) ? para_hour : 23;
				break;
			case 1:
				para_temperature = (++para_temperature <= 99) ? para_temperature : 99;
				break;
			case 2:
				para_led = (++para_led <= 8) ? para_led : 8;
			}
		}
	}
}

/* ��Ϣ������ */
void Seg_Proc()
{
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	Read_Rtc(ucRtc);   // ��ȡʵʱʱ��
	temperature_10x = rd_temperature() * 10;
	light_100x = Ad_Read(0x01) * 100 / 51;
	dark_flag = (light_100x < 50) ? 1 : 0;
	switch (Seg_show_mode)
	{
	case 0:
		/* ���ݽ��� */
		switch (data_show_mode)
		{
		case 0:
			/* ʱ������ */
			Seg_Buf[0] = ucRtc[0] / 16;
			Seg_Buf[1] = ucRtc[0] % 16;
			Seg_Buf[2] = 11; //-
			Seg_Buf[3] = ucRtc[1] / 16;
			Seg_Buf[4] = ucRtc[1] % 16;
			Seg_Buf[5] = 11; //-
			Seg_Buf[6] = ucRtc[2] / 16;
			Seg_Buf[7] = ucRtc[2] % 16;
			break;

		case 1:
			/* �¶���ʾ */
			Seg_Buf[0] = 12; // C
			Seg_Buf[5] = temperature_10x / 100 % 10;
			Seg_Buf[6] = temperature_10x / 10 % 10;
			Seg_Buf[7] = temperature_10x % 10;
			Seg_Point[6] = 1;
			break;
		case 2:
			/* ������ʾ */
			Seg_Buf[0] = 13; // E
			Seg_Buf[2] = light_100x / 100 % 10;
			Seg_Buf[3] = light_100x / 10 % 10;
			Seg_Buf[4] = light_100x % 10;
			Seg_Point[2] = 1;
			Seg_Buf[7] = dark_flag;
			break;
		}
		break;

	case 1:
		/* �������� */
		Seg_Buf[0] = 14; // P
		Seg_Buf[1] = para_show_mode + 1;
		switch (para_show_mode)
		{
		case 0:
			/* ʱ����� */
			Seg_Buf[6] = para_hour / 10 % 10;
			Seg_Buf[7] = para_hour % 10;
			break;
		case 1:
			/* �¶Ȳ��� */
			Seg_Buf[6] = para_temperature / 10 % 10;
			Seg_Buf[7] = para_temperature % 10;
			break;
		case 2:
			/* ָʾ�Ʋ��� */
			Seg_Buf[7] = para_led % 10;
		}
		break;
	}
}

/* ������ʾ���� */
void Led_Proc()
{
	uchar hour;
	hour = ucLed[0] / 16 * 10 + ucLed[0] % 16;
	ucLed[0] = ((para_hour <= hour) && (hour < 8 + 24));
	ucLed[1] = (temperature_10x < para_temperature * 10);
	if (dark_flag && time_dark_3s >= 3000)
	{
		ucLed[2] = 1;
	}
	else if (!dark_flag && time_light_3s >= 3000)
	{
		ucLed[2] = 0;
	}
	if (dark_flag)
		ucLed[para_led - 1] = 1;
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
	if (dark_flag)	 // ���ڵ�
	{
		time_light_3s = 0;
		if (++time_dark_3s >= 3000)
			time_dark_3s = 3001;
	}
	else // δ���ڵ�
	{
		time_dark_3s = 0;
		if (++time_light_3s >= 3000)
			time_light_3s = 3001;
	}
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
	Set_Rtc(ucRtc);
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}