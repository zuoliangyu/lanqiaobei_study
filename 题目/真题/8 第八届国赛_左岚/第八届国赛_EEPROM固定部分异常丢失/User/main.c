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

/* 时间方面 */
uint time_all_1s;
uchar time_200ms;
/* 数据 */
uchar Dis_Data[10];     // 存储数组
uchar Dis_show_index;   // 显示数组下标
uchar Dis_Data_index;   // 存储数组下标
uchar Dis_new, Dis_old; // 这一次和上一次测量的结果
uchar Blind_area;       // 测量盲区
uchar Led_blink_cnt;    // 闪烁计数
/* 显示 */
uchar Seg_show_mode; // 0 测距显示 1 回显 2 参数设置
/* 判断 */
bit Led_blink_start; // 是否开始闪烁
bit Led_blink_flag;  // 闪烁标志位
bit Work_mode;       // 0 上一次 1 这一次+上一次
/* 键盘处理函数 */
void Key_Proc()
{
    static uchar Key_Val, Key_Down, Key_Up, Key_Old;
    uint DA_out = 0;
    if (time_all_1s % 10)
        return;
    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;
    switch (Seg_show_mode)
    {
    case 0:
        /* 测距界面 */
        if (Key_Down == 4)
        {
            Led_blink_start = 1;
            Dis_new = Ut_Wave_Data();
            Dis_Data[Dis_Data_index] = Dis_new;
            Dis_Data_index = (++Dis_Data_index) % 10; // 0-9
            if (Dis_Data_index == 0)
            {
                // 这样写是为了防止溢出
                EEPROM_Write(&Dis_new, 10, 1);
                // 这里赋值是因为可能上一次断电后有数据保存，所以要用上次的数据
                Dis_old = Dis_Data[9];
            }
            else
            {
                EEPROM_Write(&Dis_new, Dis_Data_index, 1);
                // 这里赋值是因为可能上一次断电后有数据保存，所以要用上次的数据
                Dis_old = Dis_Data[Dis_Data_index - 1];
            }

            if (Dis_new < Blind_area)
                DA_out = 0;
            else
                DA_out = (Dis_new - Blind_area) * 51 * 0.02;
            if (DA_out >= 255)
                DA_out = 255;
            Da_Write(DA_out);
        }
        if (Key_Down == 7)
            Work_mode ^= 1;
        if (Key_Down == 5)
            // 切换回显
            Seg_show_mode = 1;
        if (Key_Down == 6)
            // 切换参数
            Seg_show_mode = 2;
        break;

    case 1:
        /* 回显界面 */
        if (Key_Down == 5)
            // 切换测距
            Seg_show_mode = 0;
        if (Key_Down == 7)
            // 翻页
            Dis_show_index = (++Dis_show_index) % 10;
        break;
    case 2:
        /* 参数设置界面 */
        if (Key_Down == 6)
        {
            // 切换测距
            Seg_show_mode = 0;
            EEPROM_Write(&Blind_area, 0, 1);
        }
        if (Key_Down == 7)
            // 循环添加
            Blind_area = (Blind_area == 90) ? 0 : Blind_area + 10;
        break;
    }
}
/* 数码管处理函数 */
void Seg_Proc()
{
    uint Work_data = 0;

    if (time_all_1s % 20)
        return;
    switch (Seg_show_mode)
    {
    case 0:
        /* 测距界面 */
        Seg_Buf[0] = Work_mode;
        Seg_Buf[1] = 10;
        if (Work_mode)
            Work_data = Dis_old + Dis_new;
        else
            Work_data = Dis_old;
        Seg_Buf[2] = (Work_data / 100 % 10 == 0) ? 10
                                                 : Work_data / 100 % 10;
        Seg_Buf[3] = (Work_data / 10 % 10 == 0 &&
                      Seg_Buf[2] == 10)
                         ? 10
                         : Work_data / 10 % 10;
        Seg_Buf[4] = Work_data % 10;
        Seg_Buf[5] = Dis_new / 100 % 10;
        Seg_Buf[6] = Dis_new / 10 % 10;
        Seg_Buf[7] = Dis_new % 10;
        break;

    case 1:
        /* 回显界面 */
        Seg_Buf[0] = (Dis_show_index + 1) / 10 % 10;
        Seg_Buf[1] = (Dis_show_index + 1) % 10;
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
        Seg_Buf[5] = Dis_Data[Dis_show_index] / 100 % 10;
        Seg_Buf[6] = Dis_Data[Dis_show_index] / 10 % 10;
        Seg_Buf[7] = Dis_Data[Dis_show_index] % 10;
        break;
    case 2:
        /* 参数设置 */
        Seg_Buf[0] = 11; // F
        Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
        Seg_Buf[5] = (Blind_area / 100 % 10 == 0) ? 10
                                                  : Blind_area / 100 % 10;
        Seg_Buf[6] = (Blind_area / 10 % 10 == 0 &&
                      Seg_Buf[5] == 10)
                         ? 10
                         : Blind_area / 10 % 10;
        Seg_Buf[7] = Blind_area % 10;
        break;
    }
}

/* LED处理函数 */
void Led_Proc()
{

    ucLed[0] = Led_blink_flag;
    ucLed[6] = (Seg_show_mode == 2);
    ucLed[7] = (Seg_show_mode == 1);
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
    if (Led_blink_start)
    {
        if (++time_200ms == 200)
        {
            time_200ms = 0;
            Led_blink_flag ^= 1;
            Led_blink_cnt++;
        }
        if (Led_blink_cnt == 20)
            Led_blink_start = 0; // 停止闪烁
    }
    else
    {
        time_200ms = 0;
        Led_blink_cnt = 0;
        Led_blink_flag = 0;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

void Delay200ms(void) //@12.000MHz
{
    unsigned char data i, j, k;

    _nop_();
    _nop_();
    i = 10;
    j = 31;
    k = 147;
    do
    {
        do
        {
            while (--k)
                ;
        } while (--j);
    } while (--i);
}

uchar passwd = 123;
uchar input_passwd;
void main()
{
    uchar i;
    System_Init();
    Timer0_Init();
    EEPROM_Read(&input_passwd,16,1);
    if (input_passwd != passwd) // 校验失败，之前未写入数据1/256概率出问题
    {
        EEPROM_Write(&passwd, 16, 1);
    }
    else // 校验通过，读取我们需要的数据
    {
        EEPROM_Read(Dis_Data, 1, 7);
        EEPROM_Read(&Dis_Data[8], 8, 3);
    }

    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}