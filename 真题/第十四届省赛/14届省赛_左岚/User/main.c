#include "main.h"
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组

// 时间
uchar ucRtc[3] = {0x12, 0x12, 0x12}; // 初始时间
uchar old_Rtc[3];					 // 上一次记录的时间

// 显示
uchar Seg_show_mode; // 0时间，1回显，3参数
uchar re_show_mode;	 // 0温度回显，1湿度回显，2时间回显
bit reset_flag;		 // 复位标志位
// 计时与测频
uchar time_100ms;
uint time_1s;
uint time_2s;
uint time_3s;
uint freq;

// 采集触发
bit collect_flag; // 采集触发标志位
uchar old_light;  // 上一次的温度，用作对比，判断是否是光->暗
// 测量温湿度
float aver_temperature;
uchar max_temperature, old_temperature;
float aver_humidity;
uchar max_humidity, old_humidity;
uchar measure_count;	// 测量次数
uchar temperature_demo; // 温度参数
uchar humidity_error;
bit wring_flag; // 温度大于温度参数
bit led_flag;	// 闪烁
bit up_flag;	// 这次比上一次采集的数据高

void init_Seg_LED()
{
	uchar i = 0;
	for (i = 0; i < 8; i++)
	{
		ucLed[i] = 0;
		Seg_Buf[i] = 10;
		Seg_Point[i] = 0;
	}
}
void reset_system()
{
	Seg_show_mode = 0;
	re_show_mode = 0;
	temperature_demo = 30;
	measure_count = 0;
}
uchar rd_humidity()
{
	if (freq < 10 || freq > 2000)
		return 0;
	else
		return ((freq - 200) * 2 / 45 + 10);
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
		Seg_show_mode = (++Seg_show_mode) % 3;
		re_show_mode = 0; // 保证每次都是切换到温度回显
		init_Seg_LED();
	}
	if (Seg_show_mode == 1 && Key_Down == 5)
	{
		re_show_mode = (++re_show_mode) % 3;
		init_Seg_LED();
	}
	if (Seg_show_mode == 2 && Key_Down == 8)
		temperature_demo = (++temperature_demo >= 99) ? 99 : temperature_demo;
	if (Seg_show_mode == 2 && Key_Down == 9)
		temperature_demo = (--temperature_demo == 0) ? 0 : temperature_demo;
	if (Seg_show_mode == 1 && re_show_mode == 2)
	{
		if (Key_Down == 9)
		{
			reset_flag = 1;
		}
		if (Key_Up == 9 && time_2s >= 2000)
		{
			reset_flag = 0;
			reset_system();
		}
	}
}

