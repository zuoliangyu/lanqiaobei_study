#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0,0,0,0,0,0,0,0};
/* 数码管显示 */
uchar Seg_Buf[8]={10,10,10,10,10,10,10,10};
uchar Seg_Point[8]={0,0,0,0,0,0,0,0};
uchar Seg_Pos;

/* 串口相关 */
uchar Uart_Buf[10];
uchar Uart_Rx_Index;
/* 时间 */
uint time_all_1s;

/* 数据处理 */
void Data_Porc()
{
	if(time_all_1s%100==0)
	{
		//时间读取
	}
		if(time_all_1s%200==0)
	{
		//AD读取
	}
}

/* 按键处理函数 */
void Key_Porc()
{
	static unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
	if(time_all_1s % 10)
		return ;
	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Old ^ Key_Val);
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
	Key_Old = Key_Val;
}

/* 数码管显示处理函数 */
void Seg_Porc()
{
	if(time_all_1s % 20)
		return ;
}
/* 其他显示函数 */
void Led_Porc()
{
}
/* 串口处理函数*/
void Uart_Porc()
{
	if(time_all_1s % 200)
		return ;
	if(Uart_Buf[0]=="O"&&Uart_Buf[1]=="K")
	{
		//执行OK收到后的函数
		printf("hello");
		memset(Uart_Buf,0,10);
		Uart_Rx_Index=0;
	}
}
void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0x7F;			//定时器时钟12T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x18;				//设置定时初始值
	TH0 = 0xFC;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0 = 1;				//使能定时器0中断
	EA = 1;
}

void Timer0_Isr(void) interrupt 1
{
	if(++time_all_1s == 1000)
		time_all_1s = 0;
	if(++Seg_Pos==8)
		Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}
void Uart1_Isr(void) interrupt 4
{
	if(RI)
	{
		Uart_Buf[Uart_Rx_Index] = SBUF;
		Uart_Rx_Index++;
		RI=0;
	}
}
void main()
{
	System_Init();
	Timer0_Init();
	Uart1_Init();
	while(1)
	{
		Key_Porc();
		Data_Porc();
		Seg_Porc();
		Led_Porc();
		Uart_Porc();
	}
}
