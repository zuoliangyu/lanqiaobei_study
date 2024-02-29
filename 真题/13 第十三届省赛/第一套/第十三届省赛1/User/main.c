#include "main.h"
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组

uchar Seg_show_mode; // 0温度，1时间，2参数设置

uint int_temperature_value;
uchar ucRtc[3] = {0x01, 0x59, 0x50};
uchar temperature_demo = 23;

bit control_mode;	// 0温度，1时间
bit time_show_flag; // 0时分，1分秒
uint time_5s;
uchar time_100ms;
bit time_relay_flag;		// 0非整点，1整点
bit temperature_relay_flag; // 0非超过温度，1超过温度

void init_Seg_LED()
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		Seg_Buf[i] = 10;
		Seg_Point[i] = 0;
		ucLed[i] = 0;
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
	switch (Key_Down)
	{
	case 12:
		init_Seg_LED();
		if (++Seg_show_mode > 2)
			Seg_show_mode = 0;
		break;

	case 13:
		control_mode = ~control_mode;
		break;
	case 16:
		if (Seg_show_mode == 2)
			if (++temperature_demo > 99)
				temperature_demo = 99;
		break;
	case 17:
		if (Seg_show_mode == 2)
			if (--temperature_demo < 10)
				temperature_demo = 10;
		if (Seg_show_mode == 1)
			time_show_flag = 1;
		break;
	}
	if (Key_Up == 17)
		time_show_flag = 0;
}

/* 信息处理函数 */
void Seg_Proc()
{

	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	Seg_Buf[0] = 11;   // U
	Seg_Buf[1] = Seg_show_mode + 1;
	Seg_Buf[2] = 10; // 灭
	switch (Seg_show_mode)
	{
	case 0:
		Seg_Buf[3] = Seg_Buf[4] = 10;
		int_temperature_value = rd_temperature() * 10;
		Seg_Buf[5] = int_temperature_value / 100;
		Seg_Buf[6] = (int_temperature_value / 10) % 10;
		Seg_Buf[7] = int_temperature_value % 10;
		Seg_Point[6] = 1;
		break;
	case 1:
		Read_Rtc(ucRtc);
		Seg_Buf[3] = (time_show_flag) ? ucRtc[1] / 16 : ucRtc[0] / 16;
		Seg_Buf[4] = (time_show_flag) ? ucRtc[1] % 16 : ucRtc[0] % 16;
		Seg_Buf[5] = 12;
		Seg_Buf[6] = (time_show_flag) ? ucRtc[2] / 16 : ucRtc[1] / 16;
		Seg_Buf[7] = (time_show_flag) ? ucRtc[2] % 16 : ucRtc[1] % 16;
		break;
	case 2:
		Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
		Seg_Buf[6] = temperature_demo / 10;
		Seg_Buf[7] = temperature_demo % 10;
		break;
	}
}

/* 其他显示函数 */
void Led_Proc()
{
	if (control_mode)
	{
		temperature_relay_flag = 0; // 防止突然切换，导致继电器卡
		if (ucRtc[1] / 16 == 0 && ucRtc[1] % 16 == 0 && ucRtc[2] / 16 == 0 && ucRtc[2] % 16 == 0)
		{
			time_relay_flag = 1;
		}
		Relay(time_relay_flag); // 定时器控制继电器
	}
	else
	{
		time_relay_flag = 0; // 防止突然切换，导致继电器卡
		if ((float)int_temperature_value / 10.0 > temperature_demo)
			temperature_relay_flag = 1;
		else
			temperature_relay_flag = 0;
		Relay(temperature_relay_flag);
	}
	ucLed[0] = time_relay_flag; // 整点点亮，5s熄灭
	ucLed[1] = ~control_mode;	// 温度控制时点亮，时间控制熄灭
	ucLed[2] = time_100ms;		// 100ms闪烁
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
	if (time_relay_flag)
	{
		if (++time_5s == 5000)
		{
			time_5s = 0;
			time_relay_flag = 0;
		}
	}
	// 触发继电器
	if (time_relay_flag || temperature_relay_flag)
		if (++time_100ms == 100)
			time_100ms = 0;

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
	Set_Rtc(ucRtc);
	System_Init();
	Timer0Init();
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}