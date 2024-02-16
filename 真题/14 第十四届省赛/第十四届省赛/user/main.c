#include "main.h"

/* 变量 */
/* 键盘方面 */
uchar Key_Slow_Down;

/* LED 方面*/
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};
/* 数码管方面 */
uint Seg_Slow_Down;
uchar Seg_Pos;
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10};
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 时间 */
uchar ucRtc[3] = {0x11, 0x59, 0x56};
uint time_1s;
uint time_2s;
uint time_3s;
uchar time_100ms;
/* 界面 */
uchar Seg_show_mode; // 0 时间 1 回显 2 参数
uchar re_show_mode;  // 0 温度 1 湿度 2 时间

/* 数据 */
uchar max_temperature;       // 最大温度
uint aver_temperature_10x;   // 平均温度
uint old_temperature_10x;    // 上一次的温度
uchar max_humidity;          // 最大湿度
uint aver_humidity_10x;      // 平均湿度
uint old_humidity_10x;       // 上一次的湿度
uchar trigger_count;         // 触发次数
uchar trigger_time[3];       // 触发时间
uchar para_temperature = 30; // 温度参数
uint freq;

/* 相关判断参数 */
uchar old_light_value;   // 上一次的光强参数
bit trigger_flag;        // 触发标志
bit long_press_flag;     // 长按标志
bit wring_flag;          // 温度超过参数报警
bit Led_light_flag;      // 闪烁
bit error_humidity_flag; // 湿度是否有效，无效为1
bit data_up;             // 温湿度均升高

/* 函数 */
/* 清空数据 */
void Clear_Data(void)
{
    uchar i;
    max_temperature = 0;      // 最大温度
    aver_temperature_10x = 0; // 平均温度
    max_humidity = 0;         // 最大湿度
    aver_humidity_10x = 0;    // 平均湿度
    trigger_count = 0;        // 触发次数
    for (i = 0; i < 3; i++)
        trigger_time[i] = 0; // 触发时间
}
/* 数码管复位 */
void Seg_Reset(void)
{
    uchar i;
    for (i = 0; i < 8; i++)
    {
        Seg_Buf[i] = 10;
        Seg_Point[i] = 0;
    }
}
/* 湿度计算 */
float rd_humidity(void)
{
    float humidity_value;
    if (freq < 200 || freq > 2000)
        return 0;
    humidity_value = (freq - 200) * 2 / 45 + 10;
    return humidity_value;
}

/* 键盘处理 */
void Key_Porc(void)
{
    static uchar Key_Val, Key_Down, Key_Old, Key_Up;
    if (Key_Slow_Down)
        return;
    Key_Slow_Down = 1;

    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;

    if (Key_Down == 4)
    {
        Seg_Reset();
        Seg_show_mode = (++Seg_show_mode) % 3;
        re_show_mode = 0;
    }
    if (Seg_show_mode == 1 && Key_Down == 5)
    {
        Seg_Reset();
        re_show_mode = (++re_show_mode) % 3;
    }
    if (Seg_show_mode == 2)
    {
        if (Key_Down == 8)
            para_temperature = (++para_temperature > 99) ? 99 : para_temperature;
        else if (Key_Down == 9)
            para_temperature = (--para_temperature < 0) ? 0 : para_temperature;
    }
    if (Seg_show_mode == 0)
    {
        if (Key_Down == 9)
        {
            long_press_flag = 1;
        }
        if (Key_Up == 9)
        {
            if (time_2s >= 2000)
                Clear_Data();
            long_press_flag = 0;
        }
    }
}

