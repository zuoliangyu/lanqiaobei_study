#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Slow_Down;                                // 数码管减速
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                      // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // 某位是否显示小数点

/* 时间方面 */
uchar ucRtc[3] = {0x13, 0x59, 0x50}; // 初始化时间13:59:50

/* 键盘方面 */
uchar Key_Slow_Down;

bit Time_ring; // 整点报时
uint time_1s;

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
    Read_Rtc(ucRtc);
    Seg_Buf[0] = ucRtc[0] / 16;
    Seg_Buf[1] = ucRtc[0] % 16;
    Seg_Buf[2] = 11; //-
    Seg_Buf[3] = ucRtc[1] / 16;
    Seg_Buf[4] = ucRtc[1] % 16;
    Seg_Buf[5] = 11; //-
    Seg_Buf[6] = ucRtc[2] / 16;
    Seg_Buf[7] = ucRtc[2] % 16;
}

/* LED处理函数 */
void Led_Proc()
{
    if ((ucRtc[1]==0) && (ucRtc[2]== 0))
        Time_ring = 1;
    Relay(Time_ring);
    Beep(Time_ring);
    if (Time_ring)
    {
        memset(ucLed, 1, 4);
        memset(ucLed + 4, 0, 4);
    }
    else
    {
        memset(ucLed, 0, 4);
        memset(ucLed + 4, 1, 4);
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
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
    if (Time_ring)
    {
        if (++time_1s == 1000)
        {
            time_1s = 0;
            Time_ring = 0;
        }
    }
    else
    {
        time_1s = 0;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

void main()
{
    System_Init();
    Timer0_Init();
    Set_Rtc(ucRtc);
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}