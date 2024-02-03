#include "main.h"
#define LIGHT_NUM 50
#define DARK_NUM 10
/* ���������� */
uchar Key_Slow_Down;								 // ��������ר�ñ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
uchar Seg_Pos;										 // �����ɨ��ר�ñ���
uint Seg_Slow_Down;									 // ����ܼ���ר�ñ���
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������
uchar ucRtc[3] = {0x11, 0x11, 0x11};
/*����*/
bit Seg_show_mode;		// 0���� 1����
uchar data_show_mode;	// 0ʱ�� 1���� 2��¼
bit celi_show_mode;		// 0ʱ����� 1�������
bit burst_mode;			// 0����ģʽ 1��ʱģʽ
uchar data_show_memory; // 0���ֵ 1��Сֵ 2ƽ��ֵ
/*����*/
uchar collection_time = 2;
uchar collection_time_arr[5] = {2, 3, 5, 7, 9};
uchar collection_time_index;
uchar dis_demo = 10;
uint dis_value;
uint dis_max;
uint dis_min;
uint collection_count;
uchar wring_count;
bit wring_flag;
float dis_aver;

uchar light_value, light_value_old;
void init_Seg()
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		Seg_Buf[i] = 10;
		Seg_Point[i] = 0;
	}
}
void hide_high_Seg(uchar start_num, uchar end_num)
{
	uchar i;
	for (i = start_num; i < end_num; i++)
	{
		if (Seg_Buf[i] != 0)
			break;
		else
			Seg_Buf[i] = 10;
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
		Seg_show_mode ^= 1;
		celi_show_mode = data_show_mode = data_show_memory = 0;
		init_Seg();
	}
	if (Key_Down == 5)
	{
		// ��������
		if (Seg_show_mode)
		{
			celi_show_mode ^= 1;
			init_Seg();
		}
		// ���ݽ���
		else
		{
			data_show_mode = (++data_show_mode) % 3;
			init_Seg();
		}
	}
	if (Key_Down == 8 && Seg_show_mode == 0)
	{
		if (data_show_mode == 1)
			burst_mode ^= 1;
		else if (data_show_mode == 2)
		{
			data_show_memory = (++data_show_memory) % 3;
			init_Seg();
		}
	}
	if (Key_Down == 9 && Seg_show_mode == 1)
	{
		if (celi_show_mode)
			dis_demo = (++dis_demo > 80) ? 10 : dis_demo;
		else
		{
			collection_time_index = (++collection_time_index) % 5;
			collection_time = collection_time_arr[collection_time_index];
		}
	}
}

/* ��Ϣ������ */
void Seg_Proc()
{
	uchar temp_dis;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	Read_Rtc(ucRtc);
	light_value = Ad_Read(0x01);
	// ��ʱģʽ
	if (burst_mode)
	{
		if ((ucRtc[2] / 16 * 10 + ucRtc[2] % 16) % collection_time == 0)
		{
			dis_value = Ut_Wave_Data();
			collection_count = (++collection_count) % 65535;
			dis_aver = (dis_aver * (collection_count - 1) + dis_value) / collection_count;
			if (dis_value > dis_demo - 5 || dis_value < dis_demo + 5)
			{
				if (++wring_count > 3)
				{
					wring_count = 4;
				}
			}
			else
				wring_count = 0;
		}
	}
	// ����ģʽ
	else
	{
		// ��->��
		if (light_value_old > LIGHT_NUM && light_value < DARK_NUM)
		{
			dis_value = Ut_Wave_Data();
			collection_count = (++collection_count) % 65535;
			dis_aver = (dis_aver * (collection_count - 1) + dis_value) / collection_count;
			if (dis_value > dis_demo - 5 || dis_value < dis_demo + 5)
				wring_count = (++wring_count) % 4;
			else
				wring_count = 0;
		}
	}
	if (dis_max < dis_value)
		dis_max = dis_value;
	if (dis_min > dis_value || dis_min == 0)
		dis_min = dis_value;
	if (dis_value > 80)
		Da_Write(255);
	else if (dis_value < 10)
		Da_Write(51);
	else
		Da_Write(dis_value / 70.0 * 4 * 51);
	light_value_old = light_value;
	// ��������
	if (Seg_show_mode)
	{
		Seg_Buf[0] = 18; // P
		Seg_Buf[1] = (uchar)celi_show_mode + 1;
		if (celi_show_mode)
		{
			Seg_Buf[6] = collection_time / 10 % 10;
			Seg_Buf[7] = collection_time % 10;
		}
		else
		{
			Seg_Buf[6] = dis_demo / 10 % 10;
			Seg_Buf[7] = dis_demo % 10;
		}
	}
	// ���ݽ���
	else
	{
		switch (data_show_mode)
		{
		case 0:
			/* ʱ����ʾ */
			Seg_Buf[0] = ucRtc[0] / 16;
			Seg_Buf[1] = ucRtc[0] % 16;
			Seg_Buf[2] = 16;
			Seg_Buf[3] = ucRtc[1] / 16;
			Seg_Buf[4] = ucRtc[1] % 16;
			Seg_Buf[5] = 16;
			Seg_Buf[6] = ucRtc[2] / 16;
			Seg_Buf[7] = ucRtc[2] % 16;
			break;
		case 1:
			/*������ʾ*/
			Seg_Buf[0] = 11; // L
			Seg_Buf[1] = (burst_mode) ? 12 : 13;
			Seg_Buf[5] = dis_value / 100 % 10;
			Seg_Buf[6] = dis_value / 10 % 10;
			Seg_Buf[7] = dis_value % 10;
			hide_high_Seg(5, 7);
			break;
		case 2:
			/*���ݼ�¼*/
			Seg_Buf[0] = 14; // H
			Seg_Buf[1] = data_show_memory + 15;
			switch (data_show_memory)
			{
			case 0:
				/* ���ֵ */
				Seg_Buf[4] = dis_max / 1000 % 10;
				Seg_Buf[5] = dis_max / 100 % 10;
				Seg_Buf[6] = dis_max / 10 % 10;
				Seg_Buf[7] = dis_max % 10;
				hide_high_Seg(4, 7);
				break;
			case 1:
				/*ƽ��ֵ*/
				temp_dis = dis_aver * 10;
				Seg_Buf[4] = temp_dis / 1000 % 10;
				Seg_Buf[5] = temp_dis / 100 % 10;
				Seg_Buf[6] = temp_dis / 10 % 10;
				Seg_Buf[7] = temp_dis % 10;
				hide_high_Seg(4, 6);
				Seg_Point[6] = 1;
				break;
			case 2:
				/*��Сֵ*/
				Seg_Buf[4] = dis_min / 1000 % 10;
				Seg_Buf[5] = dis_min / 100 % 10;
				Seg_Buf[6] = dis_min / 10 % 10;
				Seg_Buf[7] = dis_min % 10;
				hide_high_Seg(4, 7);
				break;
			}
			break;
		}
	}
}

/* ������ʾ���� */
void Led_Proc()
{
	ucLed[0] = (Seg_show_mode == 0 && data_show_mode == 0);
	ucLed[1] = (Seg_show_mode == 0 && data_show_mode == 1);
	ucLed[2] = (Seg_show_mode == 0 && data_show_mode == 2);
	ucLed[3] = !burst_mode;
	if (burst_mode)
	{
		Beep(wring_count >= 3);
		ucLed[4] = (wring_count >= 3);
	}
	else
	{
		Beep(0);
		ucLed[4] = 0;
	}
	ucLed[5] = (light_value > LIGHT_NUM);
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
	Set_Rtc(ucRtc);
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}