/* 数码管处理 */
void Seg_Porc(void)
{
    uint temp_temperature_10x, temp_humidity_10x;
    uchar temp_light;
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    temp_light = Ad_Read(0x01);
    // 避免重复触发
    if ((old_light_value > 50) &&
        (temp_light < 50) &&
        (trigger_flag == 0))
        trigger_flag = 1;

    // 被触发，并且到达3s，
    else if (trigger_flag == 1 && time_3s >= 3000)
        trigger_flag = 0;
    old_light_value = temp_light;
    if (trigger_flag)
    {
        Seg_Buf[1] = Seg_Buf[2] = 10;
        Seg_Point[6] = 0;
        /* 温湿度界面 */
        Seg_Buf[0] = 16; // E
        temp_temperature_10x = rd_temperature() * 10;
        temp_humidity_10x = rd_humidity() * 10;
        Seg_Buf[3] = temp_temperature_10x / 10 % 10;
        Seg_Buf[4] = temp_temperature_10x % 10;
        Seg_Buf[5] = 11; //-
        // 无效数据
        if (temp_humidity_10x == 0)
        {
            error_humidity_flag = 1;
            Seg_Buf[6] = 17; // A
            Seg_Buf[7] = 17; // A
        }
        // 有效数据
        else
        {
            error_humidity_flag = 0; // 数据有效
            wring_flag = (temp_temperature_10x > para_temperature * 10);
            if ((trigger_count >= 2) &&
                (temp_humidity_10x > old_humidity_10x) &&
                (temp_temperature_10x > old_temperature_10x))
                data_up = 1;
            else
                data_up = 0;
            old_humidity_10x = temp_humidity_10x;
            old_temperature_10x = temp_temperature_10x;
            Seg_Buf[6] = temp_humidity_10x / 10 % 10;
            Seg_Buf[7] = temp_humidity_10x % 10;
            if (++trigger_count == 100)
                trigger_count = 99;
            max_temperature = (max_temperature > temp_temperature_10x) ? max_temperature : temp_temperature_10x;
            max_humidity = (max_humidity > temp_humidity_10x) ? max_humidity : temp_humidity_10x;
            aver_temperature_10x = (aver_temperature_10x * (trigger_count - 1) + temp_temperature_10x) / (trigger_count);
            aver_humidity_10x = (aver_humidity_10x * (trigger_count - 1) + temp_humidity_10x) / (trigger_count);
        }
    }
    else
    {
        switch (Seg_show_mode)
        {
        case 0:
            /* 时间 */
            Read_Rtc(ucRtc);
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
            /* 回显 */
            switch (re_show_mode)
            {
            case 0:
                /* 温度 */
                Seg_Buf[0] = 12; // C
                if (trigger_count != 0)
                {
                    Seg_Buf[2] = max_temperature / 10 % 10;
                    Seg_Buf[3] = max_temperature % 10;
                    Seg_Buf[4] = 11; //-
                    Seg_Buf[5] = aver_temperature_10x / 100 % 10;
                    Seg_Buf[6] = aver_temperature_10x / 10 % 10;
                    Seg_Buf[7] = aver_temperature_10x % 10;
                    Seg_Point[6] = 1;
                }
                break;
            case 1:
                /* 湿度 */
                Seg_Buf[0] = 13; // H
                if (trigger_count != 0)
                {
                    Seg_Buf[2] = max_humidity / 10 % 10;
                    Seg_Buf[3] = max_humidity % 10;
                    Seg_Buf[4] = 11; //-
                    Seg_Buf[5] = aver_humidity_10x / 100 % 10;
                    Seg_Buf[6] = aver_humidity_10x / 10 % 10;
                    Seg_Buf[7] = aver_humidity_10x % 10;
                    Seg_Point[6] = 1;
                }
                break;
            case 2:
                /* 时间 */
                Seg_Buf[0] = 14; // F
                Seg_Buf[1] = trigger_count / 10 % 10;
                Seg_Buf[2] = trigger_count % 10;
                if (trigger_count != 0)
                {
                    Seg_Buf[3] = trigger_time[0] / 16;
                    Seg_Buf[4] = trigger_time[0] % 16;
                    Seg_Buf[5] = 11; //-
                    Seg_Buf[6] = trigger_time[1] / 16;
                    Seg_Buf[7] = trigger_time[1] % 16;
                }
                break;
            }
            break;
        case 2:
            /* 参数 */
            Seg_Buf[0] = 15; // P
            Seg_Buf[6] = para_temperature / 10 % 10;
            Seg_Buf[7] = para_temperature % 10;
            break;
        }
    }
}

/* LED处理 */
void Led_Porc(void)
{
    /* 界面 */
    ucLed[0] = (Seg_show_mode == 0);
    ucLed[1] = (Seg_show_mode == 1);
    ucLed[2] = (Seg_show_mode == 2);
    /* 报警 */
    ucLed[3] = (wring_flag) ? Led_light_flag : 0;
    ucLed[4] = error_humidity_flag;
    ucLed[5] = data_up;
}

/* 定时器/计数器部分 */
/* 计数器 */
void Timer0_Init(void) // 1毫秒@12.000MHz
{
    AUXR &= 0x7F; // 定时器时钟12T模式
    TMOD &= 0xF0; // 设置定时器模式
    TMOD |= 0x05; // 设置定时器模式
    TL0 = 0;      // 设置定时初始值
    TH0 = 0;      // 设置定时初始值
    TF0 = 0;      // 清除TF0标志
    TR0 = 1;      // 定时器0开始计时
}
/* 定时器 */
void Timer1_Init(void) // 1毫秒@12.000MHz
{
    AUXR &= 0xBF; // 定时器时钟12T模式
    TMOD &= 0x0F; // 设置定时器模式
    TL1 = 0x18;   // 设置定时初始值
    TH1 = 0xFC;   // 设置定时初始值
    TF1 = 0;      // 清除TF1标志
    TR1 = 1;      // 定时器1开始计时
    ET1 = 1;      // 允许定时器1中断
    EA = 1;       // 允许总中断
}
/* 定时器服务函数 */
void Timer1_ISR(void) interrupt 3
{
    if (++Key_Slow_Down == 10)
        Key_Slow_Down = 0;
    if (++Seg_Slow_Down == 200)
        Seg_Slow_Down = 0;
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
    if (++time_1s == 1000)
    {
        time_1s = 0;
        freq = TH0 << 8 | TL0;
        TH0 = 0;
        TL0 = 0;
    }
    if (trigger_flag)
    {
        if (++time_3s >= 3000)
        {
            time_3s = 3001;
        }
    }
    else
        time_3s = 0;
    // 开始长按计时
    if (long_press_flag)
    {
        if (++time_2s >= 2000)
        {
            time_2s = 2001;
        }
    }
    else
        time_2s = 0;
    // 处于报警

    if (++time_100ms == 100)
    {
        time_100ms = 0;
        Led_light_flag ^= 1;
    }

    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

void main()
{
    System_Init();
    Timer0_Init();
    Timer1_Init();
    Set_Rtc(ucRtc);
    while (1)
    {
        Key_Porc();
        Seg_Porc();
        Led_Porc();
    }
}