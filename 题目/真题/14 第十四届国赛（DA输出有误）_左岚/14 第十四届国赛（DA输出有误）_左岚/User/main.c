#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */                                     // 数码管减速
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                       // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};       // 某位是否显示小数点
/* 数据 */
uint T_value_10x;    // 温度测量值10倍
uint Dis_value;      // 默认为cm
uchar Para_Dis;      // 距离参数
uchar Para_T;        // 温度参数
char Dis_Cail;       // 距离校准值
uint Speed;          // 介质速度
uchar DAC_limit_10x; // 10倍dac下限
uint Record_Data[12];
uchar Record_Data_Index;
/* 显示 */
uchar Seg_show_mode;  // 0 测距 1 参数 2 工厂
uchar Dis_show_mode;  // 0 cm 1 m
uchar Para_show_mode; // 0 距离 1 温度
uchar FAC_show_mode;  // 0 校准 1 介质 2 DAC下限
/* 时间 */
uint time_all_1s;
uint time_6s;
uint time_2s;
uchar time_100ms;
uchar time_500ms;
/* 判断 */
bit Record_flag;        // 正在记录的标志
bit Key_lock;           // 键盘锁住
bit Key_Two_Press;      // 双按键按下
bit Led_Blink_flag;     // LED闪烁标志
bit Output_Record_flag; // 输出记录数据
void Recover_Sys()
{
    Seg_show_mode = 0;
    Para_Dis = 40;
    Para_T = 30;
    Dis_Cail = 0;
    Speed = 340;
    DAC_limit_10x = 10;
}
/* 数据处理函数 */
void Data_Proc()
{
    uchar DAC_temp_100x;
    if (time_all_1s % 500 == 0)
    {
        // 测距读取
        Dis_value = Ut_Wave_Data(Dis_Cail, Speed);
        if (Record_flag)
        {
            if (Dis_value < 10)
                DAC_temp_100x = DAC_limit_10x * 10;
            else if (Dis_value >= 90)
                DAC_temp_100x = 500;
            else
                DAC_temp_100x = (5 - DAC_limit_10x) * (Dis_value - 90) * 100 / 80 + 500;
            Record_Data[Record_Data_Index] = DAC_temp_100x;
            Record_Data_Index = (++Record_Data_Index) % 12;
        }
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
    // 正在记录，直接让所有动作失效
    if (Record_flag)
        return;
    if (time_all_1s % 10)
        return;
    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;
    if (Key_Old == 89)
    {
        // 现在开始长按
        Key_lock = 1;
        Key_Two_Press = 1;
        if (time_2s > 2000)
            Recover_Sys();
    }
    // 锁键盘，避免误操作
    if (Key_lock && Key_Old)
        return;
    Key_lock = 0;
    if (Key_Down == 4)
    {
        Seg_show_mode = (++Seg_show_mode) % 3;
        Dis_show_mode = FAC_show_mode = Para_show_mode = 0;
    }
    switch (Seg_show_mode)
    {
    case 0:
        /* 测距 */
        if (Key_Down == 5)
            Dis_show_mode = (++Dis_show_mode) % 2;
        if (Key_Down == 8)
            Record_flag = 1;
        if (Key_Down == 9)
        {
            Output_Record_flag = 1;
            Record_Data_Index = 11;
        }
        break;
    case 1:
        /* 参数 */
        if (Key_Down == 5)
            Para_show_mode = (++Para_show_mode) % 2;
        switch (Para_show_mode)
        {
        case 0:
            /* 距离 */
            if (Key_Down == 8)
                Para_Dis = (Para_Dis == 90) ? 10
                                            : Para_Dis + 10;
            else if (Key_Down == 9)
                Para_Dis = (Para_Dis == 10) ? 90
                                            : Para_Dis - 10;
            break;

        case 1:
            /* 温度 */
            if (Key_Down == 8)
                Para_T = (Para_T == 80) ? 0
                                        : Para_T + 1;
            else if (Key_Down == 9)
                Para_T = (Para_T == 0) ? 80
                                       : Para_T - 1;
            break;
        }
        break;
    case 2:
        /* 工厂 */
        if (Key_Down == 5)
            FAC_show_mode = (++FAC_show_mode) % 3;
        switch (FAC_show_mode)
        {
        case 0:
            /* 校准值 */
            if (Key_Down == 8)
                Dis_Cail = (Dis_Cail == 90) ? -90
                                            : Dis_Cail + 5;
            else if (Key_Down == 9)
                Dis_Cail = (Dis_Cail == -90) ? 90
                                             : Dis_Cail - 5;
            break;
        case 1:
            /* 介质 */
            if (Key_Down == 8)
                Speed = (Speed == 9990) ? 0
                                        : Speed + 10;
            else if (Key_Down == 9)
                Speed = (Speed == 10) ? 9990
                                      : Speed - 10;
            break;
        case 2:
            /* DAC下限 */
            if (Key_Down == 8)
                DAC_limit_10x = (DAC_limit_10x == 20) ? 1
                                                      : DAC_limit_10x + 1;
            else if (Key_Down == 9)
                DAC_limit_10x = (DAC_limit_10x == 1) ? 20
                                                     : DAC_limit_10x - 1;
            break;
        }
        break;
    }
}
/* 数码管处理函数 */
void Seg_Proc()
{
    uchar Dis_Cail_Temp;
    Dis_Cail_Temp = -Dis_Cail;
    if (time_all_1s % 20)
        return;
    switch (Seg_show_mode)
    {
    case 0:
        /* 测距 */
        Seg_Point[1] = 1;
        Seg_Point[6] = 0;
        Seg_Buf[0] = T_value_10x / 100 % 10;
        Seg_Buf[1] = T_value_10x / 10 % 10;
        Seg_Buf[2] = T_value_10x % 10;
        Seg_Buf[3] = 11; //-
        if (Dis_show_mode == 0)
        {
            Seg_Point[5] = 0;
            // 显示cm
            Seg_Buf[4] = (Dis_value / 1000 == 0) ? 10
                                                 : Dis_value / 1000;
            Seg_Buf[5] = (Seg_Buf[4] == 10 && Dis_value / 100 % 10 == 0) ? 10
                                                                         : Dis_value / 100 % 10;
            Seg_Buf[6] = (Seg_Buf[5] == 10 && Dis_value / 10 % 10 == 0) ? 10
                                                                        : Dis_value / 10 % 10;
            Seg_Buf[7] = Dis_value % 10;
        }
        else
        {
            Seg_Point[5] = 1;
            // 显示m
            Seg_Buf[4] = (Dis_value < 1000) ? 10
                                            : Dis_value / 1000;
            Seg_Buf[5] = Dis_value / 100 % 10;
            Seg_Buf[6] = Dis_value / 10 % 10;
            Seg_Buf[7] = Dis_value % 10;
        }
        break;

    case 1:
        Seg_Point[1] = Seg_Point[5] = Seg_Point[6] = 0;
        Seg_Buf[0] = 12; // P
        Seg_Buf[1] = Para_show_mode + 1;
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
        Seg_Buf[6] = (Para_show_mode == 0) ? Para_Dis / 10
                                           : Para_T / 10;
        Seg_Buf[7] = (Para_show_mode == 0) ? Para_Dis % 10
                                           : Para_T % 10;

        break;
    case 2:
        Seg_Buf[0] = 13; // F
        Seg_Buf[1] = FAC_show_mode + 1;
        switch (FAC_show_mode)
        {
        case 0:
            Seg_Point[1] = Seg_Point[5] = Seg_Point[6] = 0;
            /* 校准 */
            Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
            if (Dis_Cail <= -10)
            {
                Seg_Buf[5] = 11; //-
                Seg_Buf[6] = Dis_Cail_Temp / 10 % 10;
                Seg_Buf[7] = Dis_Cail_Temp % 10;
            }
            else if (Dis_Cail < 0)
            {
                Seg_Buf[5] = 10;
                Seg_Buf[6] = 11; //-
                Seg_Buf[7] = Dis_Cail_Temp % 10;
            }
            else
            {
                Seg_Buf[5] = 50;
                Seg_Buf[6] = (Dis_Cail / 10 % 10 == 0) ? 10
                                                       : Dis_Cail / 10 % 10;
                Seg_Buf[7] = Dis_Cail % 10;
            }
            break;

        case 1:
            Seg_Point[1] = Seg_Point[5] = Seg_Point[6] = 0;
            /* 介质 */
            Seg_Buf[2] = Seg_Buf[3] = 10;
            Seg_Buf[4] = (Speed / 1000 % 10 == 0) ? 10
                                                  : Speed / 1000 % 10;
            Seg_Buf[5] = (Seg_Buf[4] == 10 && Speed / 100 % 10 == 0) ? 10
                                                                     : Speed / 100 % 10;
            Seg_Buf[6] = Speed / 10 % 10;
            Seg_Buf[7] = Speed % 10;
            break;
        case 2:
            Seg_Point[1] = Seg_Point[5] = 0;
            Seg_Point[6] = 1;
            /* DAC下限 */
            Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
            Seg_Buf[6] = DAC_limit_10x / 10 % 10;
            Seg_Buf[7] = DAC_limit_10x % 10;
            break;
        }
        break;
    }
}

/* LED处理函数 */
void Led_Proc()
{
    uchar i;
    if (Output_Record_flag)
        Da_Write(Record_Data[11 - Record_Data_Index] * 51 / 100);
    switch (Seg_show_mode)
    {
    case 0:
        /* 测距 */
        if (Dis_value > 255)
            memset(ucLed, 1, 8);
        else
        {
            for (i = 0; i < 8; i++)
            {
                ucLed[i] = Dis_value & (0x01 << i);
            }
        }
        break;

    case 1:
        /*  参数 */
        memset(ucLed, 0, 7);
        ucLed[7] = 1;
        break;

    case 2:
        /* 工厂 */
        memset(ucLed + 1, 0, 7);
        ucLed[0] = Led_Blink_flag;
        break;
    }
    Relay(Dis_value <= Para_Dis + 5 &&
          Dis_value >= Para_Dis - 5 &&
          T_value_10x < Para_T * 10);
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
    uchar i;
    if (++time_all_1s == 1000)
        time_all_1s = 0;
    Seg_Pos = (++Seg_Pos) % 8;
    if (Output_Record_flag)
    {
        if (Record_Data_Index == 0)
        {
            Output_Record_flag = 0;
            time_500ms = 0;
        }
        // 输出计时
        if (++time_500ms == 500)
        {
            Record_Data_Index--;
            time_500ms = 0;
        }
    }
    else
    {
        time_500ms = 0;
    }
    // 当双按键按下的时候开始计时
    if (Key_Two_Press)
    {
        if (++time_2s >= 2000)
            time_2s = 2001;
    }
    else
    {
        time_2s = 0;
    }
    // 开始记录
    if (Record_flag)
    {
        if (++time_6s == 6000)
        {
            time_6s = 0;
            Record_flag = 0;
        }
    }
    else
    {
        time_6s = 0;
    }
    if (++time_100ms == 100)
    {
        time_100ms = 0;
        Led_Blink_flag ^= 1;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    for (i = 0; i < 8; i++)
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
    Recover_Sys();
    rd_temperature();
    Delay750ms();
    Timer0_Init();
    while (1)
    {
        Data_Proc();
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}