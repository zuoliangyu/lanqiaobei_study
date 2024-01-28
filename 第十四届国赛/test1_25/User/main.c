#include "main.h"
/* ���������� */
uchar Key_Slow_Down;								 // ��������ר�ñ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
uchar Seg_Pos;										 // �����ɨ��ר�ñ���
uint Seg_Slow_Down;									 // ����ܼ���ר�ñ���
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������

/*ʱ�䷽��*/
uint time_6s;		 // 6s��ʱ
uint time_2s;		 // 2s��ʱ
uchar time_100ms;	 // 100ms��ʱ
bit reset_flag;		 // ��λ��־λ
bit write_data_flag; // д�����ݱ�־λ
bit led_light_flag;	 // ��˸

/*��ʾģʽ���л�*/
uchar Seg_show_mode;   // 0-���;1-����;2-����
bit test1_show_mode;   // 0-cm;1-m
bit test2_show_mode;   // 0-�������;1-�¶Ȳ���
uchar test3_show_mode; // 0-У׼ֵ 1-�����ٶ� 2-DAC����

/*������*/
uint dis_value; // ����ֵcm

/*�¶�ģ��*/
float temperature_value; // �¶�ֵ
/*��������*/
uchar dis_demo;
uchar temperature_demo;

/*����ģʽ*/
char cali_value;	 // У׼ֵ
uint speed_value;	 // �����ٶ�
float dac_low_value; // DAC����

/*���ݼ�¼�����*/
float memory_data;
uchar dac_out;

