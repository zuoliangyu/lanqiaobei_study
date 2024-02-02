#include "main.h"
/* ���������� */
uchar Key_Slow_Down;								 // ��������ר�ñ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
uchar Seg_Pos;										 // �����ɨ��ר�ñ���
uint Seg_Slow_Down;									 // ����ܼ���ר�ñ���
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������

uchar Seg_show_mode; // ��ʾ���� 0Ƶ�� 1ʪ�� 2��� 3����
uchar celi_mode;	 // �������� 0Ƶ�� 1ʪ�� 2����
/*ʱ��*/
uint time_1s;
uint time_up_1s;
uchar time_100ms;
uchar pwn_count;
/*��������*/
uint freq;
uchar humidity;
uchar distance;

uint freq_demo = 9000;
uint humidity_demo = 40;
uint dis_demo = 60;
bit dis_flag;  // 0Ϊcm��1Ϊm
bit freq_flag; // 0ΪHz��1ΪkHz

bit reset_relay_flag;
uchar relay_count;
bit light_flag;
void init_Seg_LED() // ��ʼ������ܺ�LED
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		ucLed[i] = 0;
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
		Seg_show_mode = (++Seg_show_mode) % 4;
		celi_mode = 0;
		init_Seg_LED();
	}
	if (Seg_show_mode == 3 && Key_Down == 5)
	{
		celi_mode = (++celi_mode) % 3;
		init_Seg_LED();
	}
	if (Key_Down == 6)
	{
		switch (Seg_show_mode)
		{
		case 2:
			dis_flag ^= 1;
			Seg_Point[5] = 0;
			break;
		case 3:
			/* �������� */
			switch (celi_mode)
			{
			case 0:
				/* Ƶ�ʲ��� */
				freq_demo = (freq_demo + 500 > 12000) ? 1000 : freq_demo + 500;
				break;
			case 1:
				/*ʪ�Ȳ���*/
				humidity_demo = (humidity_demo + 10 > 60) ? 10 : humidity_demo + 10;
				break;
			case 2:
				/*�������*/
				dis_demo = (dis_demo + 10 > 120) ? 10 : dis_demo + 10;
				break;
			}
			break;
		}
	}
	if (Key_Down == 7)
	{
		switch (Seg_show_mode)
		{
		case 0:
			/* Ƶ����ʾ */
			freq_flag ^= 1;
			Seg_Point[6] = 0;
			break;
		case 3:
			/* �������� */
			switch (celi_mode)
			{
			case 0:
				/* Ƶ�ʲ��� */
				freq_demo = (freq_demo - 500 < 1000) ? 12000 : freq_demo - 500;
				break;
			case 1:
				/*ʪ�Ȳ���*/
				humidity_demo = (humidity_demo - 10 < 10) ? 60 : humidity_demo - 10;
				break;
			case 2:
				/*�������*/
				dis_demo = (dis_demo - 10 < 10) ? 120 : dis_demo - 10;
				break;
			}
			break;
		}
	}
	if (Key_Down == 7 && Seg_show_mode == 1)
		reset_relay_flag = 1; // ���¿�ʼ��ʱ
	else
		reset_relay_flag = 0;
	// ̧����
	if (Key_Up == 7 && time_up_1s >= 1000)
		reset_relay_flag = 0;
}

/* ��Ϣ������ */
void Seg_Proc()
{
	uchar temp_freq, i, temp_dis;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	humidity = Ad_Read(0x03) / 51.0 * 20;
	/*ʪ�ȴ���*/
	if (humidity > 80)
		Da_Write(255);
	else if (humidity < humidity_demo)
		Da_Write(51);
	else
		Da_Write(255 - 204 * (80 - humidity) / (80 - humidity_demo));
	/*��ദ��*/
	distance = Ut_Wave_Data();
	if (distance > dis_demo)
	{
		Relay(1);
		relay_count++;
		EEPROM_Write(&relay_count, 0, 1);
	}
	else
	{
		Relay(0);
	}
	switch (Seg_show_mode)
	{
	case 0:
		/* Ƶ����ʾ */
		Seg_Buf[0] = 11; // F
		// KHz
		if (freq_flag)
		{
			temp_freq = freq / 100;
			Seg_Buf[3] = temp_freq / 10000 % 10;
			Seg_Buf[4] = temp_freq / 1000 % 10;
			Seg_Buf[5] = temp_freq / 100 % 10;
			Seg_Buf[6] = temp_freq / 10 % 10;
			Seg_Buf[7] = temp_freq % 10;
			Seg_Point[6] = 1;
			for (i = 3; i < 6; i++)
			{
				if (Seg_Buf[i] == 0)
					Seg_Buf[i] = 10;
				else
					break;
			}
		}
		// Hz
		else
		{
			Seg_Buf[3] = freq / 10000 % 10;
			Seg_Buf[4] = freq / 1000 % 10;
			Seg_Buf[5] = freq / 100 % 10;
			Seg_Buf[6] = freq / 10 % 10;
			Seg_Buf[7] = freq % 10;
			for (i = 3; i < 7; i++)
			{
				if (Seg_Buf[i] == 0)
					Seg_Buf[i] = 10;
				else
					break;
			}
		}
		break;
	case 1:
		/*ʪ����ʾ*/
		Seg_Buf[0] = 12; // H
		Seg_Buf[6] = humidity / 10 % 10;
		Seg_Buf[7] = humidity % 10;
		break;
	case 2:
		/*���*/
		Seg_Buf[0] = 13; // A
		if (dis_flag)
		{
			Seg_Buf[5] = distance / 100;
			Seg_Buf[6] = distance / 10 % 10;
			Seg_Buf[7] = distance % 10;
			Seg_Point[5] = 1;
		}
		else
		{
			Seg_Buf[5] = (distance / 100 == 0) ? 10 : distance / 100;
			Seg_Buf[6] = distance / 10 % 10;
			Seg_Buf[7] = distance % 10;
		}

		break;
	case 3:
		/*����*/
		Seg_Buf[0] = 14; // P
		Seg_Buf[1] = celi_mode + 1;
		switch (celi_mode)
		{
		case 0:
			/* Ƶ�� */
			temp_freq = freq_demo / 100;
			Seg_Buf[5] = (temp_freq / 100 == 0) ? 10 : temp_freq / 100;
			Seg_Buf[6] = temp_freq / 10 % 10;
			Seg_Buf[7] = temp_freq % 10;
			Seg_Point[6] = 1;
			break;
		case 1:
			/* ʪ�� */
			Seg_Buf[6] = humidity_demo / 10;
			Seg_Buf[7] = humidity_demo % 10;
			break;
		case 2:
			/* ���� */
			temp_dis = dis_demo / 10;
			Seg_Buf[6] = temp_dis / 10;
			Seg_Buf[7] = temp_dis % 10;
			Seg_Point[6] = 1;
			break;
		}
		break;
	}
}

