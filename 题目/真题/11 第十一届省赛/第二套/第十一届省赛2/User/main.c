#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Slow_Down;                                // 数码管减速
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                      // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // 某位是否显示小数点

/* 键盘方面 */
uchar Key_Slow_Down;

/* 显示 */
uchar Seg_show_mode; // 0 数据显示 1 参数设置
/* 数据 */
uchar T_value;                          // 温度测量
uchar T_para_max = 30, T_para_min = 20; // 温度参数
uchar T_para_max_set, T_para_min_set;   // 温度参数设置
/* 判断 */
bit para_mode;      // 0 选择温度下限 1选择温度上限
bit error_data_set; // 错误数据设置
#define N 10
uint data_array[N]; // 窗口大小
uint sum_temp;      // 总和
uchar index_temp;   // 计数
uchar arr_count;    // 数组数据数量

uint filter(uint new_data)
{
    sum_temp -= data_array[index_temp];
    data_array[index_temp] = new_data;
    sum_temp += data_array[index_temp];
    index_temp = (++index_temp) % N;                    // 保证index_temp在0~N-1之间轮转
    arr_count = (++arr_count == N + 1) ? N : arr_count; // 锁定数组中的元素个数
    return sum_temp / arr_count;
}

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
    {
        Seg_show_mode = (++Seg_show_mode) % 2;
        para_mode = 0;
        if (Seg_show_mode == 1)
        {
            // 将数据值给设置值
            T_para_max_set = T_para_max;
            T_para_min_set = T_para_min;
        }
        if (Seg_show_mode == 0)
        {
            // 进行参数合理判断
            if (T_para_max_set >= T_para_min_set)
            {
                error_data_set = 0;
                T_para_max = T_para_max_set;
                T_para_min = T_para_min_set;
            }
            else
            {
                error_data_set = 1;
            }
        }
    }
    if (Key_Down == 5)
        para_mode ^= 1;
    if (Seg_show_mode == 1)
    {
        if (para_mode == 0)
        {
            // 设置温度下限
            if (Key_Down == 6)
                T_para_min_set = (++T_para_min_set) % 100;
            if (Key_Down == 7)
                T_para_min_set = (T_para_min_set == 0) ? 99 : T_para_min_set - 1;
        }
        else
        {
            // 设置温度上限
            if (Key_Down == 6)
                T_para_max_set = (++T_para_max_set) % 100;
            if (Key_Down == 7)
                T_para_max_set = (T_para_max_set == 0) ? 99 : T_para_max_set - 1;
        }
    }
}
/* 数码管处理函数 */
void Seg_Proc()
{
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    T_value = filter(rd_temperature());
    if (T_value > T_para_max)
        Da_Write(4 * 51);
    else if (T_value < T_para_min)
        Da_Write(2 * 51);
    else
        Da_Write(3 * 51);
    switch (Seg_show_mode)
    {
    case 0:
        /* 数据显示 */
        Seg_Buf[0] = 11; // C
        memset(Seg_Buf + 1, 10, 5);
        Seg_Buf[6] = T_value / 10 % 10;
        Seg_Buf[7] = T_value % 10;
        break;

    case 1:
        /* 参数设置 */
        Seg_Buf[0] = 12; // P
        Seg_Buf[1] = Seg_Buf[2] = 10;
        Seg_Buf[3] = T_para_max_set / 10 % 10;
        Seg_Buf[4] = T_para_max_set % 10;
        Seg_Buf[5] = 10;
        Seg_Buf[6] = T_para_min_set / 10 % 10;
        Seg_Buf[7] = T_para_min_set % 10;
        break;
    }
}

/* LED处理函数 */
void Led_Proc()
{
    ucLed[0] = (T_value > T_para_max);
    ucLed[1] = (T_value < T_para_min);
    ucLed[2] = (T_value < T_para_max && T_value > T_para_min);
    ucLed[3] = error_data_set;
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
    rd_temperature();
    Delay750ms();
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}