void init_sys_value()
{
	Seg_show_mode = 0;
	test1_show_mode = 0;
	test2_show_mode = 0;
	test3_show_mode = 0;
	dis_demo = 40;
	temperature_demo = 30;
	cali_value = 0;
	speed_value = 340;
	dac_low_value = 1.0;
}
void init_Seg_LED() // ������ܺ�LED��գ����������޸�
{
	uchar i; // ѭ������
	for (i = 0; i < 8; i++)
	{
		Seg_Buf[i] = 10;  // �������ʾ���ݳ�ʼ��
		Seg_Point[i] = 0; // �����С�����ʼ��
		ucLed[i] = 0;	  // LED��ʾ���ݳ�ʼ��
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

	// ������û�н��м�¼
	if (!write_data_flag)
	{
		switch (Key_Down)
		{
		case 4:
			init_Seg_LED();
			Seg_show_mode = (++Seg_show_mode < 3) ? Seg_show_mode : 0;
			test1_show_mode = test2_show_mode = test3_show_mode = 0;
			break;
		case 5:
			init_Seg_LED();
			switch (Seg_show_mode)
			{
			case 0:
				test1_show_mode = ~test1_show_mode;
				break;
			case 1:
				test2_show_mode = ~test2_show_mode;
				break;
			case 2:
				test3_show_mode = (++test3_show_mode < 3) ? test3_show_mode : 0;
				break;
			}
			break;
		case 8:
			switch (Seg_show_mode)
			{
			case 0:
				write_data_flag = 1;
				break;
			case 1:
				if (test2_show_mode)
					temperature_demo = (++temperature_demo < 80) ? temperature_demo : 80;
				else
					dis_demo = (dis_demo + 10 < 90) ? dis_demo + 10 : 90;
				break;
			case 2:
				switch (test3_show_mode)
				{
				case 0:
					cali_value = (cali_value + 5 < 90) ? cali_value + 5 : 90;
					break;
				case 1:
					speed_value = (speed_value + 10 < 9990) ? speed_value + 10 : 9990;
					break;
				case 2:
					dac_low_value = (dac_low_value + 0.1 < 2.0) ? dac_low_value + 0.1 : 2.0;
					break;
				}
				break;
			}
			break;
		case 9:
			switch (Seg_show_mode)
			{
			case 0:
				// ��ʼ�������
				if (memory_data > 90)
				{
					dac_out = 5;
				}
				else if (memory_data == 0)
				{
					dac_out = 0;
				}
				else
				{
					dac_out = (5 - dac_low_value) / 80 * (memory_data - 10) + dac_low_value;
				}
				Da_Write(dac_out);
				break;
			case 1:
				if (test2_show_mode)
					temperature_demo = (--temperature_demo > 0) ? temperature_demo : 0;
				else
					dis_demo = (dis_demo - 10 > 10) ? dis_demo - 10 : 10;
				break;
			case 2:
				switch (test3_show_mode)
				{
				case 0:
					cali_value = (cali_value - 5 > -90) ? cali_value - 5 : -90;
					break;
				case 1:
					speed_value = (speed_value - 10 > 10) ? speed_value - 10 : 10;
					break;
				case 2:
					dac_low_value = (dac_low_value - 0.1 > 0.1) ? dac_low_value - 0.1 : 0.1;
					break;
				}
				break;
			}
			break;
		}
		// ���ͬʱ����89
		if (Key_Old == 89)
		{
			reset_flag = 1;
		}
		else
		{
			reset_flag = 0;
		}
	}
}

/* ��Ϣ������ */
void Seg_Proc()
{
	uint int_temperature_value; // �¶�ֵ���ͱ���
	uchar i;
	uchar temp_DAC_low;
	int temp_cali;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	switch (Seg_show_mode)
	{
	case 0:
		/* ������ */
		temperature_value = rd_temperature();
		int_temperature_value = temperature_value * 10;
		Seg_Buf[0] = int_temperature_value / 100;
		Seg_Buf[1] = int_temperature_value % 100 / 10;
		Seg_Buf[2] = int_temperature_value % 10;
		Seg_Point[1] = 1; // С����
		Seg_Buf[3] = 13;  //-
		dis_value = (uint)Ut_Wave_Data(speed_value, cali_value);
		// ��ʾ��λΪm
		if (test1_show_mode)
		{
			Seg_Buf[4] = (dis_value / 1000) ? dis_value / 1000 : 10;
			Seg_Buf[5] = dis_value / 100 % 10;
			Seg_Buf[6] = dis_value / 10 % 10;
			Seg_Buf[7] = dis_value % 10;
			Seg_Point[5] = 1;
		}
		// ��ʾ��λΪcm
		else
		{
			Seg_Buf[4] = dis_value / 1000;
			Seg_Buf[5] = dis_value / 100 % 10;
			Seg_Buf[6] = dis_value / 10 % 10;
			Seg_Buf[7] = dis_value % 10;
			i = 4;
			while (Seg_Buf[i] == 0)
			{
				if (i == 7)
					break;
				Seg_Buf[i] = 10;
				i++;
			}
		}
		break;
	case 1:
		/*��������*/
		Seg_Buf[0] = 11; // P
		Seg_Buf[1] = (uchar)test2_show_mode + 1;
		Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10; // ��
		Seg_Buf[6] = test2_show_mode ? temperature_demo / 10 : dis_demo / 10;
		Seg_Buf[7] = test2_show_mode ? temperature_demo % 10 : dis_demo % 10;
		break;
	case 2:
		/*��������*/
		Seg_Buf[0] = 12; // F
		Seg_Buf[1] = test3_show_mode + 1;
		Seg_Buf[2] = Seg_Buf[3] = 10; // ��
		switch (test3_show_mode)
		{
		case 0:
			Seg_Buf[4] = 10;
			/* ��ʾУ׼ֵ */
			temp_cali = abs((int)cali_value);
			Seg_Buf[5] = temp_cali / 100;
			Seg_Buf[6] = temp_cali / 10 % 10;
			Seg_Buf[7] = temp_cali % 10;
			i = 5;
			while (Seg_Buf[i] == 0)
			{
				if (i == 7)
					break;
				Seg_Buf[i] = 10;
				i++;
			}
			if (cali_value < 0)
			{
				Seg_Buf[i - 1] = 13; //-
			}
			break;
		case 1:
			/* ��ʾ�����ٶ� */
			Seg_Buf[4] = speed_value / 1000;
			Seg_Buf[5] = speed_value / 100 % 10;
			Seg_Buf[6] = speed_value / 10 % 10;
			Seg_Buf[7] = speed_value % 10;
			i = 4;
			while (Seg_Buf[i] == 0)
			{
				if (i == 7)
					break;
				Seg_Buf[i] = 10;
				i++;
			}
			break;
		case 2:

			temp_DAC_low = dac_low_value * 10;
			/* ��ʾDAC���� */
			Seg_Buf[4] = Seg_Buf[5] = 10;
			Seg_Buf[6] = temp_DAC_low / 10;
			Seg_Buf[7] = temp_DAC_low % 10;
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
	switch (Seg_show_mode)
	{
	case 0:
		for (i = 0; i < 8; i++)
		{
			ucLed[i] = dis_value & (0x01 << i);
		}
		if (dis_value >= 255)
		{
			ucLed[0] = ucLed[1] = ucLed[2] = ucLed[3] = ucLed[4] = ucLed[5] = ucLed[6] = ucLed[7] = 1;
		}
		break;
	case 1:
		ucLed[7] = 1;
		break;
	case 2:
		ucLed[0] = led_light_flag;
		break;
	}
	if ((dis_demo + 5 >= dis_value) && (temperature_demo >= temperature_value))
	{
		Relay(1);
	}
	else
	{
		Relay(0);
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

/* ��ʱ��0�жϷ����� */
void Timer0Server() interrupt 1
{
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // ���̼���ר��
	if (++Seg_Slow_Down == 500)
		Seg_Slow_Down = 0; // ����ܼ���ר��
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // �������ʾר��
	if (reset_flag)
	{
		if (++time_2s >= 2000)
		{
			time_2s = 0;
			init_sys_value();
		}
	}
	if (write_data_flag)
	{
		memory_data = dis_value;
		if (++time_6s >= 6000)
		{
			time_6s = write_data_flag = 0;
		}
	}
	if (++time_100ms >= 100)
	{
		led_light_flag = ~led_light_flag;
		time_100ms = 0;
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
	init_sys_value(); // ��ʼ����������
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