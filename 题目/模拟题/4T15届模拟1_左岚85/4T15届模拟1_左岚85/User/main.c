#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Slow_Down;                                // 数码管减速
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                      // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // 某位是否显示小数点

/* 时间方面 */
uchar ucRtc[3] = {0x13, 0x11, 0x11}; // 初始化时间13:11:11
uchar input_time[3];                 // 输入数据的时间
/* 键盘方面 */
uchar Key_Slow_Down;

/* 显示 */
uchar Seg_show_mode; // 0 时间 1 输入 2 记录

/* 数据 */
uchar input_data_arr[4]; // 输入的四位数据
uchar input_data_index;  // 输入数据的索引
uint new_value;
uint old_value;
uchar EEPROM_string_new[4];
uchar EEPROM_string_old[4];
bit data_up_flag; // 数据上升标志
/* 键盘处理函数 */
void Key_Proc()
{
    static uchar Key_Val, Key_Down, Key_Up, Key_Old;
    uchar i;
    if (Key_Slow_Down)
        return;
    Key_Slow_Down = 1;

    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;
    if (Key_Down == 4)
    {
        // 在 输入界面按下切换，代表数据进行写入，我们这里将新的值计算出来
        if (Seg_show_mode == 1)
        {
            new_value = input_data_arr[0] * 1000 + input_data_arr[1] * 100 + input_data_arr[2] * 10 + input_data_arr[3];
            EEPROM_string_new[0] = input_time[0] / 16 * 10 + input_time[0] % 16;
            EEPROM_string_new[1] = input_time[1] / 16 * 10 + input_time[1] % 16;
            EEPROM_string_new[2] = new_value >> 8;
            EEPROM_string_new[3] = new_value & 0x00ff;
            EEPROM_Write(EEPROM_string_new, 0, 4);
            data_up_flag = (new_value > old_value);
            old_value = new_value;
        }
        Seg_show_mode = (++Seg_show_mode) % 3;
        for (i = 0; i < 4; i++)
            input_data_arr[i] = 10; // 直接对应灭，每次进行界面切换的时候就清空一下数组
        input_data_index = 0;       // 输入数据索引重置
    }
    if (Seg_show_mode == 1)
    {
        // 清空数据
        if (Key_Down == 5)
        {
            for (i = 0; i < 4; i++)
                input_data_arr[i] = 10; // 直接对应灭，每次进行界面切换的时候就清空一下数组
            input_data_index = 0;       // 输入数据索引重置
        }
        // 输入第一位数据了
        if (input_data_index == 1)
            Read_Rtc(input_time);
        // 当输入的数据不足四位的时候
        if (input_data_index < 4)
        {
            switch (Key_Down)
            {
            case 6:
                input_data_arr[input_data_index] = 0;
                input_data_index++;
                break;
            case 8:
                input_data_arr[input_data_index] = 7;
                input_data_index++;
                break;
            case 12:
                input_data_arr[input_data_index] = 8;
                input_data_index++;
                break;
            case 16:
                input_data_arr[input_data_index] = 9;
                input_data_index++;
                break;
            case 9:
                input_data_arr[input_data_index] = 4;
                input_data_index++;
                break;
            case 13:
                input_data_arr[input_data_index] = 5;
                input_data_index++;
                break;
            case 17:
                input_data_arr[input_data_index] = 6;
                input_data_index++;
                break;
            case 10:
                input_data_arr[input_data_index] = 1;
                input_data_index++;
                break;
            case 14:
                input_data_arr[input_data_index] = 2;
                input_data_index++;
                break;
            case 18:
                input_data_arr[input_data_index] = 3;
                input_data_index++;
                break;
            }
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
        /* 输入 */
        Seg_Buf[0] = 12; // C
        Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = 10;
        // 没有输入的时候直接是空的
        if (input_data_index == 0)
        {
            Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 10;
        }
        else
        {
            for (i = 0; i < input_data_index; i++)
            {
                Seg_Buf[7 - i] = input_data_arr[input_data_index - i - 1];
            }
        }
        break;
    case 2:
        /* 记录 */
        Seg_Buf[0] = 13; // E
        Seg_Buf[1] = Seg_Buf[2] = 10;
        Seg_Buf[3] = input_time[0] / 16;
        Seg_Buf[4] = input_time[0] % 16;
        Seg_Buf[5] = 11; //-
        Seg_Buf[6] = input_time[1] / 16;
        Seg_Buf[7] = input_time[1] % 16;
        break;
    }
}

/* LED处理函数 */
void Led_Proc()
{
    ucLed[0] = (Seg_show_mode == 0);
    ucLed[1] = (Seg_show_mode == 1);
    ucLed[2] = (Seg_show_mode == 2);

    ucLed[3] = data_up_flag;
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

void main()
{
    System_Init();
    Timer0_Init();
    Set_Rtc(ucRtc);
    EEPROM_Read(EEPROM_string_old, 0, 4);
    old_value = EEPROM_string_old[2] << 8 | EEPROM_string_old[3];
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}