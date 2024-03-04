#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Slow_Down;                                // 数码管减速
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                      // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // 某位是否显示小数点

/* 时间方面 */
uchar ucRtc[3] = {0x13, 0x11, 0x11}; // 初始化时间13:11:11

/* 键盘方面 */
uchar Key_Slow_Down;

/* 显示与控制 */
uchar Seg_show_mode;  // 0 温度 1 时间 2 参数
bit Control_mode;     // 控制模式 0 温度 1 时间
bit Time_show_mode;   // 时间显示模式 0 时分 1 分秒
bit Temperature_mode; // 温度控制点亮
bit Time_mode;        // 时间控制点亮
bit Time_Led;         // 整点点亮
bit Led_show;         // 闪烁
/* 数据 */
uint Temperature_value_10x;  // 温度测量值 10倍
uchar Temperature_para = 23; // 温度参数

/* 时间 */
uint time_5s;
uint time_5s_led;
uchar time_100ms;

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
    if (Key_Down == 12)
        Seg_show_mode = (++Seg_show_mode) % 3;
    if (Key_Down == 13)
        Control_mode ^= 1;
    if (Seg_show_mode == 2)
    {
        if (Key_Down == 16)
            Temperature_para = (++Temperature_para > 99) ? 99 : Temperature_para;
        if (Key_Down == 17)
            Temperature_para = (--Temperature_para < 10) ? 10 : Temperature_para;
    }
    if (Seg_show_mode == 2)
    {
        if (Key_Old == 17)
            Time_show_mode = 1;
        if (Key_Up == 17)
            Time_show_mode = 0;
    }
}
/* 数码管处理函数 */
void Seg_Proc()
{
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    Seg_Buf[0] = 11; // U
    Seg_Buf[1] = Seg_show_mode + 1;
    Read_Rtc(ucRtc);
    Temperature_value_10x = rd_temperature() * 10;
    switch (Seg_show_mode)
    {
    case 0:
        /* 温度 */
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
        Seg_Buf[5] = Temperature_value_10x / 100 % 10;
        Seg_Buf[6] = Temperature_value_10x / 10 % 10;
        Seg_Buf[7] = Temperature_value_10x % 10;
        Seg_Point[6] = 1;
        break;
    case 1:
        /* 时间 */

        Seg_Buf[2] = 10;
        Seg_Buf[3] = Time_show_mode ? ucRtc[1] / 16 : ucRtc[0] / 16;
        Seg_Buf[4] = Time_show_mode ? ucRtc[1] % 16 : ucRtc[0] % 16;
        Seg_Buf[5] = 12; //-
        Seg_Buf[6] = Time_show_mode ? ucRtc[2] / 16 : ucRtc[1] / 16;
        Seg_Buf[7] = Time_show_mode ? ucRtc[2] % 16 : ucRtc[1] % 16;
        Seg_Point[6] = 0;
        break;
    case 2:
        /* 参数 */
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
        Seg_Buf[6] = Temperature_para / 10 % 10;
        Seg_Buf[7] = Temperature_para % 10;
        Seg_Point[6] = 0;
        break;
    }
}

/* LED处理函数 */
void Led_Proc()
{
    // 时间控制
    if (Control_mode)
    {
        Temperature_mode = 0; // 关闭温度报警，避免突然切换导致bug
        // 当分和秒同时是0x00的时候
        if ((ucRtc[1] || ucRtc[2]) == 0)
            Time_mode = 1; // 开启整点状态
    }
    // 温度控制
    else
    {
        Time_mode = 0; // 关闭整点状态，避免突然切换导致bug
        if (Temperature_value_10x >= Temperature_para * 10)
            Temperature_mode = 1; // 温度报警
        else
            Temperature_mode = 0; // 温度正常
    }
    Relay(Temperature_mode || Time_mode); // 处于温度报警或整点状态的时候吸合

    // 当分和秒同时是0x00的时候
    if ((ucRtc[1] || ucRtc[2]) == 0)
        Time_Led = 1;
    ucLed[0] = Time_Led; // LED整点亮5s
    ucLed[1] = (Control_mode == 0);
    ucLed[2] = Led_show;
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
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
    if (Time_mode)
    {
        if (++time_5s == 5000)
        {
            time_5s = 0;
            Time_mode = 0;
        }
    }
    // 当时间没有控制的时候，锁死定时器为0
    else
    {
        time_5s = 0;
    }

    if (Time_Led)
    {
        if (++time_5s_led == 5000)
        {
            time_5s_led = 0;
            Time_mode = 0;
        }
    }

    if (Temperature_mode || Time_mode)
    {
        if (++time_100ms == 100)
        {
            time_100ms = 0;
            Led_show ^= 1;
        }
    }
    else
    {
        time_100ms = 0;
        Led_show = 0;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
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
    Set_Rtc(ucRtc);
    Delay750ms();
    rd_temperature();
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}