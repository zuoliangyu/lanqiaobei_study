#include "main.h"
/* ���������� */
uchar Key_Slow_Down;								 // ��������ר�ñ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
uchar Seg_Pos;										 // �����ɨ��ר�ñ���
uint Seg_Slow_Down;									 // ����ܼ���ר�ñ���
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������

uchar Seg_show_mode; // 0�¶ȣ�1ʱ�䣬2��������

uint int_temperature_value;
uchar ucRtc[3] = {0x01, 0x59, 0x50};
uchar temperature_demo = 23;

bit control_mode;	// 0�¶ȣ�1ʱ��
bit time_show_flag; // 0ʱ�֣�1����
uint time_5s;
uchar time_100ms;
bit time_relay_flag;		// 0�����㣬1����
bit temperature_relay_flag; // 0�ǳ����¶ȣ�1�����¶�

void init_Seg_LED()
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		Seg_Buf[i] = 10;
		Seg_Point[i] = 0;
		ucLed[i] = 0;
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
		init_Seg_LED();
		if (++Seg_show_mode > 2)
			Seg_show_mode = 0;
		break;

	case 13:
		control_mode = ~control_mode;
		break;
	case 16:
		if (Seg_show_mode == 2)
			if (++temperature_demo > 99)
				temperature_demo = 99;
		break;
	case 17:
		if (Seg_show_mode == 2)
			if (--temperature_demo < 10)
				temperature_demo = 10;
		if (Seg_show_mode == 1)
			time_show_flag = 1;
		break;
	}
	if (Key_Up == 17)
		time_show_flag = 0;
}

/* ��Ϣ������ */
void Seg_Proc()
{

	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	Seg_Buf[0] = 11;   // U
	Seg_Buf[1] = Seg_show_mode + 1;
	Seg_Buf[2] = 10; // ��
	switch (Seg_show_mode)
	{
	case 0:
		Seg_Buf[3] = Seg_Buf[4] = 10;
		int_temperature_value = rd_temperature() * 10;
		Seg_Buf[5] = int_temperature_value / 100;
		Seg_Buf[6] = (int_temperature_value / 10) % 10;
		Seg_Buf[7] = int_temperature_value % 10;
		Seg_Point[6] = 1;
		break;
	case 1:
		Read_Rtc(ucRtc);
		Seg_Buf[3] = (time_show_flag) ? ucRtc[1] / 16 : ucRtc[0] / 16;
		Seg_Buf[4] = (time_show_flag) ? ucRtc[1] % 16 : ucRtc[0] % 16;
		Seg_Buf[5] = 12;
		Seg_Buf[6] = (time_show_flag) ? ucRtc[2] / 16 : ucRtc[1] / 16;
		Seg_Buf[7] = (time_show_flag) ? ucRtc[2] % 16 : ucRtc[1] % 16;
		break;
	case 2:
		Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
		Seg_Buf[6] = temperature_demo / 10;
		Seg_Buf[7] = temperature_demo % 10;
		break;
	}
}

/* ������ʾ���� */
void Led_Proc()
{
	if (control_mode)
	{
		temperature_relay_flag = 0; // ��ֹͻȻ�л������¼̵�����
		if (ucRtc[1] / 16 == 0 && ucRtc[1] % 16 == 0 && ucRtc[2] / 16 == 0 && ucRtc[2] % 16 == 0)
		{
			time_relay_flag = 1;
		}
		Relay(time_relay_flag); // ��ʱ�����Ƽ̵���
	}
	else
	{
		time_relay_flag = 0; // ��ֹͻȻ�л������¼̵�����
		if ((float)int_temperature_value / 10.0 > temperature_demo)
			temperature_relay_flag = 1;
		else
			temperature_relay_flag = 0;
		Relay(temperature_relay_flag);
	}
	ucLed[0] = time_relay_flag; // ���������5sϨ��
	ucLed[1] = ~control_mode;	// �¶ȿ���ʱ������ʱ�����Ϩ��
	ucLed[2] = time_100ms;		// 100ms��˸
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
	if (time_relay_flag)
	{
		if (++time_5s == 5000)
		{
			time_5s = 0;
			time_relay_flag = 0;
		}
	}
	// �����̵���
	if (time_relay_flag || temperature_relay_flag)
		if (++time_100ms == 100)
			time_100ms = 0;

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
	Set_Rtc(ucRtc);
	System_Init();
	Timer0Init();
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}