/* ������ʾ���� */
void Led_Proc()
{

	switch (Seg_show_mode)
	{
	case 0:
		ucLed[0] = 1;
		break;
	case 1:
		ucLed[1] = 1;
		break;
	case 2:
		ucLed[2] = 1;
		break;
	case 3:
		ucLed[0] = (celi_mode == 0) ? light_flag : 0;
		ucLed[1] = (celi_mode == 1) ? light_flag : 0;
		ucLed[2] = (celi_mode == 2) ? light_flag : 0;
		break;
	}
	ucLed[3] = (freq > freq_demo);
	ucLed[4] = (humidity > humidity_demo);
	ucLed[5] = (distance > dis_demo);
}

/* ��ʱ��0�жϳ�ʼ������ */
void Timer0Init(void) // 1����@12.000MHz
{
	AUXR &= 0x7F; // ��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0; // ���ö�ʱ��ģʽ
	TMOD |= 0x05; // ���ö�ʱ��0Ϊ16λ������
	TL0 = 0;	  // ���ö�ʱ��ʼֵ
	TH0 = 0;	  // ���ö�ʱ��ʼֵ
	TF0 = 0;	  // ���TF0��־
	TR0 = 1;	  // ��ʱ��0��ʼ��ʱ
}

void Timer2_Init(void) // 1����@12.000MHz
{
	AUXR &= 0xFB; // ��ʱ��ʱ��12Tģʽ
	T2L = 0x18;	  // ���ö�ʱ��ʼֵ
	T2H = 0xFC;	  // ���ö�ʱ��ʼֵ
	AUXR |= 0x10; // ��ʱ��2��ʼ��ʱ
	IE2 |= 0x04;  // ����ʱ��2�ж�
	EA = 1;		  // ���жϴ�
}
void Timer3_Init(void) // 100΢��@12.000MHz
{
	T4T3M &= 0xFD; // ��ʱ��ʱ��12Tģʽ
	T3L = 0x9C;	   // ���ö�ʱ��ʼֵ
	T3H = 0xFF;	   // ���ö�ʱ��ʼֵ
	T4T3M |= 0x08; // ��ʱ��3��ʼ��ʱ
	IE2 |= 0x20;   // ����ʱ��3�ж�
	EA = 1;		   // ���жϴ�
}

/* ��ʱ��2�жϷ����� */
void Timer2Server() interrupt 12
{
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // ���̼���ר��
	if (++Seg_Slow_Down == 500)
		Seg_Slow_Down = 0; // ����ܼ���ר��
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // �������ʾר��
	if (++time_1s == 1000)
	{
		freq = TH0 << 8 | TL0;
		TH0 = 0;
		TL0 = 0;
		time_1s = 0;
	}
	if (reset_relay_flag)
	{
		if (++time_up_1s >= 1000)
		{
			time_up_1s = 1000;
		}
	}
	else
	{
		time_up_1s = 0;
	}
	if (++time_100ms == 100)
		light_flag ^= 1;
	Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}
void Timer3Server() interrupt 19
{
	if (++pwn_count == 10)
		pwn_count = 0;
	if (freq > freq_demo)
		MOTOR(pwn_count <= 8);
	else
		MOTOR(pwn_count <= 2);
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
	System_Init();
	EEPROM_Read(&relay_count, 0, 1);
	Timer0Init();
	Timer2_Init();
	Timer3_Init();
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}