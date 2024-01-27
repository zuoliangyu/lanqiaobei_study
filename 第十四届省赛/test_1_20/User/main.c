/* 头文件声明区 */
#include <STC15F2K60S2.H> //单片机寄存器专用头文件
#include <Init.h>		  //初始化底层驱动专用头文件
#include <Led.h>		  //Led底层驱动专用头文件
#include <Key.h>		  //按键底层驱动专用头文件
#include <Seg.h>		  //数码管底层驱动专用头文件
#include <stdio.h>		  //标准库底层驱动专用头文件
#include <iic.h>		  //光敏电阻头文件
#include <ds1302.h>		  //ds1302头文件
#include <onewire.h>	  //温度传感器头文件
/* 变量声明区 */
unsigned char Key_Val, Key_Down, Key_Old, Key_Up;			 // 按键专用变量
unsigned char Key_Slow_Down;								 // 按键减速专用变量
unsigned char Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
unsigned char Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
unsigned char Seg_Pos;										 // 数码管扫描专用变量
unsigned int Seg_Slow_Down;									 // 数码管减速专用变量
unsigned char ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组

/*时间模块*/
unsigned char time_100ms;					 // 100ms计时
unsigned int time_1s;						 // 1s计时
unsigned int time_2s;						 // 2s计时
unsigned int time_3s;						 // 3s计时
unsigned char ucRtc[3] = {0x15, 0x55, 0x55}; // 初始时间为11:11:11

/*数码管显示模块*/
unsigned char Seg_show_mode = 0;	 // 正常界面显示，0时间，1温度回显，2参数，100温湿度
unsigned char old_Seg_show_mode = 0; // 上一次的界面显示
unsigned char re_Seg_show_mode = 0;	 // 回显界面显示，0温度回显，1湿度回显，2时间回显

/*温度处理模块*/
idata float aver_temperature = 0;		 // 平均温度
idata unsigned char max_temperature = 0; // 最高温度（这里因为我们只需要显示整数）
idata float old_tempture = 0;			 // 旧温度
bit up_tempture_flag = 0;				 // 温度是否上升

/*湿度处理模块*/
idata float aver_humidity = 0;			   // 平均湿度
idata unsigned char max_humidity = 0;	   // 最高湿度（这里因为我们只需要显示整数）
idata unsigned int count_f = 0;			   // 溢出计数
idata unsigned int data_f = 0;			   // 1s的频率
bit humidity_flag = 1;					   // 是否有效记录
idata unsigned char demo_temperature = 30; // 温度参考
idata float old_humidity = 0;			   // 旧湿度
bit up_humidity_flag = 0;				   // 湿度是否上升

/*触发和光照模块*/
unsigned char trigger_number = 0;		   // 触发次数
unsigned char old_light = 0;			   // 旧的光照值
unsigned char trigger_time[3] = {0, 0, 0}; // 最后一次采集时间
bit light_down_flag = 0;				   // 光照是否变暗

/* 键盘处理函数 */
void Key_Proc()
{
	if (Key_Slow_Down)
		return;
	Key_Slow_Down = 1; // 键盘减速程序

	Key_Val = Key_Read();					  // 实时读取键码值
	Key_Down = Key_Val & (Key_Old ^ Key_Val); // 捕捉按键下降沿
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // 捕捉按键上降沿
	Key_Old = Key_Val;						  // 辅助扫描变量
	// 按下界面切换的按键
	if (Key_Down == 4)
	{
		Seg_show_mode++;
		if (Seg_show_mode > 2)
		{
			Seg_show_mode = 0; // 进行循环
		}
	}
	// 按下回显界面切换的按键，并且处于回显界面
	if (Key_Down == 5 && Seg_show_mode == 1)
	{
		re_Seg_show_mode++;
		if (re_Seg_show_mode > 2)
		{
			re_Seg_show_mode = 0; // 进行循环
		}
	}
	if (Key_Down == 8 && Seg_show_mode == 2)
	{
		if (++demo_temperature > 100)
		{
			demo_temperature = 0;
		}
	}
	if (Key_Down == 9 && Seg_show_mode == 2)
	{
		if (--demo_temperature < 0)
		{
			demo_temperature = 99;
		}
	}
	if (Key_Down == 9 && Seg_show_mode == 1 && re_Seg_show_mode == 2)
	{
		if (++time_2s == 200)
		{
			// 现在长按了2s，清空全部的数据
			trigger_number = 0;
			aver_temperature = 0;
			max_temperature = 0;
			old_tempture = 0;

			aver_humidity = 0;
			max_humidity = 0;
			old_humidity = 0;
			trigger_time[0] = 0;
			trigger_time[1] = 0;
			trigger_time[2] = 0;
		}
	}
	else
	{
		time_2s = 0;
	}
}

