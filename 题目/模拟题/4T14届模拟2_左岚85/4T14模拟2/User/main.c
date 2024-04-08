#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Buf[8] = {10 , 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                      // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // 某位是否显示小数点

/* 串口方面 */
uchar Uart_Slow_Down;
uchar Uart_Buf[3];   // 串口接收到的数据
uchar Uart_Rx_Index; // 串口接收到的数据的指针

/* 时间方面 */
uint time_all_1s;
uchar time_100ms;

/* 界面 */
uchar Seg_show_mode; // 0 温度显示 1 电压显示

/* 数据 */
uint T_value_10x;  // 10倍温度值
uint V_value_100x; // 100倍电压值

/* 判断 */
bit Data_send_flag;   // 判断是否应该发送数据
bit Lock_uart_change; // 串口修改页面功能是否锁定
bit Led_blink_flag;   // 闪烁标志

/* 数据处理函数 */
void Data_Proc()
{
    if (time_all_1s % 100 == 0)
    {
        // AD读取
        V_value_100x = (float)Ad_Read(0x03) / 51.0 * 100;
    }
    if (time_all_1s % 500 == 0)
    {
        // 温度读取
        T_value_10x = rd_temperature() * 10;
    }
}
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
        Data_send_flag = 1;
    if (Key_Down == 4)
        Lock_uart_change = 1;
    if (Lock_uart_change && Key_Down == 5)
        Lock_uart_change = 0;
}
/* 数码管处理函数 */
void Seg_Proc()
{
    if (time_all_1s % 20)
        return;
    Seg_Buf[0] = 11; // U
    Seg_Buf[1] = Seg_show_mode + 1;
    Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
    if (Seg_show_mode == 0)
    {
        Seg_Point[5] = 0;
        Seg_Point[6] = 1;
        Seg_Buf[5] = T_value_10x / 100 % 10;
        Seg_Buf[6] = T_value_10x / 10 % 10;
        Seg_Buf[7] = T_value_10x % 10;
    }
    else
    {
        Seg_Point[5] = 1;
        Seg_Point[6] = 0;
        Seg_Buf[5] = V_value_100x / 100 % 10;
        Seg_Buf[6] = V_value_100x / 10 % 10;
        Seg_Buf[7] = V_value_100x % 10;
    }
}

/* LED处理函数 */
void Led_Proc()
{
    ucLed[0] = (Seg_show_mode == 0);
    ucLed[1] = (Seg_show_mode == 1);
    ucLed[2] = Led_blink_flag;
    Relay(T_value_10x >= 280);
    Beep(V_value_100x > 360);
}

/* 串口处理函数 */
void Uart_Proc()
{
    if (time_all_1s % 200)
        return;
    if (Data_send_flag)
    {
        Data_send_flag = 0;
        if (Seg_show_mode == 0)
            printf("TEMP:%0.1f℃", (float)T_value_10x / 10.0);
        else
            printf("Voltage:%0.2fV", (float)V_value_100x / 100.0);
    }
    if (Lock_uart_change == 0)
    {
        // 未被锁定
        if (Uart_Buf[0] == 'A')
            Seg_show_mode = 0;
        if (Uart_Buf[0] == 'B')
            Seg_show_mode = 1;
        memset(Uart_Buf, 0, 3);
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
    if (Lock_uart_change)
    {
        if (++time_100ms == 100)
        {
            time_100ms = 0;
            Led_blink_flag ^= 1;
        }
    }
    else
    {
        time_100ms = 0;
        Led_blink_flag = 0;
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
    System_Init();
    Timer0_Init();
    Uart1_Init();
    rd_temperature();
    Delay750ms();
    T_value_10x = rd_temperature() * 10;
		Delay750ms();
    while (1)
    {
        Key_Proc();
        Data_Proc();
        Seg_Proc();
        Uart_Proc();
        Led_Proc();
    }
}