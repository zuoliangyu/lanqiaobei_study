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
/*ʱ��*/
uint time_1s_reset;
uint time_1s_DAC;
bit reset_count_flag;
bit DAC_change_flag;
bit DAC_out_mode;

/* �����л� */
bit Seg_show_mode;	  // 0���ݽ��� 1��������
uchar data_show_mode; // 0�¶����� 1�������� 2�������
bit para_show_mode;	  // 0�¶Ȳ��� 1�������

/* ���ݼ�¼ */
uint temperature_100x;	// 100�����¶�����
uchar dis_value;		// ��������
uint change_count;		// �������
uchar temperature_demo; // �¶Ȳ���
uchar dis_demo;			// �������
void init_Seg()
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		Seg_Buf[i] = 10;  // �������ʾ���ݳ�ʼ��
		Seg_Point[i] = 0; // �����С�������ݳ�ʼ��
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
	if (Key_Down == 13)
		reset_count_flag = 1;
	if (Key_Up == 13)
	{
		if (time_1s_reset >= 1000)
		{
			change_count = 0;
			EEPROM_Write(&change_count, 0, 1);
		}
		else
		{
			init_Seg();
			if (Seg_show_mode)
			{
				change_count++;
				EEPROM_Write(&change_count, 0, 1);
			}
			Seg_show_mode ^= 1;
			data_show_mode = para_show_mode = 0;
		}
		reset_count_flag = time_1s_reset = 0;
	}
	if (Key_Down == 12)
	{
		DAC_change_flag = 1;
	}
	if (Key_Up == 12)
	{
		if (time_1s_DAC >= 1000)
			DAC_out_mode ^= 1;
		else
		{
			if (Seg_show_mode)
				/* ��������*/
				para_show_mode ^= 1;
			else
			{
				/* ���ݽ���*/
				init_Seg();
				data_show_mode = (++data_show_mode) % 3;
			}
		}
	}
	if (Seg_show_mode)
	{
		if (para_show_mode)
		{
			/* ������� */
			if (Key_Down == 16)
				dis_demo = (dis_demo - 5 < 0) ? 0 : (dis_demo - 5);
			if (Key_Down == 17)
				dis_demo = (dis_demo + 5 > 99) ? 99 : (dis_demo + 5);
		}
		else
		{
			/* �¶Ȳ��� */
			if (Key_Down == 16)
				temperature_demo = (temperature_demo - 2 < 0) ? 0 : (temperature_demo - 2);
			if (Key_Down == 17)
				temperature_demo = (temperature_demo + 2 > 99) ? 99 : (temperature_demo + 2);
		}
	}
}

/* ��Ϣ������ */
void Seg_Proc()
{
	uchar i;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // ����ܼ��ٳ���
	temperature_100x = rd_temperature() * 100;
	dis_value = Ut_Wave_Data();
	if (Seg_show_mode)
	{
		/* �������� */
		switch (data_show_mode)
		{
		case 0:
			/* �¶����� */
			Seg_Buf[0] = 11; // C
			Seg_Buf[4] = temperature_100x / 1000 % 10;
			Seg_Buf[5] = temperature_100x / 100 % 10;
			Seg_Buf[6] = temperature_100x / 10 % 10;
			Seg_Buf[7] = temperature_100x % 10;
			Seg_Point[5] = 1;
			break;
		case 1:
			/* �������� */
			Seg_Buf[0] = 12; // L
			Seg_Buf[6] = dis_value / 10 % 10;
			Seg_Buf[7] = dis_value % 10;
			break;
		case 2:
			/* ������� */
			Seg_Buf[0] = 13; // N
			Seg_Buf[3] = change_count / 10000 % 10;
			Seg_Buf[4] = change_count / 1000 % 10;
			Seg_Buf[5] = change_count / 100 % 10;
			Seg_Buf[6] = change_count / 10 % 10;
			Seg_Buf[7] = change_count % 10;
			for (i = 3; i < 6; i++)
			{
				if (Seg_Buf[i] != 0)
					break;
				Seg_Buf[i] = 10;
			}

			break;
		}
	}
	else
	{
		/* ���ݽ��� */
		Seg_Buf[0] = 14; // P
		Seg_Buf[3] = (uchar)para_show_mode + 1;
		if (para_show_mode)
		{
			/* ������� */
			Seg_Buf[6] = dis_demo / 10; // ʮλ
			Seg_Buf[7] = dis_demo % 10; // ��λ
		}
		else
		{
			/* �¶Ȳ��� */
			Seg_Buf[6] = temperature_demo / 10; // ʮλ
			Seg_Buf[7] = temperature_demo % 10; // ��λ
		}
	}
}

/* ������ʾ���� */
void Led_Proc()
{
	if (DAC_out_mode)
	{
		if (dis_value <= dis_demo)
			Da_Write(102); // 2*51
		else
			Da_Write(204); // 4*51
	}
	else
		Da_Write(20); // 0.4*51
	ucLed[0] = (temperature_100x / 100.0 >= temperature_demo);
	ucLed[1] = (dis_value < dis_demo);
	ucLed[2] = DAC_out_mode;
}
/* ���ڴ����� */
void Uart_Proc()
{
	if (Uart_Slow_Down)
		return;
	Uart_Slow_Down = 1; // ���ڼ��ٳ���
	// �������������ʱ��
	if (Uart_Recv[0] != NULL)
	{
		if (Uart_Recv[0] == 'S' && Uart_Recv[1] == 'T' && Uart_Recv[2] == '\r' && Uart_Recv[3] == '\n')
			printf("$%d,%0.2f\r\n", (uint)dis_value, (float)temperature_100x / 100.0);
		else if (Uart_Recv[0] == 'P' && Uart_Recv[1] == 'A' && Uart_Recv[2] == 'R' && Uart_Recv[3] == 'A' &&
				 Uart_Recv[4] == '\r' && Uart_Recv[5] == '\n')
			printf("#%d,%d\r\n", dis_demo, temperature_demo);
		else
			printf("error\r\n");
		memset(Uart_Recv, 0, sizeof(Uart_Recv));
		Uart_Recv_Index = 0;
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
	if (++Uart_Slow_Down == 200)
		Uart_Slow_Down = 0; // ���ڼ���ר��
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // �������ʾר��
	if (reset_count_flag)
	{
		if (++time_1s_reset >= 1000)
			time_1s_reset = 1000;
	}
	else
	{
		time_1s_reset = 0;
	}
	if (DAC_change_flag)
	{
		if (++time_1s_DAC >= 1000)
			time_1s_DAC = 1000;
	}
	else
	{
		time_1s_DAC = 0;
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

	System_Init();
	Timer0Init();
	Uart1_Init();
	EEPROM_Read(&change_count, 0, 1);
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
		Uart_Proc();
	}
}