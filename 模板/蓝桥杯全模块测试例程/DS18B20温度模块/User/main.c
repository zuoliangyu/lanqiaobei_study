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
uint temperature_value_10x; // 温度数据
bit C_F_flag;               // 摄氏度转华氏度

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
        C_F_flag ^= 1;
}
/* 数码管处理函数 */
void Seg_Proc()
{
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    temperature_value_10x = filter(rd_temperature() * 10);
    if (C_F_flag)
        temperature_value_10x = temperature_value_10x * 1.8 + 320; // 华氏度
    Seg_Buf[0] = temperature_value_10x / 1000 % 10;
    Seg_Buf[1] = temperature_value_10x / 100 % 10;
    Seg_Buf[2] = temperature_value_10x / 10 % 10;
    Seg_Buf[3] = temperature_value_10x % 10;
    Seg_Buf[4] = (C_F_flag) ? 12 : 11;
    Seg_Point[2] = 1;
}

/* LED处理函数 */
void Led_Proc()
{
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