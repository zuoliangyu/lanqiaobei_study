#include "main.h"
/* ���������� */
uchar Key_Slow_Down;								 // ��������ר�ñ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
uchar Seg_Pos;										 // �����ɨ��ר�ñ���
uint Seg_Slow_Down;									 // ����ܼ���ר�ñ���
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������

// ʱ��
uchar ucRtc[3] = {0x12, 0x12, 0x12}; // ��ʼʱ��
uchar old_Rtc[3];					 // ��һ�μ�¼��ʱ��

// ��ʾ
uchar Seg_show_mode; // 0ʱ�䣬1���ԣ�3����
uchar re_show_mode;	 // 0�¶Ȼ��ԣ�1ʪ�Ȼ��ԣ�2ʱ�����
bit reset_flag;		 // ��λ��־λ
// ��ʱ���Ƶ
uchar time_100ms;
uint time_1s;
uint time_2s;
uint time_3s;
uint freq;

// �ɼ�����
bit collect_flag; // �ɼ�������־λ
uchar old_light;  // ��һ�ε��¶ȣ������Աȣ��ж��Ƿ��ǹ�->��
// ������ʪ��
float aver_temperature;
uchar max_temperature, old_temperature;
float aver_humidity;
uchar max_humidity, old_humidity;
uchar measure_count;	// ��������
uchar temperature_demo; // �¶Ȳ���
uchar humidity_error;
bit wring_flag; // �¶ȴ����¶Ȳ���
bit led_flag;	// ��˸
bit up_flag;	// ��α���һ�βɼ������ݸ�

void init_Seg_LED()
{
	uchar i = 0;
	for (i = 0; i < 8; i++)
	{
		ucLed[i] = 0;
		Seg_Buf[i] = 10;
		Seg_Point[i] = 0;
	}
}
void reset_system()
{
	Seg_show_mode = 0;
	re_show_mode = 0;
	temperature_demo = 30;
	measure_count = 0;
}
uchar rd_humidity()
{
	if (freq < 10 || freq > 2000)
		return 0;
	else
		return ((freq - 200) * 2 / 45 + 10);
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
		Seg_show_mode = (++Seg_show_mode) % 3;
		re_show_mode = 0; // ��֤ÿ�ζ����л����¶Ȼ���
		init_Seg_LED();
	}
	if (Seg_show_mode == 1 && Key_Down == 5)
	{
		re_show_mode = (++re_show_mode) % 3;
		init_Seg_LED();
	}
	if (Seg_show_mode == 2 && Key_Down == 8)
		temperature_demo = (++temperature_demo >= 99) ? 99 : temperature_demo;
	if (Seg_show_mode == 2 && Key_Down == 9)
		temperature_demo = (--temperature_demo == 0) ? 0 : temperature_demo;
	if (Seg_show_mode == 1 && re_show_mode == 2)
	{
		if (Key_Down == 9)
		{
			reset_flag = 1;
		}
		if (Key_Up == 9 && time_2s >= 2000)
		{
			reset_flag = 0;
			reset_system();
		}
	}
}

