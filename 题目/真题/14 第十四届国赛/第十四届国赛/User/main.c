#include "main.h"
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组

/* 显示 */
uchar Seg_Show_mode;  // 0 测距 1 参数 2 工厂
uchar dis_show_mode;  // 0 cm 1 m
uchar para_show_mode; // 0 距离 1 温度
uchar fac_show_mode;  // 0 校准值 1 介质 2 DAC输出

/* 数据 */
uint temperature_data_10x; // 10倍的温度值
uint distance_data;		   // 距离值 存储的时候，单位为cm
uint write_distance_data;  // 记录距离值 存储的时候，单位为cm
uchar para_temperature;	   // 温度参数
uchar para_distance;	   // 距离参数
uchar dac_output_data;	   // DAC输出值

char calibration_value;	  // 校准值
uint transmission_speed;  // 传输速度
uchar dac_low_output_10x; // DAC输出下限*10

/* 判断 */
bit write_distance_data_flag; // 距离写入标志
bit reset_flag;				  // 复位标志
bit led_show_flag;			  // Led显示标志
/* 时间 */
uint time_6s;
uint time_2s;
uchar time_100ms;
void set_Seg_init(uchar start, uchar end)
{
	uchar i;
	for (i = start; i < end; i++)
	{
		Seg_Buf[i] = 10;
		Seg_Point[i] = 0;
	}
}
void reset_data()
{
	Seg_Show_mode = 0;
	dis_show_mode = 0;
	para_distance = 40;
	para_temperature = 30;
	calibration_value = 0;
	transmission_speed = 340;
	dac_low_output_10x = 10;
}
/* 键盘处理函数 */
void Key_Proc()
{
	static uchar Key_Val, Key_Down, Key_Old, Key_Up; // 按键专用变量
	float temp_V;
	if (Key_Slow_Down)
		return;
	Key_Slow_Down = 1; // 键盘减速程序

	Key_Val = Key_Read();					  // 实时读取键码值
	Key_Down = Key_Val & (Key_Old ^ Key_Val); // 捕捉按键下降沿
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // 捕捉按键上降沿
	Key_Old = Key_Val;						  // 辅助扫描变量
	// 没有进行写入，全部按键有效
	if (!write_distance_data_flag)
	{
		if (Key_Down == 4)
		{
			Seg_Show_mode = (++Seg_Show_mode) % 3;
			set_Seg_init(0, 8);
			dis_show_mode = para_show_mode = fac_show_mode = 0;
		}
		if (Key_Down == 5)
		{
			switch (Seg_Show_mode)
			{
			case 0:
				dis_show_mode = (++dis_show_mode) % 2;
				set_Seg_init(4, 8);
				break;

			case 1:
				para_show_mode = (++para_show_mode) % 2;
				break;
			case 2:
				fac_show_mode = (++fac_show_mode) % 3;
				set_Seg_init(2, 8);
				break;
			}
		}
		switch (Seg_Show_mode)
		{
		case 0:
			/* 测距界面 */
			if (Key_Down == 8)
			{
				write_distance_data = distance_data;
				write_distance_data_flag = 1;
			}
			else if (Key_Down == 9 && write_distance_data != 0)
			{
				if (write_distance_data < 10)
				{
					dac_output_data = dac_low_output_10x * 51 / 10;
				}
				else if (write_distance_data > 90)
				{
					dac_output_data = 255;
				}
				else
				{
					temp_V = (5 - dac_low_output_10x / 10.0) * (write_distance_data - 90) / 80 + 5;
					dac_output_data = temp_V * 51;
				}
				Da_Write(dac_output_data); // 在我们没有进行数据记录的时候，输出的一直是0
			}
			break;

		case 1:
			/* 参数界面 */
			if (Key_Down == 8)
			{
				switch (para_show_mode)
				{
				case 0:
					para_distance = (para_distance + 10 == 90) ? 90 : para_distance + 10;
					break;
				case 1:
					para_temperature = (++para_temperature == 90) ? 90 : para_temperature;
					break;
				}
			}
			else if (Key_Down == 9)
			{

				switch (para_show_mode)
				{
				case 0:
					para_distance = (para_distance - 10 == 10) ? 10 : para_distance - 10;
					break;
				case 1:
					para_temperature = (--para_temperature == 0) ? 0 : para_temperature;
					break;
				}
			}
			break;

		case 2:
			/* 工厂模式 */
			if (Key_Down == 8)
			{
				switch (fac_show_mode)
				{
				case 0:
					calibration_value = (calibration_value + 5 == 90) ? 90 : calibration_value + 5;
					break;
				case 1:
					transmission_speed = (transmission_speed + 10 == 9990) ? 9990 : transmission_speed + 10;
					break;
				case 2:
					dac_low_output_10x = (++dac_low_output_10x == 90) ? 20 : dac_low_output_10x;
					break;
				}
			}
			else if (Key_Down == 9)
			{
				switch (fac_show_mode)
				{
				case 0:
					calibration_value = (calibration_value - 5 == -90) ? -90 : calibration_value - 5;
					break;
				case 1:
					transmission_speed = (transmission_speed - 10 == 10) ? 10 : transmission_speed - 10;
					break;
				case 2:
					dac_low_output_10x = (--dac_low_output_10x == 1) ? 1 : dac_low_output_10x;
					break;
				}
			}
			break;
		}
		reset_flag = (Key_Down == 89);
	}
}

