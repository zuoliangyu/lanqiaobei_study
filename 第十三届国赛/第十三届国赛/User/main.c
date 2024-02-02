#include "main.h"
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组

uchar Seg_show_mode; // 显示界面 0频率 1湿度 2测距 3参数
uchar celi_mode;	 // 参数界面 0频率 1湿度 2距离
/*时间*/
uint time_1s;
uint time_up_1s;
uchar time_100ms;
uchar pwn_count;
/*测量数据*/
uint freq;
uchar humidity;
uchar distance;

uint freq_demo = 9000;
uint humidity_demo = 40;
uint dis_demo = 60;
bit dis_flag;  // 0为cm，1为m
bit freq_flag; // 0为Hz，1为kHz

bit reset_relay_flag;
uchar relay_count;
bit light_flag;
void init_Seg_LED() // 初始化数码管和LED
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		ucLed[i] = 0;
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
		Seg_show_mode = (++Seg_show_mode) % 4;
		celi_mode = 0;
		init_Seg_LED();
	}
	if (Seg_show_mode == 3 && Key_Down == 5)
	{
		celi_mode = (++celi_mode) % 3;
		init_Seg_LED();
	}
	if (Key_Down == 6)
	{
		switch (Seg_show_mode)
		{
		case 2:
			dis_flag ^= 1;
			Seg_Point[5] = 0;
			break;
		case 3:
			/* 参数设置 */
			switch (celi_mode)
			{
			case 0:
				/* 频率参数 */
				freq_demo = (freq_demo + 500 > 12000) ? 1000 : freq_demo + 500;
				break;
			case 1:
				/*湿度参数*/
				humidity_demo = (humidity_demo + 10 > 60) ? 10 : humidity_demo + 10;
				break;
			case 2:
				/*距离参数*/
				dis_demo = (dis_demo + 10 > 120) ? 10 : dis_demo + 10;
				break;
			}
			break;
		}
	}
	if (Key_Down == 7)
	{
		switch (Seg_show_mode)
		{
		case 0:
			/* 频率显示 */
			freq_flag ^= 1;
			Seg_Point[6] = 0;
			break;
		case 3:
			/* 参数设置 */
			switch (celi_mode)
			{
			case 0:
				/* 频率参数 */
				freq_demo = (freq_demo - 500 < 1000) ? 12000 : freq_demo - 500;
				break;
			case 1:
				/*湿度参数*/
				humidity_demo = (humidity_demo - 10 < 10) ? 60 : humidity_demo - 10;
				break;
			case 2:
				/*距离参数*/
				dis_demo = (dis_demo - 10 < 10) ? 120 : dis_demo - 10;
				break;
			}
			break;
		}
	}
	if (Key_Down == 7 && Seg_show_mode == 1)
		reset_relay_flag = 1; // 按下开始计时
	else
		reset_relay_flag = 0;
	// 抬起检测
	if (Key_Up == 7 && time_up_1s >= 1000)
		reset_relay_flag = 0;
}

/* 信息处理函数 */
void Seg_Proc()
{
	uchar temp_freq, i, temp_dis;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	humidity = Ad_Read(0x03) / 51.0 * 20;
	/*湿度处理*/
	if (humidity > 80)
		Da_Write(255);
	else if (humidity < humidity_demo)
		Da_Write(51);
	else
		Da_Write(255 - 204 * (80 - humidity) / (80 - humidity_demo));
	/*测距处理*/
	distance = Ut_Wave_Data();
	if (distance > dis_demo)
	{
		Relay(1);
		relay_count++;
		EEPROM_Write(&relay_count, 0, 1);
	}
	else
	{
		Relay(0);
	}
	switch (Seg_show_mode)
	{
	case 0:
		/* 频率显示 */
		Seg_Buf[0] = 11; // F
		// KHz
		if (freq_flag)
		{
			temp_freq = freq / 100;
			Seg_Buf[3] = temp_freq / 10000 % 10;
			Seg_Buf[4] = temp_freq / 1000 % 10;
			Seg_Buf[5] = temp_freq / 100 % 10;
			Seg_Buf[6] = temp_freq / 10 % 10;
			Seg_Buf[7] = temp_freq % 10;
			Seg_Point[6] = 1;
			for (i = 3; i < 6; i++)
			{
				if (Seg_Buf[i] == 0)
					Seg_Buf[i] = 10;
				else
					break;
			}
		}
		// Hz
		else
		{
			Seg_Buf[3] = freq / 10000 % 10;
			Seg_Buf[4] = freq / 1000 % 10;
			Seg_Buf[5] = freq / 100 % 10;
			Seg_Buf[6] = freq / 10 % 10;
			Seg_Buf[7] = freq % 10;
			for (i = 3; i < 7; i++)
			{
				if (Seg_Buf[i] == 0)
					Seg_Buf[i] = 10;
				else
					break;
			}
		}
		break;
	case 1:
		/*湿度显示*/
		Seg_Buf[0] = 12; // H
		Seg_Buf[6] = humidity / 10 % 10;
		Seg_Buf[7] = humidity % 10;
		break;
	case 2:
		/*测距*/
		Seg_Buf[0] = 13; // A
		if (dis_flag)
		{
			Seg_Buf[5] = distance / 100;
			Seg_Buf[6] = distance / 10 % 10;
			Seg_Buf[7] = distance % 10;
			Seg_Point[5] = 1;
		}
		else
		{
			Seg_Buf[5] = (distance / 100 == 0) ? 10 : distance / 100;
			Seg_Buf[6] = distance / 10 % 10;
			Seg_Buf[7] = distance % 10;
		}

		break;
	case 3:
		/*参数*/
		Seg_Buf[0] = 14; // P
		Seg_Buf[1] = celi_mode + 1;
		switch (celi_mode)
		{
		case 0:
			/* 频率 */
			temp_freq = freq_demo / 100;
			Seg_Buf[5] = (temp_freq / 100 == 0) ? 10 : temp_freq / 100;
			Seg_Buf[6] = temp_freq / 10 % 10;
			Seg_Buf[7] = temp_freq % 10;
			Seg_Point[6] = 1;
			break;
		case 1:
			/* 湿度 */
			Seg_Buf[6] = humidity_demo / 10;
			Seg_Buf[7] = humidity_demo % 10;
			break;
		case 2:
			/* 距离 */
			temp_dis = dis_demo / 10;
			Seg_Buf[6] = temp_dis / 10;
			Seg_Buf[7] = temp_dis % 10;
			Seg_Point[6] = 1;
			break;
		}
		break;
	}
}

