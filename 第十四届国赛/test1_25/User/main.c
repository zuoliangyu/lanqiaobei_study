#include "main.h"
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组

/*时间方面*/
uint time_6s;		 // 6s计时
uint time_2s;		 // 2s计时
uchar time_100ms;	 // 100ms计时
bit reset_flag;		 // 复位标志位
bit write_data_flag; // 写入数据标志位
bit led_light_flag;	 // 闪烁

/*显示模式与切换*/
uchar Seg_show_mode;   // 0-测距;1-参数;2-工厂
bit test1_show_mode;   // 0-cm;1-m
bit test2_show_mode;   // 0-距离参数;1-温度参数
uchar test3_show_mode; // 0-校准值 1-传输速度 2-DAC下限

/*测距相关*/
uint dis_value; // 距离值cm

/*温度模块*/
float temperature_value; // 温度值
/*参数界面*/
uchar dis_demo;
uchar temperature_demo;

/*工厂模式*/
char cali_value;	 // 校准值
uint speed_value;	 // 传输速度
float dac_low_value; // DAC下限

/*数据记录与输出*/
float memory_data;
uchar dac_out;

void init_sys_value()
{
	Seg_show_mode = 0;
	test1_show_mode = 0;
	test2_show_mode = 0;
	test3_show_mode = 0;
	dis_demo = 40;
	temperature_demo = 30;
	cali_value = 0;
	speed_value = 340;
	dac_low_value = 1.0;
}
void init_Seg_LED() // 将数码管和LED清空，避免忘记修改
{
	uchar i; // 循环变量
	for (i = 0; i < 8; i++)
	{
		Seg_Buf[i] = 10;  // 数码管显示内容初始化
		Seg_Point[i] = 0; // 数码管小数点初始化
		ucLed[i] = 0;	  // LED显示内容初始化
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

	// 当我们没有进行记录
	if (!write_data_flag)
	{
		switch (Key_Down)
		{
		case 4:
			init_Seg_LED();
			Seg_show_mode = (++Seg_show_mode < 3) ? Seg_show_mode : 0;
			test1_show_mode = test2_show_mode = test3_show_mode = 0;
			break;
		case 5:
			init_Seg_LED();
			switch (Seg_show_mode)
			{
			case 0:
				test1_show_mode = ~test1_show_mode;
				break;
			case 1:
				test2_show_mode = ~test2_show_mode;
				break;
			case 2:
				test3_show_mode = (++test3_show_mode < 3) ? test3_show_mode : 0;
				break;
			}
			break;
		case 8:
			switch (Seg_show_mode)
			{
			case 0:
				write_data_flag = 1;
				break;
			case 1:
				if (test2_show_mode)
					temperature_demo = (++temperature_demo < 80) ? temperature_demo : 80;
				else
					dis_demo = (dis_demo + 10 < 90) ? dis_demo + 10 : 90;
				break;
			case 2:
				switch (test3_show_mode)
				{
				case 0:
					cali_value = (cali_value + 5 < 90) ? cali_value + 5 : 90;
					break;
				case 1:
					speed_value = (speed_value + 10 < 9990) ? speed_value + 10 : 9990;
					break;
				case 2:
					dac_low_value = (dac_low_value + 0.1 < 2.0) ? dac_low_value + 0.1 : 2.0;
					break;
				}
				break;
			}
			break;
		case 9:
			switch (Seg_show_mode)
			{
			case 0:
				// 开始输出数据
				if (memory_data > 90)
				{
					dac_out = 5;
				}
				else if (memory_data == 0)
				{
					dac_out = 0;
				}
				else
				{
					dac_out = (5 - dac_low_value) / 80 * (memory_data - 10) + dac_low_value;
				}
				Da_Write(dac_out);
				break;
			case 1:
				if (test2_show_mode)
					temperature_demo = (--temperature_demo > 0) ? temperature_demo : 0;
				else
					dis_demo = (dis_demo - 10 > 10) ? dis_demo - 10 : 10;
				break;
			case 2:
				switch (test3_show_mode)
				{
				case 0:
					cali_value = (cali_value - 5 > -90) ? cali_value - 5 : -90;
					break;
				case 1:
					speed_value = (speed_value - 10 > 10) ? speed_value - 10 : 10;
					break;
				case 2:
					dac_low_value = (dac_low_value - 0.1 > 0.1) ? dac_low_value - 0.1 : 0.1;
					break;
				}
				break;
			}
			break;
		}
		// 如果同时按下89
		if (Key_Old == 89)
		{
			reset_flag = 1;
		}
		else
		{
			reset_flag = 0;
		}
	}
}

/* 信息处理函数 */
void Seg_Proc()
{
	uint int_temperature_value; // 温度值整型变量
	uchar i;
	uchar temp_DAC_low;
	int temp_cali;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	switch (Seg_show_mode)
	{
	case 0:
		/* 测距界面 */
		temperature_value = rd_temperature();
		int_temperature_value = temperature_value * 10;
		Seg_Buf[0] = int_temperature_value / 100;
		Seg_Buf[1] = int_temperature_value % 100 / 10;
		Seg_Buf[2] = int_temperature_value % 10;
		Seg_Point[1] = 1; // 小数点
		Seg_Buf[3] = 13;  //-
		dis_value = (uint)Ut_Wave_Data(speed_value, cali_value);
		// 显示单位为m
		if (test1_show_mode)
		{
			Seg_Buf[4] = (dis_value / 1000) ? dis_value / 1000 : 10;
			Seg_Buf[5] = dis_value / 100 % 10;
			Seg_Buf[6] = dis_value / 10 % 10;
			Seg_Buf[7] = dis_value % 10;
			Seg_Point[5] = 1;
		}
		// 显示单位为cm
		else
		{
			Seg_Buf[4] = dis_value / 1000;
			Seg_Buf[5] = dis_value / 100 % 10;
			Seg_Buf[6] = dis_value / 10 % 10;
			Seg_Buf[7] = dis_value % 10;
			i = 4;
			while (Seg_Buf[i] == 0)
			{
				if (i == 7)
					break;
				Seg_Buf[i] = 10;
				i++;
			}
		}
		break;
	case 1:
		/*参数界面*/
		Seg_Buf[0] = 11; // P
		Seg_Buf[1] = (uchar)test2_show_mode + 1;
		Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10; // 灭
		Seg_Buf[6] = test2_show_mode ? temperature_demo / 10 : dis_demo / 10;
		Seg_Buf[7] = test2_show_mode ? temperature_demo % 10 : dis_demo % 10;
		break;
	case 2:
		/*工厂界面*/
		Seg_Buf[0] = 12; // F
		Seg_Buf[1] = test3_show_mode + 1;
		Seg_Buf[2] = Seg_Buf[3] = 10; // 灭
		switch (test3_show_mode)
		{
		case 0:
			Seg_Buf[4] = 10;
			/* 显示校准值 */
			temp_cali = abs((int)cali_value);
			Seg_Buf[5] = temp_cali / 100;
			Seg_Buf[6] = temp_cali / 10 % 10;
			Seg_Buf[7] = temp_cali % 10;
			i = 5;
			while (Seg_Buf[i] == 0)
			{
				if (i == 7)
					break;
				Seg_Buf[i] = 10;
				i++;
			}
			if (cali_value < 0)
			{
				Seg_Buf[i - 1] = 13; //-
			}
			break;
		case 1:
			/* 显示传输速度 */
			Seg_Buf[4] = speed_value / 1000;
			Seg_Buf[5] = speed_value / 100 % 10;
			Seg_Buf[6] = speed_value / 10 % 10;
			Seg_Buf[7] = speed_value % 10;
			i = 4;
			while (Seg_Buf[i] == 0)
			{
				if (i == 7)
					break;
				Seg_Buf[i] = 10;
				i++;
			}
			break;
		case 2:

			temp_DAC_low = dac_low_value * 10;
			/* 显示DAC下限 */
			Seg_Buf[4] = Seg_Buf[5] = 10;
			Seg_Buf[6] = temp_DAC_low / 10;
			Seg_Buf[7] = temp_DAC_low % 10;
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
	switch (Seg_show_mode)
	{
	case 0:
		for (i = 0; i < 8; i++)
		{
			ucLed[i] = dis_value & (0x01 << i);
		}
		if (dis_value >= 255)
		{
			ucLed[0] = ucLed[1] = ucLed[2] = ucLed[3] = ucLed[4] = ucLed[5] = ucLed[6] = ucLed[7] = 1;
		}
		break;
	case 1:
		ucLed[7] = 1;
		break;
	case 2:
		ucLed[0] = led_light_flag;
		break;
	}
	if ((dis_demo + 5 >= dis_value) && (temperature_demo >= temperature_value))
	{
		Relay(1);
	}
	else
	{
		Relay(0);
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
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // 数码管显示专用
	if (reset_flag)
	{
		if (++time_2s >= 2000)
		{
			time_2s = 0;
			init_sys_value();
		}
	}
	if (write_data_flag)
	{
		memory_data = dis_value;
		if (++time_6s >= 6000)
		{
			time_6s = write_data_flag = 0;
		}
	}
	if (++time_100ms >= 100)
	{
		led_light_flag = ~led_light_flag;
		time_100ms = 0;
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
	init_sys_value(); // 初始化所有数据
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