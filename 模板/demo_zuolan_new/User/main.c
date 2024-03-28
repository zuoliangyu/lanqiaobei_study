#include "main.h"
/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Slow_Down;                                // 数码管减速
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示的值
uchar Seg_Pos;                                      // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // 某位是否显示小数点

/* 串口方面 */
uchar Uart_Slow_Down;
uchar Uart_Buf[20];  // 串口接收到的数据
uchar Uart_Rx_Index; // 串口接收到的数据的指针

/* 时间方面 */
uchar ucRtc[3] = {0x13, 0x11, 0x11}; // 初始化时间13:11:11

/* 键盘方面 */
uchar Key_Slow_Down;

/* 时间方面 */
uint time_all_1s;
/* 数据 */
uint int_data; // 整数数据

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
/* 数据处理函数 */
void Data_Proc()
{
    if (time_all_1s % 50 == 0)
    {
        // 时间读取
    }
    if (time_all_1s % 100 == 0)
    {
        // AD读取
    }
    if (time_all_1s % 500 == 0)
    {
        // 温度读取
    }
}
/* 键盘处理函数 */
void Key_Proc()
{
    static uchar Key_Val, Key_Down, Key_Up, Key_Old;
    if (time_all_1s % 10)
        return;
    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;
    if (Key_Down == 4)
        EEPROM_Write(&int_data, 0, 2);
}
/* 数码管处理函数 */
void Seg_Proc()
{
    if (time_all_1s % 20)
        return;
}

/* LED处理函数 */
void Led_Proc()
{
}

/* 串口处理函数 */
void Uart_Proc()
{
    if (time_all_1s % 200)
        return;
    if (Uart_Buf[0] == 'O' && Uart_Buf[1] == 'K')
    {
        // 执行相关函数

        memset(Uart_Buf, 0, 20);
        Uart_Rx_Index = 0;
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
    if (++time_all_1s == 1000)
        time_all_1s = 0;
    if (++Key_Slow_Down == 10)
        Key_Slow_Down = 0;
    if (++Seg_Slow_Down == 500)
        Seg_Slow_Down = 0;
    if (++Uart_Slow_Down == 200)
        Uart_Slow_Down = 0;
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

/* 串口中断服务函数 */
void Uart_ISR(void) interrupt 4
{
    if (RI == 1) // 串口接收到数据
    {
        Uart_Buf[Uart_Rx_Index] = SBUF;
        Uart_Rx_Index++;
        RI = 0;
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
uchar passwd = 123;
uchar input_passwd;
void main()
{

    Timer0_Init();
    Uart1_Init();
    Set_Rtc(ucRtc);
    rd_temperature();
    Delay750ms();
    EEPROM_Read(&input_passwd, 8, 1); // 用不会写入的地方做校验
    if (input_passwd != passwd)       // 校验失败，之前未写入数据1/256概率出问题
    {
        EEPROM_Write(&passwd, 8, 1);
    }
    else // 校验通过，读取我们需要的数据
    {
        EEPROM_Read(&int_data, 0, 2);
    }

    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Uart_Proc();
        Led_Proc();
    }
}