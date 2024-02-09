/* ͷ�ļ������� */
#include <STC15F2K60S2.H> //��Ƭ���Ĵ���ר��ͷ�ļ�
#include <Init.h>		  //��ʼ���ײ�����ר��ͷ�ļ�
#include <Led.h>		  //Led�ײ�����ר��ͷ�ļ�
#include <Key.h>		  //�����ײ�����ר��ͷ�ļ�
#include <Seg.h>		  //����ܵײ�����ר��ͷ�ļ�
#include <stdio.h>		  //��׼��ײ�����ר��ͷ�ļ�
#include <iic.h>		  //��������ͷ�ļ�
#include <ds1302.h>		  //ds1302ͷ�ļ�
#include <onewire.h>	  //�¶ȴ�����ͷ�ļ�
/* ���������� */
unsigned char Key_Val, Key_Down, Key_Old, Key_Up;			 // ����ר�ñ���
unsigned char Key_Slow_Down;								 // ��������ר�ñ���
unsigned char Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
unsigned char Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
unsigned char Seg_Pos;										 // �����ɨ��ר�ñ���
unsigned int Seg_Slow_Down;									 // ����ܼ���ר�ñ���
unsigned char ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������

/*ʱ��ģ��*/
unsigned char time_100ms;					 // 100ms��ʱ
unsigned int time_1s;						 // 1s��ʱ
unsigned int time_2s;						 // 2s��ʱ
unsigned int time_3s;						 // 3s��ʱ
unsigned char ucRtc[3] = {0x15, 0x55, 0x55}; // ��ʼʱ��Ϊ11:11:11

/*�������ʾģ��*/
unsigned char Seg_show_mode = 0;	 // ����������ʾ��0ʱ�䣬1�¶Ȼ��ԣ�2������100��ʪ��
unsigned char old_Seg_show_mode = 0; // ��һ�εĽ�����ʾ
unsigned char re_Seg_show_mode = 0;	 // ���Խ�����ʾ��0�¶Ȼ��ԣ�1ʪ�Ȼ��ԣ�2ʱ�����

/*�¶ȴ���ģ��*/
idata float aver_temperature = 0;		 // ƽ���¶�
idata unsigned char max_temperature = 0; // ����¶ȣ�������Ϊ����ֻ��Ҫ��ʾ������
idata float old_tempture = 0;			 // ���¶�
bit up_tempture_flag = 0;				 // �¶��Ƿ�����

/*ʪ�ȴ���ģ��*/
idata float aver_humidity = 0;			   // ƽ��ʪ��
idata unsigned char max_humidity = 0;	   // ���ʪ�ȣ�������Ϊ����ֻ��Ҫ��ʾ������
idata unsigned int count_f = 0;			   // �������
idata unsigned int data_f = 0;			   // 1s��Ƶ��
bit humidity_flag = 1;					   // �Ƿ���Ч��¼
idata unsigned char demo_temperature = 30; // �¶Ȳο�
idata float old_humidity = 0;			   // ��ʪ��
bit up_humidity_flag = 0;				   // ʪ���Ƿ�����

/*�����͹���ģ��*/
unsigned char trigger_number = 0;		   // ��������
unsigned char old_light = 0;			   // �ɵĹ���ֵ
unsigned char trigger_time[3] = {0, 0, 0}; // ���һ�βɼ�ʱ��
bit light_down_flag = 0;				   // �����Ƿ�䰵

/* ���̴����� */
void Key_Proc()
{
	if (Key_Slow_Down)
		return;
	Key_Slow_Down = 1; // ���̼��ٳ���

	Key_Val = Key_Read();					  // ʵʱ��ȡ����ֵ
	Key_Down = Key_Val & (Key_Old ^ Key_Val); // ��׽�����½���
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // ��׽�����Ͻ���
	Key_Old = Key_Val;						  // ����ɨ�����
	// ���½����л��İ���
	if (Key_Down == 4)
	{
		Seg_show_mode++;
		if (Seg_show_mode > 2)
		{
			Seg_show_mode = 0; // ����ѭ��
		}
	}
	// ���»��Խ����л��İ��������Ҵ��ڻ��Խ���
	if (Key_Down == 5 && Seg_show_mode == 1)
	{
		re_Seg_show_mode++;
		if (re_Seg_show_mode > 2)
		{
			re_Seg_show_mode = 0; // ����ѭ��
		}
	}
	if (Key_Down == 8 && Seg_show_mode == 2)
	{
		if (++demo_temperature > 100)
		{
			demo_temperature = 0;
		}
	}
	if (Key_Down == 9 && Seg_show_mode == 2)
	{
		if (--demo_temperature < 0)
		{
			demo_temperature = 99;
		}
	}
	if (Key_Down == 9 && Seg_show_mode == 1 && re_Seg_show_mode == 2)
	{
		if (++time_2s == 200)
		{
			// ���ڳ�����2s�����ȫ��������
			trigger_number = 0;
			aver_temperature = 0;
			max_temperature = 0;
			old_tempture = 0;

			aver_humidity = 0;
			max_humidity = 0;
			old_humidity = 0;
			trigger_time[0] = 0;
			trigger_time[1] = 0;
			trigger_time[2] = 0;
		}
	}
	else
	{
		time_2s = 0;
	}
}