/* 其他显示函数 */
void Led_Proc()
{

	switch (Seg_show_mode)
	{
	case 0:
		ucLed[0] = 1;
		break;
	case 1:
		ucLed[1] = 1;
		break;
	case 2:
		ucLed[2] = 1;
		break;
	case 3:
		ucLed[0] = (celi_mode == 0) ? light_flag : 0;
		ucLed[1] = (celi_mode == 1) ? light_flag : 0;
		ucLed[2] = (celi_mode == 2) ? light_flag : 0;
		break;
	}
	ucLed[3] = (freq > freq_demo);
	ucLed[4] = (humidity > humidity_demo);
	ucLed[5] = (distance > dis_demo);
}

/* 定时器0中断初始化函数 */
void Timer0Init(void) // 1毫秒@12.000MHz
{
	AUXR &= 0x7F; // 定时器时钟12T模式
	TMOD &= 0xF0; // 设置定时器模式
	TMOD |= 0x05; // 设置定时器0为16位不重载
	TL0 = 0;	  // 设置定时初始值
	TH0 = 0;	  // 设置定时初始值
	TF0 = 0;	  // 清除TF0标志
	TR0 = 1;	  // 定时器0开始计时
}

void Timer2_Init(void) // 1毫秒@12.000MHz
{
	AUXR &= 0xFB; // 定时器时钟12T模式
	T2L = 0x18;	  // 设置定时初始值
	T2H = 0xFC;	  // 设置定时初始值
	AUXR |= 0x10; // 定时器2开始计时
	IE2 |= 0x04;  // 开定时器2中断
	EA = 1;		  // 总中断打开
}
void Timer3_Init(void) // 100微秒@12.000MHz
{
	T4T3M &= 0xFD; // 定时器时钟12T模式
	T3L = 0x9C;	   // 设置定时初始值
	T3H = 0xFF;	   // 设置定时初始值
	T4T3M |= 0x08; // 定时器3开始计时
	IE2 |= 0x20;   // 开定时器3中断
	EA = 1;		   // 总中断打开
}

/* 定时器2中断服务函数 */
void Timer2Server() interrupt 12
{
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // 键盘减速专用
	if (++Seg_Slow_Down == 500)
		Seg_Slow_Down = 0; // 数码管减速专用
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // 数码管显示专用
	if (++time_1s == 1000)
	{
		freq = TH0 << 8 | TL0;
		TH0 = 0;
		TL0 = 0;
		time_1s = 0;
	}
	if (reset_relay_flag)
	{
		if (++time_up_1s >= 1000)
		{
			time_up_1s = 1000;
		}
	}
	else
	{
		time_up_1s = 0;
	}
	if (++time_100ms == 100)
		light_flag ^= 1;
	Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}
void Timer3Server() interrupt 19
{
	if (++pwn_count == 10)
		pwn_count = 0;
	if (freq > freq_demo)
		MOTOR(pwn_count <= 8);
	else
		MOTOR(pwn_count <= 2);
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
	System_Init();
	EEPROM_Read(&relay_count, 0, 1);
	Timer0Init();
	Timer2_Init();
	Timer3_Init();
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}