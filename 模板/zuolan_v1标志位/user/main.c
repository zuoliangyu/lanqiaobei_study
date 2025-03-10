#include "main.h"
/* LED与数码管 */
unsigned char ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char Seg_Pos;
unsigned char Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10};
unsigned char Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 串口数据*/
unsigned char Uart_Buf[10];
unsigned char Uart_Rx_Index;
bit Uart_flag;
unsigned char Sys_Tick;
/* 时间*/
unsigned char ucRtc[3] = {11, 11, 11};
unsigned int time_all_1s;

/* 数据 */
unsigned int Freq;

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
/* 键盘处理*/
void Key_Proc()
{
  static unsigned char Key_Val, Key_Down, Key_Up, Key_Old;
  if (time_all_1s % 10)
    return;
  Key_Val = Key_Read();
  Key_Down = Key_Val & (Key_Old ^ Key_Val);
  Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
  Key_Old = Key_Val;
}

/* 数码管处理*/
void Seg_Proc()
{
  if (time_all_1s % 20)
    return;
}
void Led_Proc() {}
void Uart_Proc()
{
  if (Uart_Rx_Index == 0)
    return;
  if (Sys_Tick >= 10)
  {
    Sys_Tick = Uart_flag = 0;

    memset(Uart_Buf, 0, Uart_Rx_Index);
    Uart_Rx_Index = 0;
  }
}
void Timer0_Init(void) // 1毫秒@12.000MHz
{
  AUXR &= 0x7F; // 定时器时钟12T模式
  TMOD &= 0xF0; // 设置定时器模式
  TMOD |= 0x05;
  TL0 = 0; // 设置定时初始值
  TH0 = 0; // 设置定时初始值
  TF0 = 0; // 清除TF0标志
  TR0 = 1; // 定时器0开始计时
  EA = 1;
}

void Timer1_Init(void) // 1毫秒@12.000MHz
{
  AUXR &= 0xBF; // 定时器时钟12T模式
  TMOD &= 0x0F; // 设置定时器模式
  TL1 = 0x18;   // 设置定时初始值
  TH1 = 0xFC;   // 设置定时初始值
  TF1 = 0;      // 清除TF1标志
  TR1 = 1;      // 定时器1开始计时
  ET1 = 1;      // 使能定时器1中断
  EA = 1;
}

void Timer1_Isr(void) interrupt 3
{
  unsigned char i;
  if (++time_all_1s == 1000)
  {
    time_all_1s = 0;
    Freq = TH0 << 8 | TL0;
    TH0 = TL0 = 0;
  }
  if (Uart_flag)
    Sys_Tick++;
  Seg_Pos = (++Seg_Pos) % 8;
  Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
  for (i = 0; i < 8; i++)
    Led_Disp(i, ucLed[i]);
}
void Uart1_Isr(void) interrupt 4
{
  if (RI)
  {
    Uart_flag = 1;
    Sys_Tick = 0;
    Uart_Buf[Uart_Rx_Index] = SBUF;
    Uart_Rx_Index++;
    RI = 0;
  }
  if (Uart_Rx_Index > 10)
    Uart_Rx_Index = 0;
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
  Uart1_Init();
  Timer1_Init();
  Set_Rtc(ucRtc);
  rd_temperature();
  Delay750ms();
  while (1)
  {
    Data_Proc();
    Key_Proc();
    Seg_Proc();
    Uart_Proc();
    Led_Proc();
  }
}