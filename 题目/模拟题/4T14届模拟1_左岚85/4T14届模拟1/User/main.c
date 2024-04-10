#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                      // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // 某位是否显示小数点

/* 串口方面 */
uchar Uart_Buf[10];  // 串口接收到的数据
uchar Uart_Rx_Index; // 串口接收到的数据的指针

/* 时间方面 */
uint time_all_1s;
uchar Sys_Tick;
uchar time_100ms;
/* 判断 */
bit Uart_Flag;
bit Wring_Flag;
bit Led_Blink_Flag;
/* 显示 */
uchar Seg_show_mode; // 0 噪音分贝 1 参数显示
/* 数据 */
uint Noise_Value_10x;
uchar Noise_Para = 65;
/* 键盘处理函数 */
void Key_Proc()
{
    static uchar Key_Val, Key_Down, Key_Up, Key_Old;
    if (time_all_1s % 10)
        return;
    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;
    if (Key_Down == 12)
        Seg_show_mode = (++Seg_show_mode) % 2;
    if (Seg_show_mode == 1)
    {
        if (Key_Down == 16)
            Noise_Para = (Noise_Para == 90) ? 0 : Noise_Para + 5;
        else if (Key_Down == 17)
            Noise_Para = (Noise_Para == 0) ? 90 : Noise_Para - 5;
    }
}
/* 数码管处理函数 */
void Seg_Proc()
{
    if (time_all_1s % 50)
        return;
    Noise_Value_10x = Ad_Read(0x03) * 180 / 51;
    Wring_Flag = Noise_Value_10x > Noise_Para * 10;

    Seg_Buf[0] = 11; // U
    Seg_Buf[1] = Seg_show_mode + 1;
    switch (Seg_show_mode)
    {
    case 0:
        /* 噪音分贝显示 */
        Seg_Point[6] = 1;
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
        Seg_Buf[5] = (Noise_Value_10x / 100 % 10 == 0) ? 10
                                                       : Noise_Value_10x / 100 % 10;
        Seg_Buf[6] = Noise_Value_10x / 10 % 10;
        Seg_Buf[7] = Noise_Value_10x % 10;
        break;

    case 1:
        /* 分贝参数 */
        Seg_Point[6] = 0;
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
        Seg_Buf[6] = (Noise_Para / 10 % 10 == 0) ? 10
                                                 : Noise_Para / 10 % 10;
        Seg_Buf[7] = Noise_Para % 10;
        break;
    }
}

/* LED处理函数 */
void Led_Proc()
{
    ucLed[0] = (Seg_show_mode == 0);
    ucLed[1] = (Seg_show_mode == 1);
    ucLed[7] = Led_Blink_Flag;
}

/* 串口处理函数 */
void Uart_Proc()
{
    if (Uart_Rx_Index == 0)
        // 没有接收到任何数据
        return;
    if (Sys_Tick >= 10)
    {
        Sys_Tick = Uart_Flag = 0;
        if (Seg_show_mode == 0)
        {
            if (Uart_Buf[0] == 'R' && Uart_Buf[1] == 'e' && Uart_Buf[2] == 't' &&
                Uart_Buf[3] == 'u' && Uart_Buf[4] == 'r' && Uart_Buf[5] == 'n')
                printf("Noises:%0.1fdB", (float)Noise_Value_10x / 10.0);
        }
        memset(Uart_Buf, 0, Uart_Rx_Index);
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
    if (++time_all_1s == 1000)
        time_all_1s = 0;
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
    if (Uart_Flag)
        Sys_Tick++;
    if (Wring_Flag)
    {
        if (++time_100ms == 100)
        {
            time_100ms = 0;
            Led_Blink_Flag ^= 1;
        }
    }
    else
    {
        time_100ms = 0;
        Led_Blink_Flag = 0;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

/* 串口中断服务函数 */
void Uart_ISR(void) interrupt 4
{
    if (RI == 1) // 串口接收到数据
    {
        Uart_Flag = 1;
        Sys_Tick = 0;
        Uart_Buf[Uart_Rx_Index] = SBUF;
        Uart_Rx_Index++;
        RI = 0;
    }
    if (Uart_Rx_Index > 10)
        Uart_Rx_Index = 0;
}
void main()
{
    System_Init();
    Timer0_Init();
    Uart1_Init();

    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Uart_Proc();
        Led_Proc();
    }
}