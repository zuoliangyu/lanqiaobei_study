#include "main.h"
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组
uchar ucRtc[3] = {0x16, 0x59, 0x50};				 // 时间存放
/* 数据 */
uint light_100x;
uint temperature_10x;
uchar para_hour = 17;		 // 时间参数
uchar para_temperature = 25; // 温度参数
uchar para_led = 4;

/* 数码管显示模式 */
uchar Seg_show_mode;  // 0数据界面 1参数界面
uchar data_show_mode; // 0时间 1温度 2亮暗
uchar para_show_mode; // 0时间 1温度 2指示灯

/* 时间 */
uint time_dark_3s;
uint time_light_3s;
bit dark_flag; // 0亮 1暗
void init_Seg(uchar start, uchar end)
{
	uchar i;
	for (i = start; i < end; i++)
	{
		Seg_Buf[i] = 10;
		Seg_Point[i] = 0;
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
	if (Key_Down == 4)
	{
		init_Seg(0, 8);
		Seg_show_mode = (++Seg_show_mode) % 2;
		data_show_mode = para_show_mode = 0;
	}
	if (Key_Down == 5)
	{
		if (Seg_show_mode == 0)
		{
			init_Seg(0, 8);
			data_show_mode = (++data_show_mode) % 3;
		}
		else
		{
			init_Seg(2, 8);
			para_show_mode = (++para_show_mode) % 3;
		}
	}
	if (Seg_show_mode == 1)
	{
		if (Key_Down == 8)
		{
			switch (para_show_mode)
			{
			case 0:
				para_hour = (--para_hour >= 0) ? para_hour : 0;
				break;
			case 1:
				para_temperature = (--para_temperature >= 0) ? para_temperature : 0;
				break;
			case 2:
				para_led = (--para_led >= 4) ? para_led : 4;
			}
		}
		if (Key_Down == 9)
		{
			switch (para_show_mode)
			{
			case 0:
				para_hour = (++para_hour <= 23) ? para_hour : 23;
				break;
			case 1:
				para_temperature = (++para_temperature <= 99) ? para_temperature : 99;
				break;
			case 2:
				para_led = (++para_led <= 8) ? para_led : 8;
			}
		}
	}
}

/* 信息处理函数 */
void Seg_Proc()
{
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	Read_Rtc(ucRtc);   // 读取实时时钟
	temperature_10x = rd_temperature() * 10;
	light_100x = Ad_Read(0x01) * 100 / 51;
	dark_flag = (light_100x < 50) ? 1 : 0;
	switch (Seg_show_mode)
	{
	case 0:
		/* 数据界面 */
		switch (data_show_mode)
		{
		case 0:
			/* 时间数据 */
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
			/* 温度显示 */
			Seg_Buf[0] = 12; // C
			Seg_Buf[5] = temperature_10x / 100 % 10;
			Seg_Buf[6] = temperature_10x / 10 % 10;
			Seg_Buf[7] = temperature_10x % 10;
			Seg_Point[6] = 1;
			break;
		case 2:
			/* 亮暗显示 */
			Seg_Buf[0] = 13; // E
			Seg_Buf[2] = light_100x / 100 % 10;
			Seg_Buf[3] = light_100x / 10 % 10;
			Seg_Buf[4] = light_100x % 10;
			Seg_Point[2] = 1;
			Seg_Buf[7] = dark_flag;
			break;
		}
		break;

	case 1:
		/* 参数界面 */
		Seg_Buf[0] = 14; // P
		Seg_Buf[1] = para_show_mode + 1;
		switch (para_show_mode)
		{
		case 0:
			/* 时间参数 */
			Seg_Buf[6] = para_hour / 10 % 10;
			Seg_Buf[7] = para_hour % 10;
			break;
		case 1:
			/* 温度参数 */
			Seg_Buf[6] = para_temperature / 10 % 10;
			Seg_Buf[7] = para_temperature % 10;
			break;
		case 2:
			/* 指示灯参数 */
			Seg_Buf[7] = para_led % 10;
		}
		break;
	}
}

/* 其他显示函数 */
void Led_Proc()
{
	uchar hour;
	hour = ucLed[0] / 16 * 10 + ucLed[0] % 16;
	ucLed[0] = ((para_hour <= hour) && (hour < 8 + 24));
	ucLed[1] = (temperature_10x < para_temperature * 10);
	if (dark_flag && time_dark_3s >= 3000)
	{
		ucLed[2] = 1;
	}
	else if (!dark_flag && time_light_3s >= 3000)
	{
		ucLed[2] = 0;
	}
	if (dark_flag)
		ucLed[para_led - 1] = 1;
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
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // 数码管显示专用
	if (dark_flag)	 // 被遮挡
	{
		time_light_3s = 0;
		if (++time_dark_3s >= 3000)
			time_dark_3s = 3001;
	}
	else // 未被遮挡
	{
		time_dark_3s = 0;
		if (++time_light_3s >= 3000)
			time_light_3s = 3001;
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
	// 如果有温度读取的话
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