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

/* 显示 */
uchar Led_show_mode;      // 0 左到右单循环 1 右到左循环 2 .. 3 ..
uint Flow_interval = 400; // 400-1200
uchar Seg_show_mode;      // 0 数码管全灭 1 模式编号设置 2 流转间隔设置
uchar Led_level;          // 0 1 2 3
uchar Led_show_index;     // 显示的数码管的第几对，用于流转12模式直接使用，34模式变换使用

/* 判断 */
bit Flow_flag;   // 是否开启流转
bit Seg_flicker; // 数码管闪烁
bit Show_level;  // 是否显示等级
uchar Pwm_value;
/* EEPROM数据 */
uchar old_passwd;
uchar passwd = 123;

/* 时间 */
uint time_800ms;
uint time_flow; // 流转计时
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
    if (Key_Down == 7)
        Flow_flag ^= 1;
    if (Key_Down == 6)
        Seg_show_mode = (++Seg_show_mode) % 3;
    if (Key_Down == 5)
    {
        if (Seg_show_mode == 1)
        {
            Led_show_mode = (Led_show_mode + 1) % 4;
        }
        else if (Seg_show_mode == 2)
        {
            Flow_interval = (Flow_interval + 100 > 1200) ? 400 : Flow_interval + 100;
        }
    }

    if (Key_Down == 4)
    {
        if (Seg_show_mode == 1)
        {
            Led_show_mode = (Led_show_mode == 0) ? 3 : Led_show_mode - 1;
        }
        else if (Seg_show_mode == 2)
        {
            Flow_interval = (Flow_interval - 100 < 400) ? 1200 : Flow_interval - 100;
        }
    }
    if (Seg_show_mode == 0)
        if (Key_Old == 4)
        {
            Show_level = 1;
        }
        // 如果我们没有按下S4
        else
        {
            Show_level = 0;
        }
}
/* 数码管处理函数 */
void Seg_Proc()
{
    uchar i;
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    Led_level = Ad_Read(0x03) / 64;
    if (Seg_show_mode == 0)
    {
        if (Show_level)
        {
            memset(Seg_Buf, 10, 6);     //
            Seg_Buf[6] = 11;            //-
            Seg_Buf[7] = Led_level + 1; //
        }
        else
            memset(Seg_Buf, 10, 8); // 全灭
    }
    else
    {
        Seg_Buf[0] = 11; //-
        Seg_Buf[1] = Led_show_mode + 1;
        Seg_Buf[2] = 11; //-
        Seg_Buf[3] = 10;
        Seg_Buf[4] = (Flow_interval / 1000 == 0) ? 10 : Flow_interval / 1000 % 10;
        Seg_Buf[5] = Flow_interval / 100 % 10;
        Seg_Buf[6] = Flow_interval / 10 % 10;
        Seg_Buf[7] = Flow_interval % 10;
        if (Seg_show_mode == 1)
        {
            for (i = 0; i < 3; i++)
                Seg_Buf[i] = (Seg_flicker) ? Seg_Buf[i] : 10;
        }
        else if (Seg_show_mode == 2)
        {
            for (i = 4; i < 8; i++)
                Seg_Buf[i] = (Seg_flicker) ? Seg_Buf[i] : 10;
        }
    }
}

/* LED处理函数 */
void Led_Proc()
{
    uchar i;
    if (3 * (Led_level) > Pwm_value + 1)
    {
        if (Flow_flag)
        {
            switch (Led_show_mode)
            {
            case 0:
                for (i = 0; i < 8; i++)
                    ucLed[i] = (i == Led_show_index);
                break;
            case 1:
                for (i = 0; i < 8; i++)
                    ucLed[i] = (7 - i == Led_show_index);
                break;
            case 2:
                for (i = 0; i < 4; i++)
                {
                    ucLed[i] = (i == Led_show_index);
                    ucLed[7 - i] = (i == Led_show_index);
                }
                break;
            case 3:
                for (i = 0; i < 4; i++)
                {
                    ucLed[3 - i] = (i == Led_show_index);
                    ucLed[4 + i] = (i == Led_show_index);
                }
                break;
            }
        }
        else
        {
            memset(ucLed, 0, 8);
        }
    }
    else
    {
        memset(ucLed, 0, 8);
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
    if (++Seg_Slow_Down == 200)
        Seg_Slow_Down = 0;
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
    if (++Pwm_value == 12)
        Pwm_value = 0;
    if (++time_800ms == 800)
    {
        Seg_flicker ^= 1;
        time_800ms = 0;
    }
    if (Flow_flag)
    {
        if (++time_flow >= Flow_interval)
        {
            time_flow = 0;
            Led_show_index = (++Led_show_index) % 8;
        }
    }
    else
    {
        time_flow = 0;
        Led_show_index = 0;
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