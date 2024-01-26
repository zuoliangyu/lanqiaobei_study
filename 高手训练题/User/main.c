#include "main.h"
/* ���������� */
uchar Key_Slow_Down;								 // ��������ר�ñ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С�������ݴ������
uchar Seg_Pos;										 // �����ɨ��ר�ñ���
uint Seg_Slow_Down;									 // ����ܼ���ר�ñ���
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led��ʾ���ݴ������
uchar Uart_Slow_Down;								 // ���ڼ���ר�ñ���
uchar Uart_Recv[10];								 // ���ڽ������ݴ������� Ĭ��10���ֽ� ���������ݽϳ� �ɸ�������ֽ���
uchar Uart_Recv_Index;								 // ���ڽ�������ָ��
uchar Uart_Send[10];								 // ���ڽ������ݴ������� Ĭ��10���ֽ� ���������ݽϳ� �ɸ�������ֽ���

/* ���� */
uchar Seg_show_mode; // 0 ��ʼ���� 1ʱ�ӽ��� 2��Ϣ��ʾ���� 3 ʱ�����ý��� 4�������ý���

/*����*/
idata uchar Password_set[8] = {1, 2, 3, 4, 5, 6, 7, 8};			  // ��ʼ��Ĭ������
idata uchar Password_input[8] = {11, 11, 11, 11, 11, 11, 11, 11}; // ���룬���Ϊû�������״̬
uchar Password_input_index;										  // ���������ָ��
bit wring_flag;													  // ��������־
bit ring_flag;													  // ������

/* ʱ�� */
uchar ucRtc[3] = {0x23, 0x59, 0x55};
uchar set_ucRtc[3]; // ����ʱ��
uchar old_ucRtc[3]; // ������һ�ε�ʱ��
uchar alRtc[3] = {0, 0, 0};
uchar set_alRtc[3];	   // ��������
uchar old_alRtc[3];	   // ������һ�ε�����
uchar set_time_two[2]; // ��λ�ݴ�
uchar time_index;
uchar alarm_index;
uint time_1s;
uint time_500ms;
uint time_700ms;
bit skip_flag;				// �����ı�־
bit time_interval_flag;		// ʱ����ʾ�м�������˸
bit time_set_interval_flag; // ���ý���ʱ���м�����˸
bit massage_show_flag;		// 0��ʾʱ�䣬1��ʾ��Ϣ
bit massage_show_time_flag; // ��������ʱ��->��Ϣ��ת��ʱ
bit alarm_open;				// ���ӿ���

// LED��Seg��ʼ��
void Init_Seg_Led()
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		Seg_Buf[i] = 0;
		Seg_Point[i] = 0;
		ucLed[i] = 0;
	}
}
/* ���̴����� */
void Key_Proc()
{
	static uchar Key_Val, Key_Down, Key_Old, Key_Up; // ����ר�ñ���
	uchar i, j;
	if (Key_Slow_Down)
		return;
	Key_Slow_Down = 1; // ���̼��ٳ���

	Key_Val = Key_Read();					  // ʵʱ��ȡ����ֵ
	Key_Down = Key_Val & (Key_Old ^ Key_Val); // ��׽�����½���
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // ��׽�����Ͻ���
	Key_Old = Key_Val;						  // ����ɨ�����
	switch (Seg_show_mode)
	{
	case 0: // ��¼����
		if (Password_input_index < 8 && Key_Down)
		{
			// 0 1 2 3 4 5 6 7 8 9
			uchar key_mapping[] = {4, 8, 12, 16, 9, 13, 17, 10, 14, 18};
			for (i = 0; i < 10; i++)
			{
				if (Key_Down == key_mapping[i])
				{
					break;
				}
			}
			// ���µĲ������һ������ô���������İ��������ﲻ������
			if (i == 10 && Key_Down != 18)
			{
			}
			else
			{
				Password_input[Password_input_index] = i;
				Password_input_index++;
			}
		}
		// ����ȷ��
		if (Key_Down == 7)
		{
			for (i = 0; i < 8; i++)
			{
				if (Password_set[i] != Password_input[i])
				{
					// ����
					wring_flag = 1;
					ring_flag = 1;
					// �����������
					for (j = 0; j < 8; j++)
					{
						Password_input[j] = 11;
					}
					break;
				}
			}
			if (i == 8)
			{
				// ������ȷ
				wring_flag = 0;
				ring_flag = 0;
				Seg_show_mode++; // ������һ������
			}
		}
		// ����ɾ��
		if (Key_Down == 6)
		{
			if (Password_input_index == 8)
				Password_input_index = 7;
			Password_input[Password_input_index] = 11;
			if (Password_input_index > 0 && Password_input_index < 7)
				Password_input_index--;
		}
		// ��������
		if (Key_Old == 5)
		{
			skip_flag = 1;
			if (time_1s == 1000)
			{
				Seg_show_mode++; // ��ת����һ������
			}
		}
		else
		{
			time_1s = skip_flag = 0; // ���ó���ʱ��
		}
		break;

	case 1:
		/* ������ */
		// ��Ϣ��ʾ��ť,����1s������Ϣ��ʾ���棬���ֻص�ʱ�ӽ���
		if (Key_Old == 7)
		{
			massage_show_time_flag = 1;
		}
		if (Key_Up == 7)
		{
			Init_Seg_Led();
			massage_show_time_flag = massage_show_flag = 0;
		}
		// ����/�ر�����
		if (Key_Down == 4)
		{
			alarm_open = ~alarm_open;
		}
		// �������ý���
		if (Key_Down == 6)
		{
			Seg_show_mode++;
			Init_Seg_Led();
		}
		break;
	case 2:
		/* ʱ�����ý��� */
		switch (Key_Down)
		{
		case 11:
			/* ����Сʱ */

			break;

		default:
			break;
		}
		break;
	}
}

