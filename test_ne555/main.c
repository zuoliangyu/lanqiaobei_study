#include <STC15F2K60S2.H>
unsigned int freq;
unsigned int time_1s;
// 计数器
void Timer0Init(void) // 0微秒@12.000MHz
{
    AUXR &= 0x7F; // 定时器时钟12T模式
    TMOD &= 0xF0; // 设置定时器模式
    TMOD |= 0x05;
    TL0 = 0x00; // 设置定时初值
    TH0 = 0x00; // 设置定时初值
    TF0 = 0;    // 清除TF0标志
    TR0 = 1;    // 定时器0开始计时
}
// 定时器
void Timer1Init(void) // 1毫秒@12.000MHz
{
    AUXR &= 0xBF; // 定时器时钟12T模式
    TMOD &= 0x0F; // 设置定时器模式
    TL1 = 0x18;   // 设置定时初值
    TH1 = 0xFC;   // 设置定时初值
    TF1 = 0;      // 清除TF1标志
    TR1 = 1;      // 定时器1开始计时
    ET1 = 1;      // 允许定时器1中断
    EA = 1;       // 允许总中断
}
void Timer1Sever() interrupt 3
{
    if (++time_1s == 1000)
    {
        time_1s = 0;
        // 1秒定时器中断
        freq = TH0 << 8 | TL0; // 读取定时器0当前值
        TH0 = 0;
        TL0 = 0;
    }
}
void main()
{
}