#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Slow_Down;                                 // 数码管减速
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                       // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};       // 某位是否显示小数点

/* 时间方面 */
uchar ucRtc[3] = {0x13, 0x11, 0x11}; // 初始化时间13:11:11

/* 键盘方面 */
uchar Key_Slow_Down;

/* 显示 */
uchar Seg_show_mode; // 0 模式界面 1 输出界面
uchar Control_mode;

/* 数据 */
uint temperature_value_10x;
uchar light_value;
uchar DAC_out_dig;     // 数字输出0-255
uchar DAC_out_ana_10x; // 模拟输出0-50
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
        Control_mode = (++Control_mode) % 2;
    if (Key_Down == 5)
        Seg_show_mode = (++Seg_show_mode) % 2;
}
/* 数码管处理函数 */
void Seg_Proc()
{
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    temperature_value_10x = rd_temperature() * 10;
    light_value = Ad_Read(0x41);
    DAC_out_dig = DAC_out_ana_10x * 51 / 10;
    Da_Write(DAC_out_dig);
    switch (Seg_show_mode)
    {
    case 0:
        /* 模式界面 */
        Seg_Buf[0] = Control_mode + 1;
        Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
        if (Control_mode == 0)
        {

            Seg_Buf[5] = temperature_value_10x / 100 % 10;
            Seg_Buf[6] = temperature_value_10x / 10 % 10;
            Seg_Buf[7] = temperature_value_10x % 10;
            Seg_Point[6] = 1;
        }
        else
        {
            Seg_Buf[5] = (light_value / 100 % 10 == 0) ? 10 : light_value / 100 % 10;
            Seg_Buf[6] = ((light_value / 10 % 10 == 0) && (Seg_Buf[5] == 10)) ? 10 : light_value / 10 % 10;
            Seg_Buf[7] = light_value % 10;
            Seg_Point[6] = 0;
        }
        break;

    case 1:
        /* 输出界面 */
        Seg_Buf[0] = 11; // U

        Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
        Seg_Buf[6] = DAC_out_ana_10x / 10 % 10;
        Seg_Buf[7] = DAC_out_ana_10x % 10;
        Seg_Point[6] = 1;
        break;
    }
}

/* LED处理函数 */
void Led_Proc()
{
    if (Control_mode == 0)
    {
        if (temperature_value_10x >= 400)
            DAC_out_ana_10x = 50;
        else if (temperature_value_10x <= 100)
            DAC_out_ana_10x = 10;
        else
            DAC_out_ana_10x =
                (temperature_value_10x - 100) * 2 / 15 + 10;
    }
    else
    {
        if (light_value >= 240)
            DAC_out_ana_10x = 50;
        else if (light_value <= 10)
            DAC_out_ana_10x = 10;
        else
            DAC_out_ana_10x =
                (light_value - 10) * 4 / 23 + 10;
    }
    ucLed[0] = (Control_mode == 0);
    ucLed[1] = (Control_mode == 1);
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
    if (++Seg_Slow_Down == 200)
        Seg_Slow_Down = 0;
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
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
    Delay750ms();
    rd_temperature();
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}