#include "main.h"
/* ���������� */
uchar Key_Slow_Down;								 // ��������ר�ñ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
uchar Seg_Pos;										 // �����ɨ��ר�ñ���
uint Seg_Slow_Down;									 // ����ܼ���ר�ñ���
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������
uchar Uart_Slow_Down;								 // ���ڼ���ר�ñ���
uchar Seg_show_mode;								 // 0��ѹ 1Ƶ��
uint voltage_value_100x;
uchar DAC_value = 102; // 2*51
uint frequency_value;
uint time_1s;
bit output_mode; // 0�̶�DAC=2, 1DAC=RB2
bit LED_mode;	 // 0�������� 1�رչ���
bit Seg_mode;	 // 0��������� 1�ر������
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
	case 4:
		Seg_show_mode = (++Seg_show_mode) % 2;
		break;
	case 5:
		output_mode ^= 1;
		break;
	case 6:
		LED_mode ^= 1;
		break;
	case 7:
		Seg_mode ^= 1;
		break;
	}
}
/* ��Ϣ������ */
void Seg_Proc()
{
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���

	voltage_value_100x = Ad_Read(0x43);
	if (!output_mode)
		DAC_value = voltage_value_100x;
	else
		DAC_value = 102;
	if (!output_mode)
		Da_Write(DAC_value);
	else
		Da_Write(DAC_value);
	voltage_value_100x = voltage_value_100x * 100 / 51;
	if (!Seg_mode)
	{
		switch (Seg_show_mode)
		{
		case 0:
			/* ��ѹ���� */
			Seg_Buf[0] = 11; // U
			Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
			Seg_Buf[5] = voltage_value_100x / 100;
			Seg_Buf[6] = voltage_value_100x / 10 % 10;
			Seg_Buf[7] = voltage_value_100x % 10;
			Seg_Point[5] = 1;
			break;
		case 1:
			/* Ƶ�ʲ��� */
			Seg_Buf[0] = 12; // F
			Seg_Buf[1] = 10;
			Seg_Buf[2] = (frequency_value / 100000 == 0) ? 10 : frequency_value / 100000;
			Seg_Buf[3] = ((frequency_value / 10000 % 10 == 0) && (Seg_Buf[2] == 10)) ? 10 : frequency_value / 10000 % 10;
			Seg_Buf[4] = ((frequency_value / 1000 % 10 == 0) && (Seg_Buf[3] == 10)) ? 10 : frequency_value / 1000 % 10;
			Seg_Buf[5] = ((frequency_value / 100 % 10 == 0) && (Seg_Buf[4] == 10)) ? 10 : frequency_value / 100 % 10;
			Seg_Buf[6] = ((frequency_value / 10 % 10 == 0) && (Seg_Buf[5] == 10)) ? 10 : frequency_value / 10 % 10;
			Seg_Buf[7] = frequency_value % 10;
			Seg_Point[5] = 0;
			break;
		}
	}
	else
	{
		uchar i;
		for (i = 0; i < 8; i++)
		{
			Seg_Buf[i] = 10;
			Seg_Point[i] = 0;
		}
	}
}

/* ������ʾ���� */
void Led_Proc()
{
	if (!LED_mode)
	{
		ucLed[0] = (Seg_show_mode == 0);
		ucLed[1] = (Seg_show_mode == 1);
		ucLed[2] = ((voltage_value_100x < 250 && voltage_value_100x >= 150) ||
					(voltage_value_100x >= 350));
		ucLed[3] = ((frequency_value < 5000 && frequency_value >= 1000) ||
					frequency_value >= 10000);
		ucLed[4] = (output_mode);
	}
	else
	{
		uchar i;
		for (i = 0; i < 8; i++)
		{
			ucLed[i] = 0;
		}
	}
}

// ������
void Timer0Init(void) // 0΢��@12.000MHz
{
	AUXR &= 0x7F; // ��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0; // ���ö�ʱ��ģʽ
	TMOD |= 0x05;
	TL0 = 0x00; // ���ö�ʱ��ֵ
	TH0 = 0x00; // ���ö�ʱ��ֵ
	TF0 = 0;	// ���TF0��־
	TR0 = 1;	// ��ʱ��0��ʼ��ʱ
}
// ��ʱ��
void Timer1Init(void) // 1����@12.000MHz
{
	AUXR &= 0xBF; // ��ʱ��ʱ��12Tģʽ
	TMOD &= 0x0F; // ���ö�ʱ��ģʽ
	TL1 = 0x18;	  // ���ö�ʱ��ֵ
	TH1 = 0xFC;	  // ���ö�ʱ��ֵ
	TF1 = 0;	  // ���TF1��־
	TR1 = 1;	  // ��ʱ��1��ʼ��ʱ
	ET1 = 1;	  // ����ʱ��1�ж�
	EA = 1;		  // �������ж�
}

/* ��ʱ��1�жϷ����� */
void Timer1Server() interrupt 3
{
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // ���̼���ר��
	if (++Seg_Slow_Down == 200)
		Seg_Slow_Down = 0; // ����ܼ���ר��
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // �������ʾר��
	if (++time_1s == 1000)
	{
		time_1s = 0;
		frequency_value = TH0 << 8 | TL0;
		TH0 = TL0 = 0;
		TF0 = 0;
	}
	Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

/* Main */
void main()
{
	System_Init();
	Timer0Init();
	Timer1Init();
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}