#include "main.h"
/* ���������� */
uchar Key_Slow_Down;								 // ��������ר�ñ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
uchar Seg_Pos;										 // �����ɨ��ר�ñ���
uint Seg_Slow_Down;									 // ����ܼ���ר�ñ���
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������

uchar Seg_show_mode; // 0�¶ȣ�1������2DAC
float temperature_value;
uchar temperature_demo = 25;
uchar DAC_out_digit;
bit DAC_flag; // 0Ϊ���� 1Ϊ�¶ȹ�ϵ
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
	case 4:
		init_Seg();
		Seg_show_mode = (++Seg_show_mode) % 3;
		break;
	case 8:
		if (Seg_show_mode == 1)
			temperature_demo = (--temperature_demo < 0) ? 0 : temperature_demo;
		break;
	case 9:
		if (Seg_show_mode == 1)
			temperature_demo = (++temperature_demo > 99) ? 99 : temperature_demo;
		break;
	case 5:
		DAC_flag ^= 1;
		break;
	}
}

/* ��Ϣ������ */
void Seg_Proc()
{
	uint temp_temperature;
	uint DAC_out_analog_x100;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	temperature_value = rd_temperature();
	temp_temperature = temperature_value * 100;
	if (DAC_flag)
	{
		if (temperature_value >= 40)
			DAC_out_digit = 204; // 4*255/5
		else if (temperature_value <= 20)
			DAC_out_digit = 51; // 1*255/5
		else
			DAC_out_digit = ((temperature_value - 20) * 3.0 / 20.0 + 1.0) * 51;
	}
	DAC_out_analog_x100 = DAC_out_digit / 51.0 * 100;
	switch (Seg_show_mode)
	{
	case 0:
		/* �¶Ƚ��� */
		Seg_Buf[0] = 11; // C
		Seg_Buf[4] = temp_temperature / 1000 % 10;
		Seg_Buf[5] = temp_temperature / 100 % 10;
		Seg_Buf[6] = temp_temperature / 10 % 10;
		Seg_Buf[7] = temp_temperature % 10;
		Seg_Point[5] = 1;
		break;
	case 1:
		/*��������*/
		Seg_Buf[0] = 12; // P
		Seg_Buf[6] = temperature_demo / 10 % 10;
		Seg_Buf[7] = temperature_demo % 10;
		break;
	case 2:
		/*DAC���*/
		Seg_Buf[0] = 13; // A
		Seg_Buf[5] = DAC_out_analog_x100 / 100 % 10;
		Seg_Buf[6] = DAC_out_analog_x100 / 10 % 10;
		Seg_Buf[7] = DAC_out_analog_x100 % 10;
		Seg_Point[5] = 1;
		break;
	}
}

/* ������ʾ���� */
void Led_Proc()
{
	Da_Write(DAC_out_digit);
	ucLed[0] = !DAC_flag;
	ucLed[1] = (Seg_show_mode == 0);
	ucLed[2] = (Seg_show_mode == 1);
	ucLed[3] = (Seg_show_mode == 2);
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

	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}