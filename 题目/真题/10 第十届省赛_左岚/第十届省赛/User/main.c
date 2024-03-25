#include "main.h"
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组
uchar Uart_Slow_Down;								 // 串口减速专用变量
uchar Seg_show_mode;								 // 0电压 1频率
uint voltage_value_100x;
uchar DAC_value = 102; // 2*51
uint frequency_value;
uint time_1s;
bit output_mode; // 0固定DAC=2, 1DAC=RB2
bit LED_mode;	 // 0开启功能 1关闭功能
bit Seg_mode;	 // 0开启数码管 1关闭数码管
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
/* 信息处理函数 */
void Seg_Proc()
{
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序

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
			/* 电压测量 */
			Seg_Buf[0] = 11; // U
			Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
			Seg_Buf[5] = voltage_value_100x / 100;
			Seg_Buf[6] = voltage_value_100x / 10 % 10;
			Seg_Buf[7] = voltage_value_100x % 10;
			Seg_Point[5] = 1;
			break;
		case 1:
			/* 频率测量 */
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

/* 其他显示函数 */
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

// 计数器
void Timer0Init(void) // 0微秒@12.000MHz
{
	AUXR &= 0x7F; // 定时器时钟12T模式
	TMOD &= 0xF0; // 设置定时器模式
	TMOD |= 0x05;
	TL0 = 0x00; // 设置定时初值
	TH0 = 0x00; // 设置定时初值
	TF0 = 0;	// 清除TF0标志
	TR0 = 1;	// 定时器0开始计时
}
// 定时器
void Timer1Init(void) // 1毫秒@12.000MHz
{
	AUXR &= 0xBF; // 定时器时钟12T模式
	TMOD &= 0x0F; // 设置定时器模式
	TL1 = 0x18;	  // 设置定时初值
	TH1 = 0xFC;	  // 设置定时初值
	TF1 = 0;	  // 清除TF1标志
	TR1 = 1;	  // 定时器1开始计时
	ET1 = 1;	  // 允许定时器1中断
	EA = 1;		  // 允许总中断
}

/* 定时器1中断服务函数 */
void Timer1Server() interrupt 3
{
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // 键盘减速专用
	if (++Seg_Slow_Down == 200)
		Seg_Slow_Down = 0; // 数码管减速专用
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // 数码管显示专用
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