/* ��Ϣ������ */
void Seg_Proc()
{
	uchar i;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	switch (Seg_show_mode)
	{
	case 0:
		/* �������� */
		if (Password_input_index)
		{
			for (i = 0; i < Password_input_index; i++)
			{
				Seg_Buf[7 - i] = Password_input[Password_input_index - i - 1];
			}
			for (i = Password_input_index; i < 8; i++)
			{
				Seg_Buf[7 - i] = 11;
			}
		}
		else
		{
			// û����������
			for (i = 0; i < 8; i++)
			{
				Seg_Buf[i] = 11;
			}
		}
		break;
	case 1:
		if (massage_show_flag)
		{
			uchar dis_value;
			uint temperature_value;
			dis_value = Ut_Wave_Data() % 100;
			temperature_value = rd_temperature() * 10;
			Seg_Buf[0] = dis_value / 10;
			Seg_Buf[1] = dis_value % 10;
			Seg_Buf[3] = 11;
			Seg_Buf[4] = temperature_value / 100;
			Seg_Buf[5] = (temperature_value % 100) / 10;
			Seg_Buf[6] = temperature_value % 10;
			Seg_Point[5] = 1;
			Seg_Buf[7] = 12; // C
		}
		else
		{
			Read_Rtc(ucRtc);
			Seg_Buf[0] = ucRtc[0] / 16;
			Seg_Buf[1] = ucRtc[0] % 16;
			Seg_Buf[2] = Seg_Buf[5] = (time_interval_flag) ? 11 : 10;
			Seg_Buf[3] = ucRtc[1] / 16;
			Seg_Buf[4] = ucRtc[1] % 16;
			Seg_Buf[6] = ucRtc[2] / 16;
			Seg_Buf[7] = ucRtc[2] % 16;
		}
		break;
	case 2: // ʱ��/��������
		uchar temp_time[3];

		break;
	}
}

/* ������ʾ���� */
void Led_Proc()
{
	Beep(ring_flag);
}

/* ���ڴ����� */
void Uart_Proc()
{
	if (Uart_Slow_Down)
		return;
	Uart_Slow_Down = 1; // ���ڼ��ٳ���
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
	if (++Uart_Slow_Down == 200)
		Uart_Slow_Down = 0; // ���ڼ���ר��
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // �������ʾר��
	// ��������
	if (skip_flag)
	{
		// ����1s
		time_1s = (++time_1s < 1000) ? time_1s : 1000;
	}
	if (++time_500ms == 500)
	{
		time_500ms = 0;
		time_interval_flag = ~time_interval_flag;
	}
	if (++time_700ms == 700)
	{
		time_700ms = 0;
		time_set_interval_flag = ~time_set_interval_flag;
	}
	if (massage_show_time_flag)
	{
		time_1s = (++time_1s < 1000) ? time_1s : 1000;
		if (time_1s == 1000)
		{
			massage_show_flag = 1;
		}
	}
	Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

/* ����1�жϷ����� */
void Uart1Server() interrupt 4
{
	if (RI == 1) // ���ڽ�������
	{
		Uart_Recv[Uart_Recv_Index] = SBUF;
		Uart_Recv_Index++;
		RI = 0;
	}
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
	Set_Rtc(ucRtc); // ���ó�ʼʱ��
	System_Init();
	Timer0Init();
	UartInit();
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
		Uart_Proc();
	}
}