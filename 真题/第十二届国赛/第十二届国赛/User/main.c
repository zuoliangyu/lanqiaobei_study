#include "main.h"
#define LIGHT_NUM 50
#define DARK_NUM 10
/* 变量声明区 */
uchar Key_Slow_Down;								 // 按键减速专用变量
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据存放数组
uchar Seg_Pos;										 // 数码管扫描专用变量
uint Seg_Slow_Down;									 // 数码管减速专用变量
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // Led显示数据存放数组
uchar ucRtc[3] = {0x11, 0x11, 0x11};
/*界面*/
bit Seg_show_mode;		// 0数据 1参数
uchar data_show_mode;	// 0时间 1距离 2记录
bit celi_show_mode;		// 0时间参数 1距离参数
bit burst_mode;			// 0触发模式 1定时模式
uchar data_show_memory; // 0最大值 1最小值 2平均值
/*数据*/
uchar collection_time = 2;
uchar collection_time_arr[5] = {2, 3, 5, 7, 9};
uchar collection_time_index;
uchar dis_demo = 10;
uint dis_value;
uint dis_max;
uint dis_min;
uint collection_count;
uchar wring_count;
bit wring_flag;
float dis_aver;

uchar light_value, light_value_old;
void init_Seg()
{
	uchar i;
	for (i = 0; i < 8; i++)
	{
		Seg_Buf[i] = 10;
		Seg_Point[i] = 0;
	}
}
void hide_high_Seg(uchar start_num, uchar end_num)
{
	uchar i;
	for (i = start_num; i < end_num; i++)
	{
		if (Seg_Buf[i] != 0)
			break;
		else
			Seg_Buf[i] = 10;
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
		Seg_show_mode ^= 1;
		celi_show_mode = data_show_mode = data_show_memory = 0;
		init_Seg();
	}
	if (Key_Down == 5)
	{
		// 参数界面
		if (Seg_show_mode)
		{
			celi_show_mode ^= 1;
			init_Seg();
		}
		// 数据界面
		else
		{
			data_show_mode = (++data_show_mode) % 3;
			init_Seg();
		}
	}
	if (Key_Down == 8 && Seg_show_mode == 0)
	{
		if (data_show_mode == 1)
			burst_mode ^= 1;
		else if (data_show_mode == 2)
		{
			data_show_memory = (++data_show_memory) % 3;
			init_Seg();
		}
	}
	if (Key_Down == 9 && Seg_show_mode == 1)
	{
		if (celi_show_mode)
			dis_demo = (++dis_demo > 80) ? 10 : dis_demo;
		else
		{
			collection_time_index = (++collection_time_index) % 5;
			collection_time = collection_time_arr[collection_time_index];
		}
	}
}