/* 信息处理函数 */
void Seg_Proc()
{
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	switch (Seg_Show_mode)
	{
	case 0:
		temperature_data_10x = rd_temperature() * 10;
		Seg_Buf[0] = temperature_data_10x / 100 % 10;
		Seg_Buf[1] = temperature_data_10x / 10 % 10;
		Seg_Buf[2] = temperature_data_10x % 10;
		Seg_Point[1] = 1;
		Seg_Buf[3] = 11; // -
		distance_data = Ut_Wave_Data(calibration_value, transmission_speed);
		/* 测距界面 */
		switch (dis_show_mode)
		{
		case 0:
			/* cm */
			Seg_Buf[4] = (distance_data / 1000 % 10 == 0) ? 10 : distance_data / 1000 % 10;
			Seg_Buf[5] = ((distance_data / 100 % 10 == 0) && (Seg_Buf[4] == 10)) ? 10 : distance_data / 100 % 10;
			Seg_Buf[6] = ((distance_data / 10 % 10 == 0) && (Seg_Buf[5] == 10)) ? 10 : distance_data / 10 % 10;
			Seg_Buf[7] = distance_data % 10;
			break;
		case 1:
			/* m */
			Seg_Buf[4] = (distance_data / 1000 % 10 == 0) ? 10 : distance_data / 1000 % 10;
			Seg_Buf[5] = distance_data / 100 % 10;
			Seg_Buf[6] = distance_data / 10 % 10;
			Seg_Buf[7] = distance_data % 10;
			Seg_Point[5] = 1;
			break;
		}
		break;

	case 1:
		/* 参数界面 */
		Seg_Buf[0] = 12; // P
		Seg_Buf[1] = para_show_mode + 1;
		switch (para_show_mode)
		{
		case 0:
			/* 距离 */
			Seg_Buf[6] = para_distance / 10 % 10;
			Seg_Buf[7] = para_distance % 10;
			break;
		case 1:
			/* 温度 */
			Seg_Buf[6] = para_temperature / 10 % 10;
			Seg_Buf[7] = para_temperature % 10;
			break;
		}
		break;

	case 2:
		/* 工厂模式 */
		Seg_Buf[0] = 13; // F
		Seg_Buf[1] = fac_show_mode + 1;
		switch (para_show_mode)
		{
		case 0:
			/* 校准值 */
			if (calibration_value <= -10)
			{
				Seg_Buf[5] = 11; //-
				Seg_Buf[6] = (-calibration_value) / 10 % 10;
				Seg_Buf[7] = (-calibration_value) % 10;
			}
			else if (calibration_value < 0)
			{
				Seg_Buf[5] = 10;
				Seg_Buf[6] = 11; //-
				Seg_Buf[7] = (-calibration_value) % 10;
			}
			else
			{
				Seg_Buf[5] = 10;
				Seg_Buf[6] = calibration_value / 10 % 10;
				Seg_Buf[7] = calibration_value % 10;
			}
			break;
		case 1:
			/* 介质 */
			Seg_Buf[4] = (transmission_speed / 1000 % 10 == 0) ? 10 : transmission_speed / 1000 % 10;
			Seg_Buf[5] = ((transmission_speed / 100 % 10 == 0) && Seg_Buf[4] == 10) ? 10 : transmission_speed / 100 % 10;
			Seg_Buf[6] = transmission_speed / 10 % 10;
			Seg_Buf[7] = transmission_speed % 10;

			break;
		case 2:
			/* DAC输出 */
			Seg_Buf[6] = dac_low_output_10x / 10 % 10;
			Seg_Buf[7] = dac_low_output_10x % 10;
			Seg_Point[6] = 1;
			break;
		}
		break;
	}
}

/* 其他显示函数 */
void Led_Proc()
{
	uchar i;
	switch (Seg_Show_mode)
	{
	case 0:
		/* 测距 */
		if (distance_data > 255)
		{
			for (i = 0; i < 8; i++)
			{
				ucLed[i] = 1;
			}
		}
		else
		{
			for (i = 0; i < 8; i++)
			{
				ucLed[i] = distance_data & (1 << i);
			}
		}
		break;
	case 1:
		/* 参数 */
		for (i = 0; i < 7; i++)
			ucLed[i] = 0;
		ucLed[7] = 1;
		break;
	case 2:
		/* 工厂 */
		for (i = 1; i < 8; i++)
			ucLed[i] = 0;
		ucLed[0] = led_show_flag;
		break;
	}
	Relay(((para_distance - 5 <= distance_data) &&
		   (distance_data <= para_distance + 5) &&
		   (temperature_data_10x / 10.0 <= para_temperature)));
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
	if (write_distance_data_flag)
	{
		if (++time_6s == 6000)
		{
			time_6s = 0;
			write_distance_data_flag = 0;
		}
	}
	// 开始复位按键计时
	if (reset_flag)
	{
		if (++time_2s >= 2000)
		{
			time_2s = 0;
			reset_data();
		}
	}
	else
		time_2s = 0;
	// 100ms闪烁一下
	if (++time_100ms == 100)
	{
		time_100ms = 0;
		led_show_flag ^= 1;
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

	System_Init();
	Timer0Init();
	reset_data();
	// 如果有温度读取的话
	rd_temperature();
	Delay750ms();
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}