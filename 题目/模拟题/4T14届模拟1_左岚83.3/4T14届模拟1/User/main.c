#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uint Seg_Slow_Down;                                 // 数码管减速
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                      // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // 某位是否显示小数点

/* 串口方面 */
uchar Uart_Slow_Down;
uchar Uart_Buf[20];  // 串口接收到的数据
uchar Uart_Rx_Index; // 串口接收到的数据的指针

/* 键盘方面 */
uchar Key_Slow_Down;

/* 显示 */
uchar Seg_show_mode; // 0 距离显示 1 参数显示
uchar Dis_value;
uchar Dis_para;
bit Send_data_flag;
bit Wring_flag;
bit Led_show_flag;
uchar time_200ms;
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
    if (Key_Down == 4)
        Seg_show_mode = (++Seg_show_mode) % 2;
    if (Seg_show_mode == 0)
    {
        if (Key_Down == 8)
            Dis_para = Dis_value;
    }
    else
    {
        if (Key_Down == 12)
            Dis_para = (Dis_para >= 245) ? 255 : Dis_para + 10;
        else if (Key_Down == 16)
            Dis_para = (Dis_para <= 10) ? 0 : Dis_para - 10;
    }
    if (Key_Down == 9)
        Send_data_flag = 1;
}
/* 数码管处理函数 */
void Seg_Proc()
{
    uchar i;
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    Dis_value = Ut_Wave_Data();
    if (Dis_value > Dis_para)
        Wring_flag = 1;
    else
        Wring_flag = 0;
    Seg_Buf[0] = 11; // U
    Seg_Buf[1] = Seg_show_mode + 1;
    memset(Seg_Buf + 2, 10, 3);
    Seg_Buf[5] = (Seg_show_mode == 0) ? Dis_value / 100 % 10
                                      : Dis_para / 1000 % 10;
    Seg_Buf[6] = (Seg_show_mode == 0) ? Dis_value / 10 % 10
                                      : Dis_para / 100 % 10;
    Seg_Buf[7] = (Seg_show_mode == 0) ? Dis_value % 10
                                      : Dis_para / 10 % 10;
    for (i = 5; i < 7; i++)
    {
        if (Seg_Buf[i - 1] == 10 && Seg_Buf[i] == 0)
            Seg_Buf[i] = 10;
        else
            break;
    }
}

/* LED处理函数 */
void Led_Proc()
{
    ucLed[0] = (Seg_show_mode == 0);
    ucLed[1] = (Seg_show_mode == 1);
    ucLed[2] = Led_show_flag;
    memset(ucLed + 3, 0, 5);
}

/* 串口处理函数 */
void Uart_Proc()
{
    if (Uart_Slow_Down)
        return;
    Uart_Slow_Down = 1;
    if (Send_data_flag)
    {
        printf("Distance:%bucm", Dis_value);
        Send_data_flag = 0;
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
    if (++Seg_Slow_Down == 300)
        Seg_Slow_Down = 0;
    if (++Uart_Slow_Down == 200)
        Uart_Slow_Down = 0;
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
    if (Wring_flag)
    {
        if (++time_200ms == 200)
        {
            time_200ms = 0;
            Led_show_flag ^= 1;
        }
    }
    else
    {
        time_200ms = 0;
        Led_show_flag = 0;
    }
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
void main()
{
    System_Init();
    Timer0_Init();
    Uart1_Init();
    Dis_para = 30;
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Uart_Proc();
        Led_Proc();
    }
}