/* 信息处理函数 */
void Seg_Proc()
{
	uchar temp_dis;
	if (Seg_Slow_Down)
		return;
	Seg_Slow_Down = 1; // 数码管减速程序
	Read_Rtc(ucRtc);
	light_value = Ad_Read(0x01);
	// 定时模式
	if (burst_mode)
	{
		if ((ucRtc[2] / 16 * 10 + ucRtc[2] % 16) % collection_time == 0)
		{
			dis_value = Ut_Wave_Data();
			collection_count = (++collection_count) % 65535;
			dis_aver = (dis_aver * (collection_count - 1) + dis_value) / collection_count;
			if (dis_value > dis_demo - 5 || dis_value < dis_demo + 5)
			{
				if (++wring_count > 3)
				{
					wring_count = 4;
				}
			}
			else
				wring_count = 0;
		}
	}
	// 触发模式
	else
	{
		// 光->暗
		if (light_value_old > LIGHT_NUM && light_value < DARK_NUM)
		{
			dis_value = Ut_Wave_Data();
			collection_count = (++collection_count) % 65535;
			dis_aver = (dis_aver * (collection_count - 1) + dis_value) / collection_count;
			if (dis_value > dis_demo - 5 || dis_value < dis_demo + 5)
				wring_count = (++wring_count) % 4;
			else
				wring_count = 0;
		}
	}
	if (dis_max < dis_value)
		dis_max = dis_value;
	if (dis_min > dis_value || dis_min == 0)
		dis_min = dis_value;
	if (dis_value > 80)
		Da_Write(255);
	else if (dis_value < 10)
		Da_Write(51);
	else
		Da_Write(dis_value / 70.0 * 4 * 51);
	light_value_old = light_value;
	// 参数界面
	if (Seg_show_mode)
	{
		Seg_Buf[0] = 18; // P
		Seg_Buf[1] = (uchar)celi_show_mode + 1;
		if (celi_show_mode)
		{
			Seg_Buf[6] = collection_time / 10 % 10;
			Seg_Buf[7] = collection_time % 10;
		}
		else
		{
			Seg_Buf[6] = dis_demo / 10 % 10;
			Seg_Buf[7] = dis_demo % 10;
		}
	}
	// 数据界面
	else
	{
		switch (data_show_mode)
		{
		case 0:
			/* 时间显示 */
			Seg_Buf[0] = ucRtc[0] / 16;
			Seg_Buf[1] = ucRtc[0] % 16;
			Seg_Buf[2] = 16;
			Seg_Buf[3] = ucRtc[1] / 16;
			Seg_Buf[4] = ucRtc[1] % 16;
			Seg_Buf[5] = 16;
			Seg_Buf[6] = ucRtc[2] / 16;
			Seg_Buf[7] = ucRtc[2] % 16;
			break;
		case 1:
			/*距离显示*/
			Seg_Buf[0] = 11; // L
			Seg_Buf[1] = (burst_mode) ? 12 : 13;
			Seg_Buf[5] = dis_value / 100 % 10;
			Seg_Buf[6] = dis_value / 10 % 10;
			Seg_Buf[7] = dis_value % 10;
			hide_high_Seg(5, 7);
			break;
		case 2:
			/*数据记录*/
			Seg_Buf[0] = 14; // H
			Seg_Buf[1] = data_show_memory + 15;
			switch (data_show_memory)
			{
			case 0:
				/* 最大值 */
				Seg_Buf[4] = dis_max / 1000 % 10;
				Seg_Buf[5] = dis_max / 100 % 10;
				Seg_Buf[6] = dis_max / 10 % 10;
				Seg_Buf[7] = dis_max % 10;
				hide_high_Seg(4, 7);
				break;
			case 1:
				/*平均值*/
				temp_dis = dis_aver * 10;
				Seg_Buf[4] = temp_dis / 1000 % 10;
				Seg_Buf[5] = temp_dis / 100 % 10;
				Seg_Buf[6] = temp_dis / 10 % 10;
				Seg_Buf[7] = temp_dis % 10;
				hide_high_Seg(4, 6);
				Seg_Point[6] = 1;
				break;
			case 2:
				/*最小值*/
				Seg_Buf[4] = dis_min / 1000 % 10;
				Seg_Buf[5] = dis_min / 100 % 10;
				Seg_Buf[6] = dis_min / 10 % 10;
				Seg_Buf[7] = dis_min % 10;
				hide_high_Seg(4, 7);
				break;
			}
			break;
		}
	}
}

/* 其他显示函数 */
void Led_Proc()
{
	ucLed[0] = (Seg_show_mode == 0 && data_show_mode == 0);
	ucLed[1] = (Seg_show_mode == 0 && data_show_mode == 1);
	ucLed[2] = (Seg_show_mode == 0 && data_show_mode == 2);
	ucLed[3] = !burst_mode;
	if (burst_mode)
	{
		Beep(wring_count >= 3);
		ucLed[4] = (wring_count >= 3);
	}
	else
	{
		Beep(0);
		ucLed[4] = 0;
	}
	ucLed[5] = (light_value > LIGHT_NUM);
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
	Set_Rtc(ucRtc);
	while (1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
	}
}