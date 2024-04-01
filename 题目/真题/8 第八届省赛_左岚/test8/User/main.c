#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Slow_Down;                                 // 数码管减速
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                       // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};       // 某位是否显示小数点

/* 时间方面 */
uchar ucRtc[3] = {0x23, 0x59, 0x50};    // 初始化时钟时间23:59:50
uchar alarmRtc[3] = {0x00, 0x00, 0x00}; // 初始化闹钟时间00:00:00
uchar setUcRtc[3];                      // 时间设置数组 十进制
uchar setAlarmRtc[3];                   // 闹钟设置数组 十进制
uchar setUcRtc_index;
uchar setAlarmRtc_index;
/* 键盘方面 */
uchar Key_Slow_Down;

/* 时间方面 */
uint time_all_1s;
uint time_blink_1s; // 1s闪烁
uchar time_200ms;
uint time_5s;
/* 数据方面 */
uchar T_value; // 温度值
/* 显示方面 */
uchar Seg_show_mode; // 0 时钟显示 1 时钟设置 2 闹钟设置 3 温度显示
/* 判断 */
bit Seg_blink_flag; // 数码管闪烁标志
bit Led_blink_flag; // LED闪烁标志
bit ring_flag;      // 闹钟触发标志
/* 数据处理函数 */
void Data_Proc()
{
    if (time_all_1s % 500 == 0)
    {
        // 时间读取
        Read_Rtc(ucRtc);
        if (ucRtc[0] == alarmRtc[0] && ucRtc[1] == alarmRtc[1] && ucRtc[2] == alarmRtc[2])
            // 闹钟触发
            ring_flag = 1;
    }
    if (time_all_1s % 500 == 0)
    {
        // 温度读取
        T_value = rd_temperature();
    }
}
/* 键盘处理函数 */
void Key_Proc()
{
    static uchar Key_Val, Key_Down, Key_Up, Key_Old;
    uchar i;
    if (time_all_1s % 10)
        return;
    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;
    switch (Seg_show_mode)
    {
    case 0:
        if (Key_Down == 7)
        {
            // 进入时钟设置
            for (i = 0; i < 3; i++)
            {
                setUcRtc[i] = ucRtc[i] / 16 * 10 + ucRtc[i] % 16;
            }
            Seg_show_mode = 1;
        }
        else if (Key_Down == 6)
        {
            // 进入闹钟设置
            for (i = 0; i < 3; i++)
            {
                setAlarmRtc[i] = alarmRtc[i] / 16 * 10 + alarmRtc[i] % 16;
            }
            Seg_show_mode = 2;
        }
        if (Key_Old == 4)
            // 进入温度显示
            Seg_show_mode = 3;
        break;

    case 1:
        if (Key_Down == 7)
            setUcRtc_index++;
        if (setUcRtc_index >= 3)
        {
            // 返回时钟显示
            Seg_show_mode = 0;
            setUcRtc_index = 0;
            for (i = 0; i < 3; i++)
                ucRtc[i] = setUcRtc[i] / 10 << 4 | setUcRtc[i] % 10;
            Set_Rtc(ucRtc);
        }
        if (Key_Down == 5)
        {
            if (setUcRtc_index == 0)
                // 修改小时
                setUcRtc[setUcRtc_index] = (setUcRtc[setUcRtc_index] >= 24) ? 24
                                                                            : setUcRtc[setUcRtc_index] + 1;
            else
                // 修改分秒
                setUcRtc[setUcRtc_index] = (setUcRtc[setUcRtc_index] >= 60) ? 60
                                                                            : setUcRtc[setUcRtc_index] + 1;
        }
        else if (Key_Down == 4)
            // 修改时分秒
            setUcRtc[setUcRtc_index] = (setUcRtc[setUcRtc_index] == 0) ? 0
                                                                       : setUcRtc[setUcRtc_index] - 1;
        break;
    case 2:
        if (Key_Down == 6)
            setAlarmRtc_index++;
        if (setAlarmRtc_index >= 3)
        {
            // 返回时钟显示
            Seg_show_mode = 0;
            setAlarmRtc_index = 0;
            for (i = 0; i < 3; i++)
                alarmRtc[i] = setAlarmRtc[i] / 10 << 4 | setAlarmRtc[i] % 10;
        }
        if (Key_Down == 5)
        {
            if (setAlarmRtc_index == 0)
                // 修改小时
                setAlarmRtc[setAlarmRtc_index] = (setAlarmRtc[setAlarmRtc_index] >= 24) ? 24
                                                                                        : setAlarmRtc[setAlarmRtc_index] + 1;
            else
                // 修改分秒
                setAlarmRtc[setUcRtc_index] = (setAlarmRtc[setUcRtc_index] >= 60) ? 60
                                                                                  : setAlarmRtc[setAlarmRtc_index] + 1;
        }
        else if (Key_Down == 4)
            // 修改时分秒
            setAlarmRtc[setUcRtc_index] = (setAlarmRtc[setAlarmRtc_index] == 0) ? 0
                                                                                : setAlarmRtc[setAlarmRtc_index] - 1;
        break;
    case 3:
        if (Key_Up == 4)
            // 返回时钟显示
            Seg_show_mode = 0;
        break;
    }
    if (ring_flag && (Key_Down != 0))
        ring_flag = 0;
}
/* 数码管处理函数 */
void Seg_Proc()
{
    if (time_all_1s % 20)
        return;
    switch (Seg_show_mode)
    {
    case 0:
        /* 时钟显示 */
        Seg_Buf[0] = ucRtc[0] / 16;
        Seg_Buf[1] = ucRtc[0] % 16;
        Seg_Buf[2] = 11; //-
        Seg_Buf[3] = ucRtc[1] / 16;
        Seg_Buf[4] = ucRtc[1] % 16;
        Seg_Buf[5] = 11; //-
        Seg_Buf[6] = ucRtc[2] / 16;
        Seg_Buf[7] = ucRtc[2] % 16;
        break;

    case 1:
        /* 时钟设置 */
        Seg_Buf[0] = setUcRtc[0] / 10;
        Seg_Buf[1] = setUcRtc[0] % 10;
        Seg_Buf[2] = 11; //-
        Seg_Buf[3] = setUcRtc[1] / 10;
        Seg_Buf[4] = setUcRtc[1] % 10;
        Seg_Buf[5] = 11; //-
        Seg_Buf[6] = setUcRtc[2] / 10;
        Seg_Buf[7] = setUcRtc[2] % 10;
        Seg_Buf[setUcRtc_index * 3] = Seg_blink_flag ? 10
                                                     : Seg_Buf[setUcRtc_index * 3];
        Seg_Buf[setUcRtc_index * 3 + 1] = Seg_blink_flag ? 10
                                                         : Seg_Buf[setUcRtc_index * 3 + 1];
        break;
    case 2:
        /* 闹钟设置 */
        Seg_Buf[0] = setAlarmRtc[0] / 10;
        Seg_Buf[1] = setAlarmRtc[0] % 10;
        Seg_Buf[2] = 11; //-
        Seg_Buf[3] = setAlarmRtc[1] / 10;
        Seg_Buf[4] = setAlarmRtc[1] % 10;
        Seg_Buf[5] = 11; //-
        Seg_Buf[6] = setAlarmRtc[2] / 10;
        Seg_Buf[7] = setAlarmRtc[2] % 10;
        Seg_Buf[setAlarmRtc_index * 3] = Seg_blink_flag ? 10
                                                        : Seg_Buf[setAlarmRtc_index * 3];
        Seg_Buf[setAlarmRtc_index * 3 + 1] = Seg_blink_flag ? 10
                                                            : Seg_Buf[setAlarmRtc_index * 3 + 1];
        break;
        break;
    case 3:
        /* 温度显示 */
        memset(Seg_Buf, 10, 5);
        Seg_Buf[5] = T_value / 10;
        Seg_Buf[6] = T_value % 10;
        Seg_Buf[7] = 12; // C
        break;
    }
}

/* LED处理函数 */
void Led_Proc()
{
    ucLed[0] = Led_blink_flag;
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
    if (Seg_show_mode == 1 || Seg_show_mode == 2)
    {
        // 当进入设置界面的时候，我们开始进行计时
        if (++time_blink_1s == 1000)
        {
            // 1s闪烁
            time_blink_1s = 0;
            Seg_blink_flag ^= 1;
        }
    }
    else
    {
        // 不在设置界面的时候，我们不进行计时
        time_blink_1s = 0;
        Seg_blink_flag = 0;
    }
    if (ring_flag)
    {
        // 当闹钟触发的时候，我们开始计时
        if (++time_200ms == 200)
            Led_blink_flag ^= 1;
        if (++time_5s == 5000)
            ring_flag = 0;
    }
    else
    {
        // 防止因为按下按键后第二次计时的bug
        Led_blink_flag = 0;
        time_5s = 0;
        time_200ms = 0;
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
    rd_temperature();
    Delay750ms();
    while (1)
    {
        Data_Proc();
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}