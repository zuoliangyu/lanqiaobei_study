#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Slow_Down;                                 // 数码管减速
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                       // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};       // 某位是否显示小数点

/* 键盘方面 */
uchar Key_Slow_Down;

/* 数据 */
uchar Work_mode;   // 0 睡眠风20 1 自然风30 2 常风70
bit Seg_show_mode; // 0 显示风速 1 显示温度

/* 时间 */
uchar Work_time;       // 工作时长
uchar time_mode_index; // 0->0分 1->1分 2->2分
uchar time_mode[3] = {0, 60, 120};
uchar Work_mode_P34[3] = {2, 3, 7};
uchar time_1ms;
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

    if (Key_Down == 4)
        Work_mode = (++Work_mode) % 3;
    if (Key_Down == 5)
    {
        time_mode_index = (++time_mode_index) % 3;
        Work_time = time_mode[time_mode_index];
    }
    if (Key_Down == 6)
        Work_time = 0;
    if (Key_Down == 7)
        Seg_show_mode ^= 1;
}
/* 数码管处理函数 */
void Seg_Proc()
{
    uchar T_value;
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    Seg_Buf[0] = 11; //-
    Seg_Buf[2] = 11; //-
    Seg_Buf[3] = 10; // 灭
    T_value = rd_temperature();
    if (Seg_show_mode == 0)
    {
        // 现在是风速显示页面
        Seg_Buf[1] = Work_mode + 1;
        Seg_Buf[4] = Work_time / 1000 % 10;
        Seg_Buf[5] = Work_time / 100 % 10;
        Seg_Buf[6] = Work_time / 10 % 10;
        Seg_Buf[7] = Work_time % 10;
    }
    else
    {
        // 现在是温度显示页面
        Seg_Buf[1] = 4;
        Seg_Buf[4] = 10; // 灭
        Seg_Buf[5] = T_value / 10 % 10;
        Seg_Buf[6] = T_value % 10;
        Seg_Buf[7] = 12; // C
    }
}

/* LED处理函数 */
void Led_Proc()
{
    if (Work_time)
    {
        // 当计时没有结束
        ucLed[0] = (Work_mode == 0);
        ucLed[1] = (Work_mode == 1);
        ucLed[2] = (Work_mode == 2);
    }
    else
        memset(ucLed, 0, 3);
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

void Timer1_Init(void) // 100微秒@12.000MHz
{
    AUXR |= 0x40; // 定时器时钟1T模式
    TMOD &= 0x0F; // 设置定时器模式
    TL1 = 0x50;   // 设置定时初始值
    TH1 = 0xFB;   // 设置定时初始值
    TF1 = 0;      // 清除TF1标志
    TR1 = 1;      // 定时器1开始计时
    ET1 = 1;      // 使能定时器1中断
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
    if (Work_time)
    {
        if (++time_1s == 1000)
        {
            time_1s = 0;
            Work_time--;
        }
    }
    // 防止重新设定后出现不足1s的bug
    else
    {
        time_1s = 0;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}
void Timer1_Isr(void) interrupt 3
{
    // 当工作时间没有为0的时候
    if (Work_time)
    {
        if (++time_1ms == 10)
            time_1ms = 0;
        // 当计时小于我们的时候，就让他高电平，否则低电平，触发占空比
        if (time_1ms < Work_mode_P34[Work_mode])
            P34 = 1;
        else
            P34 = 0;
    }
    // 工作时间归零，为了避免bug，我们把全部拉低
    else
    {
        time_1ms = 0;
        P34 = 0;
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
    Timer1_Init();
    rd_temperature();
    Delay750ms();
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}