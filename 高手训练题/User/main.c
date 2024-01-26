#include "main.h"
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组
uchar Uart_Slow_Down;								 // 串口减速专用变量
uchar Uart_Recv[10];								 // 串口接收数据储存数组 默认10个字节 若接收数据较长 可更改最大字节数
uchar Uart_Recv_Index;								 // 串口接收数组指针
uchar Uart_Send[10];								 // 串口接收数据储存数组 默认10个字节 若发送数据较长 可更改最大字节数

/* 界面 */
uchar Seg_show_mode; // 0 初始界面 1时钟界面 2信息显示界面 3 时钟设置界面 4闹钟设置界面

/*密码*/
idata uchar Password_set[8] = {1, 2, 3, 4, 5, 6, 7, 8};			  // 初始的默认密码
idata uchar Password_input[8] = {11, 11, 11, 11, 11, 11, 11, 11}; // 密码，最初为没有输入的状态
uchar Password_input_index;										  // 密码输入的指针
bit wring_flag;													  // 密码错误标志
bit ring_flag;													  // 蜂鸣器

/* 时间 */
uchar ucRtc[3] = {0x23, 0x59, 0x55};
uchar set_ucRtc[3]; // 设置时间
uchar old_ucRtc[3]; // 保存上一次的时间
uchar alRtc[3] = {0, 0, 0};
uchar set_alRtc[3];	   // 设置闹钟
uchar old_alRtc[3];	   // 保存上一次的闹钟
uchar set_time_two[2]; // 两位暂存
uchar time_index;
uchar alarm_index;
uint time_1s;
uint time_500ms;
uint time_700ms;
bit skip_flag;				// 跳过的标志
bit time_interval_flag;		// 时间显示中间间隔符闪烁
bit time_set_interval_flag; // 设置界面时间中间间隔闪烁
bit massage_show_flag;		// 0显示时间，1显示信息
bit massage_show_time_flag; // 开启长按时间->信息跳转计时
bit alarm_open;				// 闹钟开启

// LED和Seg初始化
void Init_Seg_Led()
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		Seg_Buf[i] = 0;
		Seg_Point[i] = 0;
		ucLed[i] = 0;
	}
}
/* 键盘处理函数 */
void Key_Proc()
{
	static uchar Key_Val, Key_Down, Key_Old, Key_Up; // 按键专用变量
	uchar i, j;
	if (Key_Slow_Down)
		return;
	Key_Slow_Down = 1; // 键盘减速程序

	Key_Val = Key_Read();					  // 实时读取键码值
	Key_Down = Key_Val & (Key_Old ^ Key_Val); // 捕捉按键下降沿
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // 捕捉按键上降沿
	Key_Old = Key_Val;						  // 辅助扫描变量
	switch (Seg_show_mode)
	{
	case 0: // 登录界面
		if (Password_input_index < 8 && Key_Down)
		{
			// 0 1 2 3 4 5 6 7 8 9
			uchar key_mapping[] = {4, 8, 12, 16, 9, 13, 17, 10, 14, 18};
			for (i = 0; i < 10; i++)
			{
				if (Key_Down == key_mapping[i])
				{
					break;
				}
			}
			// 按下的不是最后一个，那么就是其他的按键，这里不做处理
			if (i == 10 && Key_Down != 18)
			{
			}
			else
			{
				Password_input[Password_input_index] = i;
				Password_input_index++;
			}
		}
		// 按下确认
		if (Key_Down == 7)
		{
			for (i = 0; i < 8; i++)
			{
				if (Password_set[i] != Password_input[i])
				{
					// 报警
					wring_flag = 1;
					ring_flag = 1;
					// 清空输入内容
					for (j = 0; j < 8; j++)
					{
						Password_input[j] = 11;
					}
					break;
				}
			}
			if (i == 8)
			{
				// 密码正确
				wring_flag = 0;
				ring_flag = 0;
				Seg_show_mode++; // 到达下一个界面
			}
		}
		// 按下删除
		if (Key_Down == 6)
		{
			if (Password_input_index == 8)
				Password_input_index = 7;
			Password_input[Password_input_index] = 11;
			if (Password_input_index > 0 && Password_input_index < 7)
				Password_input_index--;
		}
		// 跳过密码
		if (Key_Old == 5)
		{
			skip_flag = 1;
			if (time_1s == 1000)
			{
				Seg_show_mode++; // 跳转到下一个界面
			}
		}
		else
		{
			time_1s = skip_flag = 0; // 重置长按时间
		}
		break;

	case 1:
		/* 主界面 */
		// 信息显示按钮,长按1s进入信息显示界面，松手回到时钟界面
		if (Key_Old == 7)
		{
			massage_show_time_flag = 1;
		}
		if (Key_Up == 7)
		{
			Init_Seg_Led();
			massage_show_time_flag = massage_show_flag = 0;
		}
		// 开启/关闭闹钟
		if (Key_Down == 4)
		{
			alarm_open = ~alarm_open;
		}
		// 进入设置界面
		if (Key_Down == 6)
		{
			Seg_show_mode++;
			Init_Seg_Led();
		}
		break;
	case 2:
		/* 时钟设置界面 */
		switch (Key_Down)
		{
		case 11:
			/* 设置小时 */

			break;

		default:
			break;
		}
		break;
	}
}

