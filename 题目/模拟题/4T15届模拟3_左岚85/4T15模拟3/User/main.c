#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Slow_Down;                                // 数码管减速
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                      // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // 某位是否显示小数点

/* 界面显示 */
uchar Seg_show_mode; // 0 测距 1 参数 2 记录
uchar Control_mode;  // 0 按键 1 旋钮
/* 键盘方面 */
uchar Key_Slow_Down;
/* 数据 */
uchar Distance_value; // 距离测量数据
uchar Para_max = 60;  // 参数上限
uchar Para_min = 10;  // 参数下限
uchar Wring_count;    // 报警次数
bit Wring_flag;       // 报警标志
uchar Para_mode;      // 0 不控制 1 上限 2 下限
uchar Vol_level;      // 电压等级，用于控制上下限 1 2 3 4
bit Led_show_flag;    // LED闪烁
/* 时间 */
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
    if (Key_Down == 4)
        Seg_show_mode = (++Seg_show_mode) % 3;

    if (Key_Down == 5)
    {
        if (Seg_show_mode == 1)
            Control_mode = (++Control_mode) % 2;
        if (Seg_show_mode == 2)
            Wring_count = 0;
        Para_mode = 0; // 这里切换的时候将旋钮调整的上下限模式重置，防止出现bug
    }
    if (Seg_show_mode == 1)
    { // 按键控制模式
        if (Control_mode == 0)
        {
            if (Key_Down == 9)
                Para_max = (Para_max + 10 > 90) ? 50
                                                : Para_max + 10;
            if (Key_Down == 8)
                Para_min = (Para_min + 10 > 40) ? 0
                                                : Para_min + 10;
        }
        // 旋钮控制模式
        else
        {
            if (Key_Down == 9)
                Para_mode = 1;
            if (Key_Down == 8)
                Para_mode = 2;
        }
    }
}
/* 数码管处理函数 */
void Seg_Proc()
{
    uchar i;
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    Distance_value = Ut_Wave_Data();
    // 超出上下限
    if ((Distance_value < Para_min || Distance_value > Para_max))
    {
        // 没有报警的时候
        if (Wring_flag == 0)
        {
            Wring_count++;
            Wring_flag = 1;
        }
    }
    // 没有超出上下限，拉低报警flag，准备下一次报警
    else
    {
        Wring_flag = 0;
    }
    // 当我们处于旋钮控制才进行采集电压
    if (Control_mode == 1)
    {
        Vol_level = Ad_Read(0x03) / 51;
        Vol_level = (Vol_level >= 4) ? 4 : Vol_level; // 限制一下，防止出现判定5的极端情况
        if (Para_mode == 1)
            Para_max = Vol_level * 10 + 50;
        else if (Para_mode == 2)
            Para_min = Vol_level * 10;
    }
    switch (Seg_show_mode)
    {
    case 0:
        /* 测距界面 */
        Seg_Buf[0] = 11; // A
        for (i = 1; i < 5; i++)
        {
            Seg_Buf[i] = 10;
        }
        Seg_Buf[5] = (Distance_value / 100 % 10 == 0) ? 10
                                                      : Distance_value / 100 % 10;
        Seg_Buf[6] = ((Distance_value / 10 % 10 == 0) && (Seg_Buf[5] == 10)) ? 10
                                                                             : Distance_value / 10 % 10;
        Seg_Buf[7] = Distance_value % 10;
        break;

    case 1:
        /* 参数界面 */
        Seg_Buf[0] = 12; // P
        Seg_Buf[1] = Control_mode + 1;
        Seg_Buf[2] = 10;
        Seg_Buf[3] = Para_min / 10 % 10;
        Seg_Buf[4] = Para_min % 10;
        Seg_Buf[5] = 13; //-
        Seg_Buf[6] = Para_max / 10 % 10;
        Seg_Buf[7] = Para_max % 10;
        break;

    case 2:
        /* 记录界面 */
        Seg_Buf[0] = 14; // E
        for (i = 1; i < 7; i++)
        {
            Seg_Buf[i] = 10;
        }
        Seg_Buf[7] = (Wring_count > 9) ? 13
                                       : Wring_count;
        break;
    }
}

/* LED处理函数 */
void Led_Proc()
{
    ucLed[0] = (Seg_show_mode == 0);
    ucLed[1] = (Seg_show_mode == 1);
    ucLed[2] = (Seg_show_mode == 2);
    ucLed[7] = (Wring_flag == 0) ? 1
                                 : Led_show_flag;
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
    if (Wring_flag)
    {
        if (++time_100ms == 100)
        {
            time_100ms = 0;
            Led_show_flag ^= 1;
        }
    }
    else
    {
        time_100ms = 0;
    }
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