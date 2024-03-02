#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Slow_Down;                                // 数码管减速
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                      // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // 某位是否显示小数点

/* 串口方面 */
uchar Uart_Slow_Down;
uchar Uart_Buf[20];  // 串口接收到的数据
uchar Uart_Rx_Index; // 串口接收到的数据的指针

/* 时间方面 */
uchar ucRtc[3] = {0x13, 0x11, 0x11}; // 初始化时间13:11:11

/* 键盘方面 */
uchar Key_Slow_Down;
/* 键盘处理函数 */
void Key_Proc()
{
    static uchar Key_Val, Key_Down, Key_Up, Key_Old;
    if (Key_Slow_Down)
        return;
    Key_Slow_Down = 1;

    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;
}
/* 数码管处理函数 */
void Seg_Proc()
{
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
}

/* LED处理函数 */
void Led_Proc()
{
}

/* 串口处理函数 */
void Uart_Proc()
{
    if (Uart_Slow_Down)
        return;
    Uart_Slow_Down = 1;
    if (Uart_Buf[0] == 'O' && Uart_Buf[1] == 'K')
    {
        // 执行相关函数

        memset(Uart_Buf, 0, 20);
        Uart_Rx_Index = 0;
    }
}

/* 定时器0中断初始化 */
void Timer0_Init(void) // 1毫秒@12.000MHz
{
    AUXR &= 0x7F; // 定时器时钟12T模式
    TMOD &= 0xF0; // 设置定时器模式
    TL0 = 0x18;   // 设置定时初始值
    TH0 = 0xFC;   // 设置定时初始值
    TF0 = 0;      // 清除TF0标志
    TR0 = 1;      // 定时器0开始计时
    ET0 = 1;
    EA = 1;
}

/* 定时器0中断函数 */
void Timer0_ISR(void) interrupt 1
{
    if (++Key_Slow_Down == 10)
        Key_Slow_Down = 0;
    if (++Seg_Slow_Down == 500)
        Seg_Slow_Down = 0;
    if (++Uart_Slow_Down == 200)
        Uart_Slow_Down = 0;
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

/* 串口中断服务函数 */
void Uart_ISR(void) interrupt 4
{
    if (RI == 1) // 串口接收到数据
    {
        Uart_Buf[Uart_Rx_Index] = SBUF;
        Uart_Rx_Index++;
        RI = 0;
    }
}
void Delay750ms(void) //@12.000MHz
{
    unsigned char data i, j, k;

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
void main()
{

    Timer0_Init();
    Uart1_Init();
    Set_Rtc(ucRtc);
    Delay750ms();
    rd_temperature();
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Uart_Proc();
        Led_Proc();
    }
}