/* 信息处理函数 */
void Seg_Proc()
{
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	if (Seg_show_mode == 0)
	{
		re_Seg_show_mode = 0;
		// 显示时间
		Read_Rtc(ucRtc);
		Seg_Buf[0] = ucRtc[0] / 16;
		Seg_Buf[1] = ucRtc[0] % 16;
		Seg_Buf[3] = ucRtc[1] / 16;
		Seg_Buf[4] = ucRtc[1] % 16;
		Seg_Buf[6] = ucRtc[2] / 16;
		Seg_Buf[7] = ucRtc[2] % 16;
		Seg_Buf[2] = Seg_Buf[5] = 16; //-
		Seg_Point[6] = 0;
	}
	else if (Seg_show_mode == 1)
	{
		// 进入回显界面，且默认为温度
		switch (re_Seg_show_mode)
		{
		case 0:				 // 温度回显
			Seg_Buf[0] = 11; // C
			if (trigger_number == 0)
			{
				Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 10; // 灭
				Seg_Point[6] = 0;
			}
			else
			{
				unsigned int temp;
				temp = aver_temperature * 10;
				Seg_Buf[1] = 10; // 灭
				Seg_Buf[2] = max_temperature / 10;
				Seg_Buf[3] = max_temperature % 10;
				Seg_Buf[4] = 16; //-
				Seg_Buf[5] = temp / 100;
				Seg_Buf[6] = (temp % 100) / 10;
				Seg_Buf[7] = temp % 10;
				Seg_Point[6] = 1;
			}
			break;
		case 1:				 // 湿度回显
			Seg_Buf[0] = 12; // H
			if (trigger_number == 0)
			{
				Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 10; // 灭
				Seg_Point[6] = 0;
			}
			else
			{
				unsigned int temp;
				temp = aver_humidity * 10;
				Seg_Buf[1] = 10; // 灭
				Seg_Buf[2] = max_humidity / 10;
				Seg_Buf[3] = max_humidity % 10;
				Seg_Buf[4] = 16; //-
				Seg_Buf[5] = temp / 100;
				Seg_Buf[6] = (temp % 100) / 10;
				Seg_Buf[7] = temp % 10;
				Seg_Point[6] = 1;
			}
			break;
		case 2:				 // 时间回显
			Seg_Buf[0] = 13; // F
			if (trigger_number == 0)
			{
				Seg_Buf[1] = Seg_Buf[2] = 0;
				Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 16; // 灭
				Seg_Point[6] = 0;
			}
			else
			{
				Seg_Buf[1] = trigger_number / 10;
				Seg_Buf[2] = trigger_number % 10;
				Seg_Buf[3] = trigger_time[0] / 16;
				Seg_Buf[4] = trigger_time[0] % 16;
				Seg_Buf[5] = 16; //-
				Seg_Buf[6] = trigger_time[1] / 16;
				Seg_Buf[7] = trigger_time[1] % 16;
				Seg_Point[6] = 0;
			}
			break;
		}
	}
	else if (Seg_show_mode == 2)
	{
		// 显示参数
		Seg_Buf[0] = 14; // P

		Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10; // 灭
		Seg_Buf[6] = demo_temperature / 10;
		Seg_Buf[7] = demo_temperature % 10;
		Seg_Point[6] = 0;
		re_Seg_show_mode = 0;
	}
	else if (Seg_show_mode == 100)
	{
		unsigned char temp_temperature;
		unsigned char temp_humidity;
		// 显示温湿度界面
		Seg_Buf[0] = 15; // E
		temp_temperature = old_tempture;
		temp_humidity = old_humidity;
		Seg_Buf[1] = Seg_Buf[2] = 10; // 灭
		Seg_Buf[3] = temp_temperature / 10;
		Seg_Buf[4] = temp_temperature % 10;
		Seg_Buf[5] = 16; //-
		Seg_Buf[6] = temp_humidity / 10;
		Seg_Buf[7] = temp_humidity % 10;
		Seg_Point[6] = 0;
	}
}

