#include "main.h"
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组

uchar Seg_show_mode; // 0温度，1参数，2DAC
float temperature_value;
uchar temperature_demo = 25;
uchar DAC_out_digit;
bit DAC_flag; // 0为参数 1为温度关系
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
	case 4:
		init_Seg();
		Seg_show_mode = (++Seg_show_mode) % 3;
		break;
	case 8:
		if (Seg_show_mode == 1)
			temperature_demo = (--temperature_demo < 0) ? 0 : temperature_demo;
		break;
	case 9:
		if (Seg_show_mode == 1)
			temperature_demo = (++temperature_demo > 99) ? 99 : temperature_demo;
		break;
	case 5:
		DAC_flag ^= 1;
		break;
	}
}

/* 信息处理函数 */
void Seg_Proc()
{
	uint temp_temperature;
	uint DAC_out_analog_x100;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	temperature_value = rd_temperature();
	temp_temperature = temperature_value * 100;
	if (DAC_flag)
	{
		if (temperature_value >= 40)
			DAC_out_digit = 204; // 4*255/5
		else if (temperature_value <= 20)
			DAC_out_digit = 51; // 1*255/5
		else
			DAC_out_digit = ((temperature_value - 20) * 3.0 / 20.0 + 1.0) * 51;
	}
	DAC_out_analog_x100 = DAC_out_digit / 51.0 * 100;
	switch (Seg_show_mode)
	{
	case 0:
		/* 温度界面 */
		Seg_Buf[0] = 11; // C
		Seg_Buf[4] = temp_temperature / 1000 % 10;
		Seg_Buf[5] = temp_temperature / 100 % 10;
		Seg_Buf[6] = temp_temperature / 10 % 10;
		Seg_Buf[7] = temp_temperature % 10;
		Seg_Point[5] = 1;
		break;
	case 1:
		/*参数界面*/
		Seg_Buf[0] = 12; // P
		Seg_Buf[6] = temperature_demo / 10 % 10;
		Seg_Buf[7] = temperature_demo % 10;
		break;
	case 2:
		/*DAC输出*/
		Seg_Buf[0] = 13; // A
		Seg_Buf[5] = DAC_out_analog_x100 / 100 % 10;
		Seg_Buf[6] = DAC_out_analog_x100 / 10 % 10;
		Seg_Buf[7] = DAC_out_analog_x100 % 10;
		Seg_Point[5] = 1;
		break;
	}
}

/* 其他显示函数 */
void Led_Proc()
{
	Da_Write(DAC_out_digit);
	ucLed[0] = !DAC_flag;
	ucLed[1] = (Seg_show_mode == 0);
	ucLed[2] = (Seg_show_mode == 1);
	ucLed[3] = (Seg_show_mode == 2);
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

	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}