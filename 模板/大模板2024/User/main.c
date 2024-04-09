/* 头文件声明区 */
// 包含各种硬件驱动的头文件，如I2C通信、1-Wire通信、超声波传感器、单片机寄存器、数码管和串口通信等。
#include "ds1302.h"
#include "iic.h"
#include "onewire.h"
#include "ultrasound.h"
#include "string.h"
#include "stdio.h"
#include <Init.h>		  // 初始化底层驱动专用头文件
#include <Key.h>		  // 按键底层驱动专用头文件
#include <Led.h>		  // LED底层驱动专用头文件
#include <STC15F2K60S2.H> // 单片机寄存器专用头文件
#include <Seg.h>		  // 数码管底层驱动专用头文件
#include <Uart.h>		  // 串口底层驱动专用头文件

/* 变量声明区 */
// 定义各种全局变量，包括按键状态、数码管显示数据、LED显示数据、串口接收数据等。
unsigned char Key_Val, Key_Down, Key_Old, Key_Up;			 // 按键状态变量
unsigned char Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据
unsigned char Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // 数码管小数点数据
unsigned char Seg_Pos;										 // 数码管扫描位置
unsigned char ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // LED显示数据
unsigned char Uart_Recv[10];								 // 串口接收数据缓冲区
unsigned char Uart_Recv_Index;								 // 串口接收数据索引
unsigned char ucRtc[3] = {11, 12, 13};						 // 实时时钟数据
unsigned int Slow_Down;										 // 减速计数器
bit Seg_Flag, Key_Flag;										 // 数码管和按键的标志位
bit Uart_Flag;												 // 串口标志位
unsigned int Time_1s;										 // 1秒钟计数器
unsigned int Freq;											 // 频率计算变量
unsigned int Sys_Tick;										 // 系统时钟计数

/* 键盘处理函数 */
// 处理按键输入，检测按键的下降沿和上升沿，并更新按键状态。
void Key_Proc()
{
	if (Key_Flag)return;
	Key_Flag = 1;							  // 设置标志位，防止重复进入
	Key_Val = Key_Read();					  // 读取按键值
	Key_Down = Key_Val & (Key_Old ^ Key_Val); // 检测下降沿
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // 检测上升沿
	Key_Old = Key_Val;						  // 更新按键状态
}

/* 信息处理函数 */
// 更新数码管显示的数据
void Seg_Proc()
{
	if (Seg_Flag)return;
	Seg_Flag = 1; // 设置标志位
}

/* 其他显示函数 */
// LED显示处理函数，这里没有具体实现。
void Led_Proc() {}

/* 串口处理函数 */
// 处理串口接收到的数据，当接收到数据时更新接收索引和缓冲区。
void Uart_Proc()
{
	if (Uart_Recv_Index == 0)return;
	if (Sys_Tick >= 10)
	{
		Sys_Tick = Uart_Flag = 0;
		// 逻辑函数

		
		
		
		memset(Uart_Recv, 0, Uart_Recv_Index);
		Uart_Recv_Index = 0; // 重置接收索引
	}
}

/* 定时器0初始化函数 */
// 初始化定时器0，用于产生1ms的时钟中断。
void Timer0_Init(void)
{
	AUXR &= 0x7F; // 设置定时器时钟12T模式
	TMOD &= 0xF0; // 设置定时器模式为16位定时器
	TMOD |= 0x05;
	TL0 = 0; // 设置定时器初始值
	TH0 = 0; // 设置定时器初始值
	TF0 = 0; // 清除TF0标志位
	TR0 = 1; // 启动定时器
}

/* 定时器1初始化函数 */
// 初始化定时器1，用于产生1ms的时钟中断，并允许中断。
void Timer1_Init(void)
{
	AUXR &= 0xBF; // 设置定时器时钟12T模式
	TMOD &= 0x0F; // 设置定时器模式为16位定时器
	TL1 = 0x18;	  // 设置定时器初始值
	TH1 = 0xFC;	  // 设置定时器初始值
	TF1 = 0;	  // 清除TF1标志位
	TR1 = 1;	  // 启动定时器
	ET1 = 1;	  // 使能定时器1中断
	EA = 1;		  // 开启全局中断
}

/* 定时器1中断服务函数 */
// 定时器1的中断服务函数，用于更新系统时钟、处理按键和数码管显示。
void Timer1_Isr(void) interrupt 3
{
	if (++Slow_Down == 400)
	{
		Seg_Flag = Slow_Down = 0; // 更新数码管显示标志位
	}
	if (Slow_Down % 10 == 0)
	{
		Key_Flag = 0; // 更新按键处理标志位
	}
	if (Uart_Flag)
		Sys_Tick++;
	if (++Time_1s == 1000)
	{
		Time_1s = 0;		   // 重置1秒钟计数器
		Freq = TH0 << 8 | TL0; // 计算频率
		TH0 = 0;			   // 重置定时器0的值
		TL0 = 0;
	}
	Seg_Disp(Slow_Down % 8, Seg_Buf[Slow_Down % 8], Seg_Point[Slow_Down % 8]); // 更新数码管显示
	Led_Disp(Slow_Down % 8, ucLed[Slow_Down % 8]);							   // 更新LED显示
}

/* 串口1中断服务函数 */
// 串口1的中断服务函数，用于处理串口接收到的数据。
void Uart1Server() interrupt 4
{
	if (RI == 1) // 检测到串口接收中断
	{
		Uart_Flag = 1;					   // 设置串口标志位
		Sys_Tick = 0;					   // 重置系统时钟
		Uart_Recv[Uart_Recv_Index] = SBUF; // 保存接收到的数据
		Uart_Recv_Index++;				   // 更新接收索引
		RI = 0;							   // 清除中断标志位
	}
	if (Uart_Recv_Index > 10)
		Uart_Recv_Index = 0;
}

/* 主函数 */
// 系统初始化，设置定时器和串口，然后进入主循环。
void main()
{
	System_Init();	// 系统初始化
	Timer1_Init();	// 初始化定时器1
	Set_Rtc(ucRtc); // 设置实时时钟
	UartInit();		// 初始化串口
	while (1)
	{
		Key_Proc();	 // 处理按键
		Seg_Proc();	 // 更新数码管显示
		Led_Proc();	 // 更新LED显示
		Uart_Proc(); // 处理串口数据
	}
}