/* ��Ϣ������ */
void Seg_Proc()
{
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	if (Seg_show_mode == 0)
	{
		re_Seg_show_mode = 0;
		// ��ʾʱ��
		Read_Rtc(ucRtc);
		Seg_Buf[0] = ucRtc[0] / 16;
		Seg_Buf[1] = ucRtc[0] % 16;
		Seg_Buf[3] = ucRtc[1] / 16;
		Seg_Buf[4] = ucRtc[1] % 16;
		Seg_Buf[6] = ucRtc[2] / 16;
		Seg_Buf[7] = ucRtc[2] % 16;
		Seg_Buf[2] = Seg_Buf[5] = 16; //-
		Seg_Point[6] = 0;
	}
	else if (Seg_show_mode == 1)
	{
		// ������Խ��棬��Ĭ��Ϊ�¶�
		switch (re_Seg_show_mode)
		{
		case 0:				 // �¶Ȼ���
			Seg_Buf[0] = 11; // C
			if (trigger_number == 0)
			{
				Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 10; // ��
				Seg_Point[6] = 0;
			}
			else
			{
				unsigned int temp;
				temp = aver_temperature * 10;
				Seg_Buf[1] = 10; // ��
				Seg_Buf[2] = max_temperature / 10;
				Seg_Buf[3] = max_temperature % 10;
				Seg_Buf[4] = 16; //-
				Seg_Buf[5] = temp / 100;
				Seg_Buf[6] = (temp % 100) / 10;
				Seg_Buf[7] = temp % 10;
				Seg_Point[6] = 1;
			}
			break;
		case 1:				 // ʪ�Ȼ���
			Seg_Buf[0] = 12; // H
			if (trigger_number == 0)
			{
				Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 10; // ��
				Seg_Point[6] = 0;
			}
			else
			{
				unsigned int temp;
				temp = aver_humidity * 10;
				Seg_Buf[1] = 10; // ��
				Seg_Buf[2] = max_humidity / 10;
				Seg_Buf[3] = max_humidity % 10;
				Seg_Buf[4] = 16; //-
				Seg_Buf[5] = temp / 100;
				Seg_Buf[6] = (temp % 100) / 10;
				Seg_Buf[7] = temp % 10;
				Seg_Point[6] = 1;
			}
			break;
		case 2:				 // ʱ�����
			Seg_Buf[0] = 13; // F
			if (trigger_number == 0)
			{
				Seg_Buf[1] = Seg_Buf[2] = 0;
				Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 16; // ��
				Seg_Point[6] = 0;
			}
			else
			{
				Seg_Buf[1] = trigger_number / 10;
				Seg_Buf[2] = trigger_number % 10;
				Seg_Buf[3] = trigger_time[0] / 16;
				Seg_Buf[4] = trigger_time[0] % 16;
				Seg_Buf[5] = 16; //-
				Seg_Buf[6] = trigger_time[1] / 16;
				Seg_Buf[7] = trigger_time[1] % 16;
				Seg_Point[6] = 0;
			}
			break;
		}
	}
	else if (Seg_show_mode == 2)
	{
		// ��ʾ����
		Seg_Buf[0] = 14; // P

		Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10; // ��
		Seg_Buf[6] = demo_temperature / 10;
		Seg_Buf[7] = demo_temperature % 10;
		Seg_Point[6] = 0;
		re_Seg_show_mode = 0;
	}
	else if (Seg_show_mode == 100)
	{
		unsigned char temp_temperature;
		unsigned char temp_humidity;
		// ��ʾ��ʪ�Ƚ���
		Seg_Buf[0] = 15; // E
		temp_temperature = old_tempture;
		temp_humidity = old_humidity;
		Seg_Buf[1] = Seg_Buf[2] = 10; // ��
		Seg_Buf[3] = temp_temperature / 10;
		Seg_Buf[4] = temp_temperature % 10;
		Seg_Buf[5] = 16; //-
		Seg_Buf[6] = temp_humidity / 10;
		Seg_Buf[7] = temp_humidity % 10;
		Seg_Point[6] = 0;
	}
}

