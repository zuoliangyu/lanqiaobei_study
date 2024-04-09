#include <Uart.h>

/* 串口初始化函数 */
void UartInit(void) // 9600bps@12.000MHz
{
	SCON = 0x50;  // 8位数据,可变波特率
	AUXR |= 0x01; // 串口1选择定时器2为波特率发生器
	AUXR |= 0x04; // 定时器时钟1T模式
	T2L = 0xC7;	  // 设置定时初始值
	T2H = 0xFE;	  // 设置定时初始值
	AUXR |= 0x10; // 定时器2开始计时
	ES = 1;
	EA = 1;
}
extern char putchar(char ch)
{
	SBUF = ch; // 将ch写入SBUF，发出数据
	while (TI == 0)
		;	// 等待发送完成
	TI = 0; // 清除发送完成标志
	return ch;
}