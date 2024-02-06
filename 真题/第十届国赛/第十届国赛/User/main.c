#include "main.h"
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组
uchar Uart_Slow_Down;								 // 串口减速专用变量
uchar Uart_Recv[10];								 // 串口接收数据储存数组 默认10个字节 若接收数据较长 可更改最大字节数
uchar Uart_Recv_Index;								 // 串口接收数组指针
/*时间*/
uint time_1s_reset;
uint time_1s_DAC;
bit reset_count_flag;
bit DAC_change_flag;
bit DAC_out_mode;

/* 界面切换 */
bit Seg_show_mode;	  // 0数据界面 1参数界面
uchar data_show_mode; // 0温度数据 1距离数据 2变更次数
bit para_show_mode;	  // 0温度参数 1距离参数

/* 数据记录 */
uint temperature_100x;	// 100倍的温度数据
uchar dis_value;		// 距离数据
uint change_count;		// 变更次数
uchar temperature_demo; // 温度参数
uchar dis_demo;			// 距离参数
void init_Seg()
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		Seg_Buf[i] = 10;  // 数码管显示数据初始化
		Seg_Point[i] = 0; // 数码管小数点数据初始化
	}
}
/* 键盘处理函数 */
void Key_Proc()
{
	static uchar Key_Val, Key_Down, Key_Old, Key_Up; // 按键专用变量
	if (Key_Slow_Down)
		return;
	Key_Slow_Down = 1; // 键盘减速程序

	Key_Val = Key_Read();					  // 实时读取键码值
	Key_Down = Key_Val & (Key_Old ^ Key_Val); // 捕捉按键下降沿
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // 捕捉按键上降沿
	Key_Old = Key_Val;						  // 辅助扫描变量
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
				/* 参数界面*/
				para_show_mode ^= 1;
			else
			{
				/* 数据界面*/
				init_Seg();
				data_show_mode = (++data_show_mode) % 3;
			}
		}
	}
	if (Seg_show_mode)
	{
		if (para_show_mode)
		{
			/* 距离参数 */
			if (Key_Down == 16)
				dis_demo = (dis_demo - 5 < 0) ? 0 : (dis_demo - 5);
			if (Key_Down == 17)
				dis_demo = (dis_demo + 5 > 99) ? 99 : (dis_demo + 5);
		}
		else
		{
			/* 温度参数 */
			if (Key_Down == 16)
				temperature_demo = (temperature_demo - 2 < 0) ? 0 : (temperature_demo - 2);
			if (Key_Down == 17)
				temperature_demo = (temperature_demo + 2 > 99) ? 99 : (temperature_demo + 2);
		}
	}
}

/* 信息处理函数 */
void Seg_Proc()
{
	uchar i;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	temperature_100x = rd_temperature() * 100;
	dis_value = Ut_Wave_Data();
	if (Seg_show_mode)
	{
		/* 参数界面 */
		switch (data_show_mode)
		{
		case 0:
			/* 温度数据 */
			Seg_Buf[0] = 11; // C
			Seg_Buf[4] = temperature_100x / 1000 % 10;
			Seg_Buf[5] = temperature_100x / 100 % 10;
			Seg_Buf[6] = temperature_100x / 10 % 10;
			Seg_Buf[7] = temperature_100x % 10;
			Seg_Point[5] = 1;
			break;
		case 1:
			/* 距离数据 */
			Seg_Buf[0] = 12; // L
			Seg_Buf[6] = dis_value / 10 % 10;
			Seg_Buf[7] = dis_value % 10;
			break;
		case 2:
			/* 变更次数 */
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
		/* 数据界面 */
		Seg_Buf[0] = 14; // P
		Seg_Buf[3] = (uchar)para_show_mode + 1;
		if (para_show_mode)
		{
			/* 距离参数 */
			Seg_Buf[6] = dis_demo / 10; // 十位
			Seg_Buf[7] = dis_demo % 10; // 个位
		}
		else
		{
			/* 温度参数 */
			Seg_Buf[6] = temperature_demo / 10; // 十位
			Seg_Buf[7] = temperature_demo % 10; // 个位
		}
	}
}

/* 其他显示函数 */
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
/* 串口处理函数 */
void Uart_Proc()
{
	if (Uart_Slow_Down)
		return;
	Uart_Slow_Down = 1; // 串口减速程序
	// 当有数据输入的时候
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

/* 定时器0中断初始化函数 */
void Timer0Init(void) // 1毫秒@12.000MHz
{
	AUXR &= 0x7F; // 定时器时钟12T模式
	TMOD &= 0xF0; // 设置定时器模式
	TL0 = 0x18;	  // 设置定时初始值
	TH0 = 0xFC;	  // 设置定时初始值
	TF0 = 0;	  // 清除TF0标志
	TR0 = 1;	  // 定时器0开始计时
	ET0 = 1;	  // 定时器中断0打开
	EA = 1;		  // 总中断打开
}

/* 定时器0中断服务函数 */
void Timer0Server() interrupt 1
{
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // 键盘减速专用
	if (++Seg_Slow_Down == 500)
		Seg_Slow_Down = 0; // 数码管减速专用
	if (++Uart_Slow_Down == 200)
		Uart_Slow_Down = 0; // 串口减速专用
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // 数码管显示专用
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

/* 串口1中断服务函数 */
void Uart1Server() interrupt 4
{
	if (RI == 1) // 串口接收数据
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
	// 如果有温度读取的话
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