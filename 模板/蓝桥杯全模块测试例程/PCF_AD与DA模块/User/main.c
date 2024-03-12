#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Slow_Down;                                // 数码管减速
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                      // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // 某位是否显示小数点

/* 键盘方面 */
uchar Key_Slow_Down;

/* 数据 */
uchar external_input;     // 外部输入
uchar light_input;        // 光敏电阻输入
uchar differential_input; // 差分电路输入
uchar Rb2_input;          // Rb2滑动变阻器输入
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
    /* 这里因为数码管显示不了全部，所以就显示最常用的两个*/
    // external_input = Ad_Read(0x40);
    light_input = Ad_Read(0x41);
    // differential_input = Ad_Read(0x42);
    Rb2_input = Ad_Read(0x43);
    Seg_Buf[0] = light_input / 100 % 10;
    Seg_Buf[1] = light_input / 10 % 10;
    Seg_Buf[2] = light_input % 10;
    Seg_Buf[5] = Rb2_input / 100 % 10;
    Seg_Buf[6] = Rb2_input / 10 % 10;
    Seg_Buf[7] = Rb2_input % 10;
}

/* LED处理函数 */
void Led_Proc()
{
    Da_Write(Rb2_input); // 这里让DA的输出电压和Rb2一致
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
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

void main()
{
    System_Init();
    Timer0_Init();
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}