#include "main.h"
/* ���������� */
uchar Key_Slow_Down;								 // ��������ר�ñ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
uchar Seg_Pos;										 // �����ɨ��ר�ñ���
uint Seg_Slow_Down;									 // ����ܼ���ר�ñ���
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������

/* ��ʾ */
uchar Seg_Show_mode;  // 0 ��� 1 ���� 2 ����
uchar dis_show_mode;  // 0 cm 1 m
uchar para_show_mode; // 0 ���� 1 �¶�
uchar fac_show_mode;  // 0 У׼ֵ 1 ���� 2 DAC���

/* ���� */
uint temperature_data_10x; // 10�����¶�ֵ
uint distance_data;		   // ����ֵ �洢��ʱ�򣬵�λΪcm
uint write_distance_data;  // ��¼����ֵ �洢��ʱ�򣬵�λΪcm
uchar para_temperature;	   // �¶Ȳ���
uchar para_distance;	   // �������
uchar dac_output_data;	   // DAC���ֵ

char calibration_value;	  // У׼ֵ
uint transmission_speed;  // �����ٶ�
uchar dac_low_output_10x; // DAC�������*10

/* �ж� */
bit write_distance_data_flag; // ����д���־
bit reset_flag;				  // ��λ��־
bit led_show_flag;			  // Led��ʾ��־
/* ʱ�� */
uint time_6s;
uint time_2s;
uchar time_100ms;
void set_Seg_init(uchar start, uchar end)
{
	uchar i;
	for (i = start; i < end; i++)
	{
		Seg_Buf[i] = 10;
		Seg_Point[i] = 0;
	}
}
void reset_data()
{
	Seg_Show_mode = 0;
	dis_show_mode = 0;
	para_distance = 40;
	para_temperature = 30;
	calibration_value = 0;
	transmission_speed = 340;
	dac_low_output_10x = 10;
}
/* ���̴����� */
void Key_Proc()
{
	static uchar Key_Val, Key_Down, Key_Old, Key_Up; // ����ר�ñ���
	float temp_V;
	if (Key_Slow_Down)
		return;
	Key_Slow_Down = 1; // ���̼��ٳ���

	Key_Val = Key_Read();					  // ʵʱ��ȡ����ֵ
	Key_Down = Key_Val & (Key_Old ^ Key_Val); // ��׽�����½���
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // ��׽�����Ͻ���
	Key_Old = Key_Val;						  // ����ɨ�����
	// û�н���д�룬ȫ��������Ч
	if (!write_distance_data_flag)
	{
		if (Key_Down == 4)
		{
			Seg_Show_mode = (++Seg_Show_mode) % 3;
			set_Seg_init(0, 8);
			dis_show_mode = para_show_mode = fac_show_mode = 0;
		}
		if (Key_Down == 5)
		{
			switch (Seg_Show_mode)
			{
			case 0:
				dis_show_mode = (++dis_show_mode) % 2;
				set_Seg_init(4, 8);
				break;

			case 1:
				para_show_mode = (++para_show_mode) % 2;
				break;
			case 2:
				fac_show_mode = (++fac_show_mode) % 3;
				set_Seg_init(2, 8);
				break;
			}
		}
		switch (Seg_Show_mode)
		{
		case 0:
			/* ������ */
			if (Key_Down == 8)
			{
				write_distance_data = distance_data;
				write_distance_data_flag = 1;
			}
			else if (Key_Down == 9 && write_distance_data != 0)
			{
				if (write_distance_data < 10)
				{
					dac_output_data = dac_low_output_10x * 51 / 10;
				}
				else if (write_distance_data > 90)
				{
					dac_output_data = 255;
				}
				else
				{
					temp_V = (5 - dac_low_output_10x / 10.0) * (write_distance_data - 90) / 80 + 5;
					dac_output_data = temp_V * 51;
				}
				Da_Write(dac_output_data); // ������û�н������ݼ�¼��ʱ�������һֱ��0
			}
			break;

		case 1:
			/* �������� */
			if (Key_Down == 8)
			{
				switch (para_show_mode)
				{
				case 0:
					para_distance = (para_distance + 10 == 90) ? 90 : para_distance + 10;
					break;
				case 1:
					para_temperature = (++para_temperature == 90) ? 90 : para_temperature;
					break;
				}
			}
			else if (Key_Down == 9)
			{

				switch (para_show_mode)
				{
				case 0:
					para_distance = (para_distance - 10 == 10) ? 10 : para_distance - 10;
					break;
				case 1:
					para_temperature = (--para_temperature == 0) ? 0 : para_temperature;
					break;
				}
			}
			break;

		case 2:
			/* ����ģʽ */
			if (Key_Down == 8)
			{
				switch (fac_show_mode)
				{
				case 0:
					calibration_value = (calibration_value + 5 == 90) ? 90 : calibration_value + 5;
					break;
				case 1:
					transmission_speed = (transmission_speed + 10 == 9990) ? 9990 : transmission_speed + 10;
					break;
				case 2:
					dac_low_output_10x = (++dac_low_output_10x == 90) ? 20 : dac_low_output_10x;
					break;
				}
			}
			else if (Key_Down == 9)
			{
				switch (fac_show_mode)
				{
				case 0:
					calibration_value = (calibration_value - 5 == -90) ? -90 : calibration_value - 5;
					break;
				case 1:
					transmission_speed = (transmission_speed - 10 == 10) ? 10 : transmission_speed - 10;
					break;
				case 2:
					dac_low_output_10x = (--dac_low_output_10x == 1) ? 1 : dac_low_output_10x;
					break;
				}
			}
			break;
		}
		reset_flag = (Key_Down == 89);
	}
}