/* ��Ϣ������ */
void Seg_Proc()
{
	uchar temp_temperature, temp_humidity, light;

	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	light = Ad_Read(0x41);
	temp_temperature = rd_temperature();
	temp_humidity = rd_humidity();
	// ��һ���ǹ⣬��һ���ǰ������һ�û�д��ڲɼ���ʱ��
	if (light < 10 && old_light > 40 && (time_3s == 0))
	{
		init_Seg_LED();
		collect_flag = 1;
		measure_count = (++measure_count) % 100;
		// ��ʪ�Ƚ���
		Read_Rtc(old_Rtc);
	}
	old_light = light;
	if (collect_flag)
	{
		// ��ʪ�Ƚ���
		if (measure_count >= 2 && (old_temperature < temp_temperature) && (old_humidity < temp_humidity))
			up_flag = 1;
		else
			up_flag = 0;
		old_temperature = temp_temperature;
		old_humidity = temp_humidity;
		if (temp_temperature > temperature_demo)
			wring_flag = 1;
		else
			wring_flag = 0;
		if (temp_temperature > max_temperature)
			max_temperature = temp_temperature;
		if (temp_humidity > max_humidity)
			max_humidity = temp_humidity;
		if (temp_humidity == 0)
		{
			++humidity_error; // ��Ч����+1
			ucLed[4] = 1;	  // ����LED5
		}
		else
		{
			ucLed[4] = 0; // Ϩ��LED5
		}
		aver_temperature = (aver_temperature * (measure_count - 1) + temp_temperature) / measure_count;
		aver_humidity = (aver_humidity * (measure_count - humidity_error - 1) + temp_humidity) / (measure_count - humidity_error);
		Seg_Buf[0] = 16; // E
		Seg_Buf[1] = measure_count / 10;
		Seg_Buf[2] = measure_count % 10;
		Seg_Buf[3] = temp_temperature / 10;
		Seg_Buf[4] = temp_temperature % 10;
		Seg_Buf[5] = 11; //-
		// ʪ�ȷǷ���ʾAA
		Seg_Buf[6] = (temp_humidity == 0) ? 17 : temp_humidity / 10;
		Seg_Buf[7] = (temp_humidity == 0) ? 17 : temp_humidity % 10;
	}
	else
	{
		switch (Seg_show_mode)
		{
		case 0:
			// ʱ�����
			Read_Rtc(ucRtc);
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
			// ���Խ���
			switch (re_show_mode)
			{
			case 0:
				// �¶Ȼ���
				Seg_Buf[0] = 12; // C
				Seg_Buf[2] = (measure_count > 0) ? (max_temperature / 10) : 10;
				Seg_Buf[3] = (measure_count > 0) ? (max_temperature % 10) : 10;
				Seg_Buf[4] = 11; //-
				Seg_Buf[5] = (measure_count > 0) ? ((uchar)(aver_temperature * 10) / 100) : 10;
				Seg_Buf[6] = (measure_count > 0) ? ((uchar)(aver_temperature * 10) % 100 / 10) : 10;
				Seg_Buf[7] = (measure_count > 0) ? ((uchar)(aver_temperature * 10) % 10) : 10;
				Seg_Point[6] = 1;
				break;
			case 1:
				// ʪ�Ȼ���
				Seg_Buf[0] = 13; // H
				Seg_Buf[2] = (measure_count > 0) ? (max_humidity / 10) : 10;
				Seg_Buf[3] = (measure_count > 0) ? (max_humidity % 10) : 10;
				Seg_Buf[4] = 11; //-
				Seg_Buf[5] = (measure_count > 0) ? ((uchar)(aver_humidity * 10) / 100) : 10;
				Seg_Buf[6] = (measure_count > 0) ? ((uchar)(aver_humidity * 10) % 100 / 10) : 10;
				Seg_Buf[7] = (measure_count > 0) ? ((uchar)(aver_humidity * 10) % 10) : 10;
				Seg_Point[6] = 1;
				break;
			case 2:
				// ʱ�����
				Seg_Buf[0] = 14; // F
				Seg_Buf[1] = measure_count / 10;
				Seg_Buf[2] = measure_count % 10;
				Seg_Buf[3] = (measure_count > 0) ? (old_Rtc[0] / 16) : 10;
				Seg_Buf[4] = (measure_count > 0) ? (old_Rtc[0] % 16) : 10;
				Seg_Buf[5] = (measure_count > 0) ? 11 : 10; //-
				Seg_Buf[6] = (measure_count > 0) ? (old_Rtc[1] / 16) : 10;
				Seg_Buf[7] = (measure_count > 0) ? (old_Rtc[1] % 16) : 10;
				break;
			}
			break;
		case 2:
			// ��������
			Seg_Buf[0] = 15; // P
			Seg_Buf[6] = temperature_demo / 10;
			Seg_Buf[7] = temperature_demo % 10;
			break;
		}
	}
}

/* ������ʾ���� */
void Led_Proc()
{
	switch (Seg_show_mode)
	{
	case 0:
		// ʱ�����
		ucLed[0] = 1;
		break;
	case 1:
		ucLed[1] = 1;
		break;
	}
	if (collect_flag)
		ucLed[2] = 1;
	ucLed[3] = led_flag;
	ucLed[5] = up_flag;
}

void Timer0_Init(void) // 0΢��@12.000MHz
{
	AUXR &= 0x7F; // ��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0; // ���ö�ʱ��ģʽ
	TMOD |= 0x05; // �ֶ����ó�ֵ
	TL0 = 0x00;	  // ���ö�ʱ��ֵ
	TH0 = 0x00;	  // ���ö�ʱ��ֵ
	TF0 = 0;	  // ���TF0��־
	TR0 = 1;	  // ��ʱ��0��ʼ��ʱ
	ET0 = 1;	  // ��ʱ��0�жϴ�
	EA = 1;		  // ���жϴ�
}

/* ��ʱ��1�жϳ�ʼ������ */
void Timer1_Init(void) // 1����@12.000MHz
{
	AUXR &= 0xBF; // ��ʱ��ʱ��12Tģʽ
	TMOD &= 0x0F; // ���ö�ʱ��ģʽ
	TL1 = 0x18;	  // ���ö�ʱ��ʼֵ
	TH1 = 0xFC;	  // ���ö�ʱ��ʼֵ
	TF1 = 0;	  // ���TF1��־
	TR1 = 1;	  // ��ʱ��1��ʼ��ʱ
	ET1 = 1;	  // ��ʱ��1�жϴ�
	EA = 1;		  // ���жϴ�
}
/* ��ʱ��1�жϷ����� */
void Timer1Server() interrupt 3
{
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // ���̼���ר��
	if (++Seg_Slow_Down == 500)
		Seg_Slow_Down = 0; // ����ܼ���ר��
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // �������ʾר��
	if (++time_1s == 1000)
	{
		time_1s = 0;
		freq = TH0 << 8 | TL0; // ȡ��Ƶ��
		TH0 = 0;			   // ����
		TL0 = 0;			   // ����
	}
	if (collect_flag)
	{
		if (++time_3s == 3000)
		{
			collect_flag = 0;
			time_3s = 0;
		}
	}
	else
	{
		time_3s = 0;
	}
	if (reset_flag)
	{
		if (++time_2s >= 2000)
		{
			time_2s = 2001;
		}
	}
	if (wring_flag)
	{
		if (++time_100ms == 100)
		{
			time_100ms = 0;
			led_flag ^= 1;
		}
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
	reset_system();
	System_Init();
	Timer0_Init();
	Timer1_Init();
	Set_Rtc(ucRtc); // д���ʼʱ��
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}