/* 信息处理函数 */
void Seg_Proc()
{
	uchar temp_temperature, temp_humidity, light;

	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	light = Ad_Read(0x41);
	temp_temperature = rd_temperature();
	temp_humidity = rd_humidity();
	// 上一次是光，这一次是暗，并且还没有处于采集的时候
	if (light < 10 && old_light > 40 && (time_3s == 0))
	{
		init_Seg_LED();
		collect_flag = 1;
		measure_count = (++measure_count) % 100;
		// 温湿度界面
		Read_Rtc(old_Rtc);
	}
	old_light = light;
	if (collect_flag)
	{
		// 温湿度界面
		if (measure_count >= 2 && (old_temperature < temp_temperature) && (old_humidity < temp_humidity))
			up_flag = 1;
		else
			up_flag = 0;
		old_temperature = temp_temperature;
		old_humidity = temp_humidity;
		if (temp_temperature > temperature_demo)
			wring_flag = 1;
		else
			wring_flag = 0;
		if (temp_temperature > max_temperature)
			max_temperature = temp_temperature;
		if (temp_humidity > max_humidity)
			max_humidity = temp_humidity;
		if (temp_humidity == 0)
		{
			++humidity_error; // 无效次数+1
			ucLed[4] = 1;	  // 点亮LED5
		}
		else
		{
			ucLed[4] = 0; // 熄灭LED5
		}
		aver_temperature = (aver_temperature * (measure_count - 1) + temp_temperature) / measure_count;
		aver_humidity = (aver_humidity * (measure_count - humidity_error - 1) + temp_humidity) / (measure_count - humidity_error);
		Seg_Buf[0] = 16; // E
		Seg_Buf[1] = measure_count / 10;
		Seg_Buf[2] = measure_count % 10;
		Seg_Buf[3] = temp_temperature / 10;
		Seg_Buf[4] = temp_temperature % 10;
		Seg_Buf[5] = 11; //-
		// 湿度非法显示AA
		Seg_Buf[6] = (temp_humidity == 0) ? 17 : temp_humidity / 10;
		Seg_Buf[7] = (temp_humidity == 0) ? 17 : temp_humidity % 10;
	}
	else
	{
		switch (Seg_show_mode)
		{
		case 0:
			// 时间界面
			Read_Rtc(ucRtc);
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
			// 回显界面
			switch (re_show_mode)
			{
			case 0:
				// 温度回显
				Seg_Buf[0] = 12; // C
				Seg_Buf[2] = (measure_count > 0) ? (max_temperature / 10) : 10;
				Seg_Buf[3] = (measure_count > 0) ? (max_temperature % 10) : 10;
				Seg_Buf[4] = 11; //-
				Seg_Buf[5] = (measure_count > 0) ? ((uchar)(aver_temperature * 10) / 100) : 10;
				Seg_Buf[6] = (measure_count > 0) ? ((uchar)(aver_temperature * 10) % 100 / 10) : 10;
				Seg_Buf[7] = (measure_count > 0) ? ((uchar)(aver_temperature * 10) % 10) : 10;
				Seg_Point[6] = 1;
				break;
			case 1:
				// 湿度回显
				Seg_Buf[0] = 13; // H
				Seg_Buf[2] = (measure_count > 0) ? (max_humidity / 10) : 10;
				Seg_Buf[3] = (measure_count > 0) ? (max_humidity % 10) : 10;
				Seg_Buf[4] = 11; //-
				Seg_Buf[5] = (measure_count > 0) ? ((uchar)(aver_humidity * 10) / 100) : 10;
				Seg_Buf[6] = (measure_count > 0) ? ((uchar)(aver_humidity * 10) % 100 / 10) : 10;
				Seg_Buf[7] = (measure_count > 0) ? ((uchar)(aver_humidity * 10) % 10) : 10;
				Seg_Point[6] = 1;
				break;
			case 2:
				// 时间回显
				Seg_Buf[0] = 14; // F
				Seg_Buf[1] = measure_count / 10;
				Seg_Buf[2] = measure_count % 10;
				Seg_Buf[3] = (measure_count > 0) ? (old_Rtc[0] / 16) : 10;
				Seg_Buf[4] = (measure_count > 0) ? (old_Rtc[0] % 16) : 10;
				Seg_Buf[5] = (measure_count > 0) ? 11 : 10; //-
				Seg_Buf[6] = (measure_count > 0) ? (old_Rtc[1] / 16) : 10;
				Seg_Buf[7] = (measure_count > 0) ? (old_Rtc[1] % 16) : 10;
				break;
			}
			break;
		case 2:
			// 参数界面
			Seg_Buf[0] = 15; // P
			Seg_Buf[6] = temperature_demo / 10;
			Seg_Buf[7] = temperature_demo % 10;
			break;
		}
	}
}

/* 其他显示函数 */
void Led_Proc()
{
	switch (Seg_show_mode)
	{
	case 0:
		// 时间界面
		ucLed[0] = 1;
		break;
	case 1:
		ucLed[1] = 1;
		break;
	}
	if (collect_flag)
		ucLed[2] = 1;
	ucLed[3] = led_flag;
	ucLed[5] = up_flag;
}

void Timer0_Init(void) // 0微秒@12.000MHz
{
	AUXR &= 0x7F; // 定时器时钟12T模式
	TMOD &= 0xF0; // 设置定时器模式
	TMOD |= 0x05; // 手动设置初值
	TL0 = 0x00;	  // 设置定时初值
	TH0 = 0x00;	  // 设置定时初值
	TF0 = 0;	  // 清除TF0标志
	TR0 = 1;	  // 定时器0开始计时
	ET0 = 1;	  // 定时器0中断打开
	EA = 1;		  // 总中断打开
}

/* 定时器1中断初始化函数 */
void Timer1_Init(void) // 1毫秒@12.000MHz
{
	AUXR &= 0xBF; // 定时器时钟12T模式
	TMOD &= 0x0F; // 设置定时器模式
	TL1 = 0x18;	  // 设置定时初始值
	TH1 = 0xFC;	  // 设置定时初始值
	TF1 = 0;	  // 清除TF1标志
	TR1 = 1;	  // 定时器1开始计时
	ET1 = 1;	  // 定时器1中断打开
	EA = 1;		  // 总中断打开
}
/* 定时器1中断服务函数 */
void Timer1Server() interrupt 3
{
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // 键盘减速专用
	if (++Seg_Slow_Down == 500)
		Seg_Slow_Down = 0; // 数码管减速专用
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // 数码管显示专用
	if (++time_1s == 1000)
	{
		time_1s = 0;
		freq = TH0 << 8 | TL0; // 取出频率
		TH0 = 0;			   // 归零
		TL0 = 0;			   // 归零
	}
	if (collect_flag)
	{
		if (++time_3s == 3000)
		{
			collect_flag = 0;
			time_3s = 0;
		}
	}
	else
	{
		time_3s = 0;
	}
	if (reset_flag)
	{
		if (++time_2s >= 2000)
		{
			time_2s = 2001;
		}
	}
	if (wring_flag)
	{
		if (++time_100ms == 100)
		{
			time_100ms = 0;
			led_flag ^= 1;
		}
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
	reset_system();
	System_Init();
	Timer0_Init();
	Timer1_Init();
	Set_Rtc(ucRtc); // 写入初始时钟
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}