/* 信息处理函数 */
void Seg_Proc()
{
	uchar i;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	switch (Seg_show_mode)
	{
	case 0:
		/* 密码输入 */
		if (Password_input_index)
		{
			for (i = 0; i < Password_input_index; i++)
			{
				Seg_Buf[7 - i] = Password_input[Password_input_index - i - 1];
			}
			for (i = Password_input_index; i < 8; i++)
			{
				Seg_Buf[7 - i] = 11;
			}
		}
		else
		{
			// 没有密码输入
			for (i = 0; i < 8; i++)
			{
				Seg_Buf[i] = 11;
			}
		}
		break;
	case 1:
		if (massage_show_flag)
		{
			uchar dis_value;
			uint temperature_value;
			dis_value = Ut_Wave_Data() % 100;
			temperature_value = rd_temperature() * 10;
			Seg_Buf[0] = dis_value / 10;
			Seg_Buf[1] = dis_value % 10;
			Seg_Buf[3] = 11;
			Seg_Buf[4] = temperature_value / 100;
			Seg_Buf[5] = (temperature_value % 100) / 10;
			Seg_Buf[6] = temperature_value % 10;
			Seg_Point[5] = 1;
			Seg_Buf[7] = 12; // C
		}
		else
		{
			Read_Rtc(ucRtc);
			Seg_Buf[0] = ucRtc[0] / 16;
			Seg_Buf[1] = ucRtc[0] % 16;
			Seg_Buf[2] = Seg_Buf[5] = (time_interval_flag) ? 11 : 10;
			Seg_Buf[3] = ucRtc[1] / 16;
			Seg_Buf[4] = ucRtc[1] % 16;
			Seg_Buf[6] = ucRtc[2] / 16;
			Seg_Buf[7] = ucRtc[2] % 16;
		}
		break;
	case 2: // 时钟/闹钟设置
		uchar temp_time[3];

		break;
	}
}

/* 其他显示函数 */
void Led_Proc()
{
	Beep(ring_flag);
}

/* 串口处理函数 */
void Uart_Proc()
{
	if (Uart_Slow_Down)
		return;
	Uart_Slow_Down = 1; // 串口减速程序
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
	if (++Uart_Slow_Down == 200)
		Uart_Slow_Down = 0; // 串口减速专用
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // 数码管显示专用
	// 触发跳过
	if (skip_flag)
	{
		// 长按1s
		time_1s = (++time_1s < 1000) ? time_1s : 1000;
	}
	if (++time_500ms == 500)
	{
		time_500ms = 0;
		time_interval_flag = ~time_interval_flag;
	}
	if (++time_700ms == 700)
	{
		time_700ms = 0;
		time_set_interval_flag = ~time_set_interval_flag;
	}
	if (massage_show_time_flag)
	{
		time_1s = (++time_1s < 1000) ? time_1s : 1000;
		if (time_1s == 1000)
		{
			massage_show_flag = 1;
		}
	}
	Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

/* 串口1中断服务函数 */
void Uart1Server() interrupt 4
{
	if (RI == 1) // 串口接收数据
	{
		Uart_Recv[Uart_Recv_Index] = SBUF;
		Uart_Recv_Index++;
		RI = 0;
	}
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
	Set_Rtc(ucRtc); // 设置初始时间
	System_Init();
	Timer0Init();
	UartInit();
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
		Uart_Proc();
	}
}