/* ������ʾ���� */
void Led_Proc()
{
	if (Seg_show_mode == 0)
	{
		ucLed[0] = 1;
	}
	else
	{
		ucLed[0] = 0;
	}
	if (Seg_show_mode == 1)
	{
		ucLed[1] = 1;
	}
	else
	{
		ucLed[1] = 0;
	}
	if (Seg_show_mode == 100)
	{
		ucLed[2] = 1;
	}
	else
	{
		ucLed[2] = 0;
	}
	// �ɼ�ʪ����Ч
	if (!humidity_flag)
	{
		ucLed[4] = 1;
	}
	else
	{
		ucLed[4] = 0;
	}
	if (up_humidity_flag && up_tempture_flag && (trigger_number > 2))
	{
		ucLed[5] = 1;
	}
	else
	{
		ucLed[5] = 0;
	}
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
/*������1�ն˳�ʼ������*/
void Timer1Init(void) // 1΢��@12.000MHz
{
	AUXR &= 0xBF; // ��ʱ��ʱ��12Tģʽ
	TMOD &= 0x0F; // ���ö�ʱ��ģʽ
	TMOD |= 0x20; // ���ö�ʱ��ģʽ
	TL1 = 0xFF;	  // ���ö�ʱ��ֵ
	TH1 = 0xFF;	  // ���ö�ʱ����ֵ
	TF1 = 0;	  // ���TF1��־
	TR1 = 1;	  // ��ʱ��1��ʼ��ʱ
	ET1 = 1;	  // ��ʱ��1�жϴ�
	EA = 1;		  // ���жϴ�
}
/* ������1�жϷ����� */
void Timer1Server() interrupt 3
{
	count_f++;
}
/* ��ʱ��0�жϷ����� */
void Timer0Server() interrupt 1
{
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // ���̼���ר��
	if (++Seg_Slow_Down == 200)
		Seg_Slow_Down = 0; // ����ܼ���ר��
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // �������ʾר��
	// 1s��ʱ�����ǽ�����ļ�������ȡ��
	if (++time_1s == 1000)
	{
		data_f = count_f;
		count_f = 0;
	}
	// �����Ǵ����˹ⰵ��ʱ��
	if (light_down_flag)
	{
		// �ɼ�3s
		if (++time_3s == 3000)
		{
			light_down_flag = 0;
			Seg_show_mode = old_Seg_show_mode; // �ص�ԭ������ʾ
		}
		// 0.1sȡ��
		if (++time_100ms == 100)
		{
			ucLed[3] = ~ucLed[3];
		}
	}

	Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}
void get_temperature()
{
	float temp_temperature;
	temp_temperature = rd_temperature();
	if (trigger_number > 2 && temp_temperature > old_tempture)
	{
		up_tempture_flag = 1;
	}
	else
	{
		up_tempture_flag = 0;
	}
	if (max_temperature < temp_temperature)
	{
		max_temperature = temp_temperature;
	}
	aver_temperature = (aver_temperature * (trigger_number - 1) + temp_temperature) / trigger_number;
	old_tempture = temp_temperature;
}
void get_humidity()
{
	// ���ʱ����Ϊ����Ч��¼
	if (data_f < 200 || data_f > 2000)
	{
		humidity_flag = 0;
	}
	else
	{
		float temp_humidity;
		// �ɼ���Ч
		humidity_flag = 1;
		temp_humidity = (data_f - 200) * 2 / 45 + 10;
		if (trigger_number > 2 && temp_humidity > old_humidity)
		{
			// ʪ������
			up_humidity_flag = 1;
		}
		else
		{
			up_humidity_flag = 0;
		}
		if (max_humidity < temp_humidity)
		{
			max_humidity = temp_humidity;
		}
		aver_humidity = (aver_humidity * (trigger_number - 1) + temp_humidity) / trigger_number;
		old_humidity = temp_humidity;
	}
}
void get_light()
{
	unsigned char temp_light;
	temp_light = Ad_Read(0x01);
	// �������п��Ǳ䰵����ֵ
	// �������䰵������֮ǰû�н��б仯��ʱ��
	if (old_light > 120 && temp_light < 80 && !light_down_flag)
	{
		light_down_flag = 1;
		trigger_number++;
		get_temperature();
		get_humidity();
		Read_Rtc(trigger_time);			   // ��¼һ�²ɼ���ʱ��
		old_Seg_show_mode = Seg_show_mode; // ��¼һ����תǰ��״̬
		Seg_show_mode = 100;			   // ��ת����ʪ����ʾ����
	}
	old_light = temp_light;
}
/* Main */
void main()
{
	System_Init();
	Timer0Init();
	Timer1Init();
	Set_Rtc(ucRtc);
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
		get_light();
	}
}