/* 其他显示函数 */
void Led_Proc()
{
	if (Seg_show_mode == 0)
	{
		ucLed[0] = 1;
	}
	else
	{
		ucLed[0] = 0;
	}
	if (Seg_show_mode == 1)
	{
		ucLed[1] = 1;
	}
	else
	{
		ucLed[1] = 0;
	}
	if (Seg_show_mode == 100)
	{
		ucLed[2] = 1;
	}
	else
	{
		ucLed[2] = 0;
	}
	// 采集湿度无效
	if (!humidity_flag)
	{
		ucLed[4] = 1;
	}
	else
	{
		ucLed[4] = 0;
	}
	if (up_humidity_flag && up_tempture_flag && (trigger_number > 2))
	{
		ucLed[5] = 1;
	}
	else
	{
		ucLed[5] = 0;
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
/*计数器1终端初始化函数*/
void Timer1Init(void) // 1微秒@12.000MHz
{
	AUXR &= 0xBF; // 定时器时钟12T模式
	TMOD &= 0x0F; // 设置定时器模式
	TMOD |= 0x20; // 设置定时器模式
	TL1 = 0xFF;	  // 设置定时初值
	TH1 = 0xFF;	  // 设置定时重载值
	TF1 = 0;	  // 清除TF1标志
	TR1 = 1;	  // 定时器1开始计时
	ET1 = 1;	  // 定时器1中断打开
	EA = 1;		  // 总中断打开
}
/* 计数器1中断服务函数 */
void Timer1Server() interrupt 3
{
	count_f++;
}
/* 定时器0中断服务函数 */
void Timer0Server() interrupt 1
{
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // 键盘减速专用
	if (++Seg_Slow_Down == 200)
		Seg_Slow_Down = 0; // 数码管减速专用
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // 数码管显示专用
	// 1s的时候，我们将溢出的计数进行取出
	if (++time_1s == 1000)
	{
		data_f = count_f;
		count_f = 0;
	}
	// 当我们触发了光暗的时候
	if (light_down_flag)
	{
		// 采集3s
		if (++time_3s == 3000)
		{
			light_down_flag = 0;
			Seg_show_mode = old_Seg_show_mode; // 回到原本的显示
		}
		// 0.1s取反
		if (++time_100ms == 100)
		{
			ucLed[3] = ~ucLed[3];
		}
	}

	Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}
void get_temperature()
{
	float temp_temperature;
	temp_temperature = rd_temperature();
	if (trigger_number > 2 && temp_temperature > old_tempture)
	{
		up_tempture_flag = 1;
	}
	else
	{
		up_tempture_flag = 0;
	}
	if (max_temperature < temp_temperature)
	{
		max_temperature = temp_temperature;
	}
	aver_temperature = (aver_temperature * (trigger_number - 1) + temp_temperature) / trigger_number;
	old_tempture = temp_temperature;
}
void get_humidity()
{
	// 这个时候认为是无效记录
	if (data_f < 200 || data_f > 2000)
	{
		humidity_flag = 0;
	}
	else
	{
		float temp_humidity;
		// 采集有效
		humidity_flag = 1;
		temp_humidity = (data_f - 200) * 2 / 45 + 10;
		if (trigger_number > 2 && temp_humidity > old_humidity)
		{
			// 湿度升高
			up_humidity_flag = 1;
		}
		else
		{
			up_humidity_flag = 0;
		}
		if (max_humidity < temp_humidity)
		{
			max_humidity = temp_humidity;
		}
		aver_humidity = (aver_humidity * (trigger_number - 1) + temp_humidity) / trigger_number;
		old_humidity = temp_humidity;
	}
}
void get_light()
{
	unsigned char temp_light;
	temp_light = Ad_Read(0x01);
	// 这里自行考虑变暗的阈值
	// 当从亮变暗，并且之前没有进行变化的时候
	if (old_light > 120 && temp_light < 80 && !light_down_flag)
	{
		light_down_flag = 1;
		trigger_number++;
		get_temperature();
		get_humidity();
		Read_Rtc(trigger_time);			   // 记录一下采集的时间
		old_Seg_show_mode = Seg_show_mode; // 记录一下跳转前的状态
		Seg_show_mode = 100;			   // 跳转到温湿度显示界面
	}
	old_light = temp_light;
}
/* Main */
void main()
{
	System_Init();
	Timer0Init();
	Timer1Init();
	Set_Rtc(ucRtc);
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
		get_light();
	}
}