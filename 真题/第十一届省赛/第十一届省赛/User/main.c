#include "main.h"
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组

/* 数据显示 */
uchar Seg_show_mode; // 0数据 1参数 2计数

uint old_vol;			  // 上一次的电压值
uchar vol_demo;			  // 参考电压0-50
unsigned long count_down; // 下降沿计数
bit count_down_flag;	  // 下降沿计数标志位
/* 时间 */
uint time_5s;

uchar error_count;
void init_Seg()
{
	uchar i;
	for (i = 0; i < 8; i++)
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
	switch (Key_Down)
	{
	case 12:
		if (Seg_show_mode == 0)
		{
			EEPROM_Write(&vol_demo, 0, 1);
		}
		Seg_show_mode = (++Seg_show_mode) % 3;
		error_count = 0;
		break;
	case 16:
		if (Seg_show_mode == 1)
			vol_demo = (vol_demo + 5 > 50 ? 0 : vol_demo + 5);
		error_count = 0;
		break;
	case 17:
		if (Seg_show_mode == 1)
			vol_demo = (vol_demo - 5 < 0 ? 50 : vol_demo - 5);
		error_count = 0;
		break;
	case 13:
		if (Seg_show_mode == 2)
			count_down = 0;
		error_count = 0;
		break;
	default:
		error_count = (++error_count >= 3) ? 3 : error_count;
		break;
	}
}

/* 信息处理函数 */
void Seg_Proc()
{
	uint real_V;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	switch (Seg_show_mode)
	{
	case 0:
		/* 数据显示 */
		Seg_Buf[0] = 11;				   // U
		real_V = Ad_Read(0x03) * 100 / 51; // 0-255->0-500
		// 当检测到下降沿的时候计数+1
		if (old_vol > vol_demo * 10 && real_V < vol_demo * 10)

			count_down++;
		if (real_V < vol_demo * 10)
			count_down_flag = 1;
		else
			count_down_flag = 0;
		old_vol = real_V;
		Seg_Buf[5] = real_V / 100 % 10; // 百位
		Seg_Buf[6] = real_V % 100 / 10; // 十位
		Seg_Buf[7] = real_V % 10;		// 个位
		Seg_Point[5] = 1;
		break;

	case 1:
		/*参数设置*/
		Seg_Buf[0] = 12;				  // P
		Seg_Buf[5] = vol_demo / 100 % 10; // 百位
		Seg_Buf[6] = vol_demo / 10 % 10;  // 十位
		Seg_Buf[7] = vol_demo % 10;		  // 个位
		Seg_Point[5] = 1;
		break;
	case 2:
		/*计数界面*/
		Seg_Buf[0] = 13; // N
		Seg_Buf[1] = count_down / 10000000 % 10;
		Seg_Buf[2] = count_down / 1000000 % 10;
		Seg_Buf[3] = count_down / 100000 % 10;
		Seg_Buf[4] = count_down / 10000 % 10;
		Seg_Buf[5] = count_down / 1000 % 10;
		Seg_Buf[6] = count_down / 100 % 10;
		Seg_Buf[7] = count_down % 10;
		break;
	}
}

/* 其他显示函数 */
void Led_Proc()
{
	ucLed[0] = (time_5s >= 5000);
	ucLed[1] = (count_down % 2);
	ucLed[2] = (error_count >= 3);
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
	if (count_down_flag)
	{
		if (++time_5s >= 5000)
			time_5s = 5000;
	}
	else
		time_5s = 0;
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
	EEPROM_Read(&vol_demo, 0, 1);
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}