/* ��Ϣ������ */
void Seg_Proc()
{
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	switch (Seg_Show_mode)
	{
	case 0:
		temperature_data_10x = rd_temperature() * 10;
		Seg_Buf[0] = temperature_data_10x / 100 % 10;
		Seg_Buf[1] = temperature_data_10x / 10 % 10;
		Seg_Buf[2] = temperature_data_10x % 10;
		Seg_Point[1] = 1;
		Seg_Buf[3] = 11; // -
		distance_data = Ut_Wave_Data(calibration_value, transmission_speed);
		/* ������ */
		switch (dis_show_mode)
		{
		case 0:
			/* cm */
			Seg_Buf[4] = (distance_data / 1000 % 10 == 0) ? 10 : distance_data / 1000 % 10;
			Seg_Buf[5] = ((distance_data / 100 % 10 == 0) && (Seg_Buf[4] == 10)) ? 10 : distance_data / 100 % 10;
			Seg_Buf[6] = ((distance_data / 10 % 10 == 0) && (Seg_Buf[5] == 10)) ? 10 : distance_data / 10 % 10;
			Seg_Buf[7] = distance_data % 10;
			break;
		case 1:
			/* m */
			Seg_Buf[4] = (distance_data / 1000 % 10 == 0) ? 10 : distance_data / 1000 % 10;
			Seg_Buf[5] = distance_data / 100 % 10;
			Seg_Buf[6] = distance_data / 10 % 10;
			Seg_Buf[7] = distance_data % 10;
			Seg_Point[5] = 1;
			break;
		}
		break;

	case 1:
		/* �������� */
		Seg_Buf[0] = 12; // P
		Seg_Buf[1] = para_show_mode + 1;
		switch (para_show_mode)
		{
		case 0:
			/* ���� */
			Seg_Buf[6] = para_distance / 10 % 10;
			Seg_Buf[7] = para_distance % 10;
			break;
		case 1:
			/* �¶� */
			Seg_Buf[6] = para_temperature / 10 % 10;
			Seg_Buf[7] = para_temperature % 10;
			break;
		}
		break;

	case 2:
		/* ����ģʽ */
		Seg_Buf[0] = 13; // F
		Seg_Buf[1] = fac_show_mode + 1;
		switch (para_show_mode)
		{
		case 0:
			/* У׼ֵ */
			if (calibration_value <= -10)
			{
				Seg_Buf[5] = 11; //-
				Seg_Buf[6] = (-calibration_value) / 10 % 10;
				Seg_Buf[7] = (-calibration_value) % 10;
			}
			else if (calibration_value < 0)
			{
				Seg_Buf[5] = 10;
				Seg_Buf[6] = 11; //-
				Seg_Buf[7] = (-calibration_value) % 10;
			}
			else
			{
				Seg_Buf[5] = 10;
				Seg_Buf[6] = calibration_value / 10 % 10;
				Seg_Buf[7] = calibration_value % 10;
			}
			break;
		case 1:
			/* ���� */
			Seg_Buf[4] = (transmission_speed / 1000 % 10 == 0) ? 10 : transmission_speed / 1000 % 10;
			Seg_Buf[5] = ((transmission_speed / 100 % 10 == 0) && Seg_Buf[4] == 10) ? 10 : transmission_speed / 100 % 10;
			Seg_Buf[6] = transmission_speed / 10 % 10;
			Seg_Buf[7] = transmission_speed % 10;

			break;
		case 2:
			/* DAC��� */
			Seg_Buf[6] = dac_low_output_10x / 10 % 10;
			Seg_Buf[7] = dac_low_output_10x % 10;
			Seg_Point[6] = 1;
			break;
		}
		break;
	}
}

/* ������ʾ���� */
void Led_Proc()
{
	uchar i;
	switch (Seg_Show_mode)
	{
	case 0:
		/* ��� */
		if (distance_data > 255)
		{
			for (i = 0; i < 8; i++)
			{
				ucLed[i] = 1;
			}
		}
		else
		{
			for (i = 0; i < 8; i++)
			{
				ucLed[i] = distance_data & (1 << i);
			}
		}
		break;
	case 1:
		/* ���� */
		for (i = 0; i < 7; i++)
			ucLed[i] = 0;
		ucLed[7] = 1;
		break;
	case 2:
		/* ���� */
		for (i = 1; i < 8; i++)
			ucLed[i] = 0;
		ucLed[0] = led_show_flag;
		break;
	}
	Relay(((para_distance - 5 <= distance_data) &&
		   (distance_data <= para_distance + 5) &&
		   (temperature_data_10x / 10.0 <= para_temperature)));
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
	if (write_distance_data_flag)
	{
		if (++time_6s == 6000)
		{
			time_6s = 0;
			write_distance_data_flag = 0;
		}
	}
	// ��ʼ��λ������ʱ
	if (reset_flag)
	{
		if (++time_2s >= 2000)
		{
			time_2s = 0;
			reset_data();
		}
	}
	else
		time_2s = 0;
	// 100ms��˸һ��
	if (++time_100ms == 100)
	{
		time_100ms = 0;
		led_show_flag ^= 1;
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

	System_Init();
	Timer0Init();
	reset_data();
	// ������¶ȶ�ȡ�Ļ�
	rd_temperature();
	Delay750ms();
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}