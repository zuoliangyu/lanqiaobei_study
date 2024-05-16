#include "main.h"

/* Led与Seg相关 */
unsigned char ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10};
unsigned char Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char Seg_Pos;
/* 时间 */
unsigned int Time_1s;
unsigned char ucRtc[3] = {0x00, 0x00, 0x00};
unsigned char MAX_Rtc[3];
unsigned char Time_200ms_Freq;   // 测频界面闪烁时间200ms
unsigned char Time_200ms_Wring;  // 超过阈值闪烁时间200ms
/* 数据 */
unsigned int Freq;               // 测量频率
unsigned int Freq_CAL;           // 测量频率校准值
unsigned int Freq_MAX;           // 最大频率
unsigned int Para_Limit = 2000;  // 超限参数  1000 - 9000
int Para_Right = 0;              // 校准参数 -900 - 900
float Da_Out_Amp;                // 输出电压模拟值
unsigned char Da_Out_Dig;        // 输出电压数字值
/* 显示 */
unsigned char Seg_Show_Mode;  // 0 频率界面 1 参数界面 2 时间界面 3 回显界面
unsigned char Para_Mode;     // 0 超限参数界面 1 校准值参数界面
unsigned char Re_Show_Mode;  // 0 频率回显 1 时间回显
/* 判断 */
bit Freq_Error;            // 小于校准参数
bit Led_Blink_Flag_Freq;   // 频率测量界面闪烁标志
bit Led_Blink_Flag_Wring;  // 超出阈值闪烁标志
bit Freq_Wring;            // 超过超限参数

void Key_Proc() {
  static unsigned char Key_Val, Key_Old, Key_Down, Key_Up;
  if (Time_1s % 10) return;
  Key_Val = Key_Scan();
  Key_Down = Key_Val & (Key_Old ^ Key_Val);
  Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
  Key_Old = Key_Val;
  if (Key_Down == 4) {
    Seg_Show_Mode = (++Seg_Show_Mode) % 4;
    Para_Mode = Re_Show_Mode = 0;
  }
  switch (Seg_Show_Mode) {
    case 1:
      /* 参数页面 */
      if (Key_Down == 5) Para_Mode = (++Para_Mode) % 2;
      switch (Para_Mode) {
        case 0:
          /* 超限参数 */
          if (Key_Down == 8)
            Para_Limit = (Para_Limit == 9000) ? 1000 : Para_Limit + 1000;
          else if (Key_Down == 9)
            Para_Limit = (Para_Limit == 1000) ? 9000 : Para_Limit - 1000;
          break;
        case 1:
          /* 校准值参数 */
          if (Key_Down == 8)
            Para_Right = (Para_Right == 900) ? -900 : Para_Right + 100;
          else if (Key_Down == 9)
            Para_Right = (Para_Right == -900) ? 900 : Para_Right - 100;
          break;
          break;
      }
      break;
    case 3:
      /* 回显页面 */
      if (Key_Down == 5) Re_Show_Mode = (++Re_Show_Mode) % 2;
      break;
  }
}
void Seg_Proc() {
  unsigned int Para_Temp;
  Para_Temp = -Para_Right;
  if (Time_1s % 20) return;
  Read_Rtc(ucRtc);
  if (Freq_CAL > Freq_MAX) {
    Freq_MAX = Freq_CAL;
    Read_Rtc(MAX_Rtc);
  }
  if (Freq_Error)
    Da_Out_Amp = 0;
  else {
    if (Freq_CAL > Para_Limit)
      Da_Out_Amp = 5;
    else if (Freq_CAL < 500)
      Da_Out_Amp = 1;
    else
      Da_Out_Amp = (float)Freq_CAL * 4.0 / (Para_Limit - 500.0);
  }
  Da_Out_Dig = Da_Out_Amp * 51;
  switch (Seg_Show_Mode) {
    case 0:
      /* 频率界面 */
      Seg_Buf[0] = 11;  // F
      Seg_Buf[1] = Seg_Buf[2] = 10;
      if (Freq_Error) {
        Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
        Seg_Buf[6] = Seg_Buf[7] = 16;  // L
      } else {
        Seg_Buf[3] = (Freq_CAL / 10000 % 10 == 0) ? 10 : Freq_CAL / 10000 % 10;
        Seg_Buf[4] = (Seg_Buf[3] == 10 && Freq_CAL / 1000 % 10 == 0)
                         ? 10
                         : Freq_CAL / 1000 % 10;
        Seg_Buf[5] = (Seg_Buf[4] == 10 && Freq_CAL / 100 % 10 == 0)
                         ? 10
                         : Freq_CAL / 100 % 10;
        Seg_Buf[6] = (Seg_Buf[5] == 10 && Freq_CAL / 10 % 10 == 0)
                         ? 10
                         : Freq_CAL / 10 % 10;
        Seg_Buf[7] = Freq_CAL % 10;
      }
      break;
    case 1:
      /* 参数界面 */
      Seg_Buf[0] = 12;  // P
      Seg_Buf[1] = Para_Mode + 1;
      Seg_Buf[2] = Seg_Buf[3] = 10;
      switch (Para_Mode) {
        case 0:
          /* 超限参数 */
          Seg_Buf[4] = Para_Limit / 1000 % 10;
          Seg_Buf[5] = Para_Limit / 100 % 10;
          Seg_Buf[6] = Para_Limit / 10 % 10;
          Seg_Buf[7] = Para_Limit % 10;
          break;
        case 1:
          /* 校准值 */
          if (Para_Right >= 0) {
            // 正数
            Seg_Buf[4] = 10;
            if (Para_Right != 0) {
              Seg_Buf[5] = Para_Right / 100 % 10;
              Seg_Buf[6] = Para_Right / 10 % 10;
              Seg_Buf[7] = Para_Right % 10;
            } else {
              Seg_Buf[5] = Seg_Buf[6] = 10;
              Seg_Buf[7] = 0;
            }
          } else {
            // 负数
            Seg_Buf[4] = 13;
            Seg_Buf[5] = Para_Temp / 100 % 10;
            Seg_Buf[6] = Para_Temp / 10 % 10;
            Seg_Buf[7] = Para_Temp % 10;
          }
          break;
      }
      break;
    case 2:
      /* 时间界面 */
      Seg_Buf[0] = ucRtc[0] / 16;
      Seg_Buf[1] = ucRtc[0] % 16;
      Seg_Buf[2] = 13;  //-
      Seg_Buf[3] = ucRtc[1] / 16;
      Seg_Buf[4] = ucRtc[1] % 16;
      Seg_Buf[5] = 13;  //-
      Seg_Buf[6] = ucRtc[2] / 16;
      Seg_Buf[7] = ucRtc[2] % 16;
      break;
    case 3:
      /* 回显界面 */
      Seg_Buf[0] = 14;  // H
      switch (Re_Show_Mode) {
        case 0:
          /* 频率回显 */
          Seg_Buf[1] = 11;  // F
          Seg_Buf[2] = 10;
          Seg_Buf[3] =
              (Freq_MAX / 10000 % 10 == 0) ? 10 : Freq_MAX / 10000 % 10;
          Seg_Buf[4] = (Seg_Buf[3] == 10 && Freq_MAX / 1000 % 10 == 0)
                           ? 10
                           : Freq_MAX / 1000 % 10;
          Seg_Buf[5] = (Seg_Buf[4] == 10 && Freq_MAX / 100 % 10 == 0)
                           ? 10
                           : Freq_MAX / 100 % 10;
          Seg_Buf[6] = (Seg_Buf[5] == 10 && Freq_MAX / 10 % 10 == 0)
                           ? 10
                           : Freq_MAX / 10 % 10;
          Seg_Buf[7] = Freq_MAX % 10;
          break;
        case 1:
          /* 时间回显 */
          Seg_Buf[1] = 15;  // A
          Seg_Buf[2] = MAX_Rtc[0] / 16;
          Seg_Buf[3] = MAX_Rtc[0] % 16;
          Seg_Buf[4] = MAX_Rtc[1] / 16;
          Seg_Buf[5] = MAX_Rtc[1] % 16;
          Seg_Buf[6] = MAX_Rtc[2] / 16;
          Seg_Buf[7] = MAX_Rtc[2] % 16;
          break;
      }
  }
}
void Led_Proc() {
  ucLed[0] = Led_Blink_Flag_Freq;
  ucLed[1] = Led_Blink_Flag_Wring;
  Da_Write(Da_Out_Dig);
}
void Timer0Init(void)  // 100微秒@12.000MHz
{
  AUXR &= 0x7F;  // 定时器时钟12T模式
  TMOD &= 0xF0;  // 设置定时器模式
  TMOD |= 0x05;
  TL0 = 0x00;  // 设置定时初值
  TH0 = 0x00;  // 设置定时初值
  TF0 = 0;     // 清除TF0标志
  TR0 = 1;     // 定时器0开始计时
}
void Timer1Init(void)  // 1毫秒@12.000MHz
{
  AUXR &= 0xBF;  // 定时器时钟12T模式
  TMOD &= 0x0F;  // 设置定时器模式
  TL1 = 0x18;    // 设置定时初值
  TH1 = 0xFC;    // 设置定时初值
  TF1 = 0;       // 清除TF1标志
  TR1 = 1;       // 定时器1开始计时
  ET1 = 1;
  EA = 1;
}
void Timer1Isr(void) interrupt 3 {
  unsigned char i;
  unsigned int Para_Temp;
  Para_Temp = -Para_Right;
  if (++Time_1s == 1000) {
    Time_1s = 0;
    Freq = TH0 << 8 | TL0;
    if (Para_Right < 0 && Para_Temp > Freq) {
      Freq_Error = 1;
      Freq = 0;
    } else {
      Freq_Error = 0;
      Freq_CAL = Freq + Para_Right;
    }
    TH0 = TL0 = 0;
  }
  if (Seg_Show_Mode == 0) {
    if (++Time_200ms_Freq == 200) {
      Time_200ms_Freq = 0;
      Led_Blink_Flag_Freq ^= 1;
    }
  } else {
    Time_200ms_Freq = 0;
    Led_Blink_Flag_Freq = 0;
  }
  if (Freq_Error) {
    Time_200ms_Wring = 0;
    Led_Blink_Flag_Wring = 1;
  } else if (Freq > Para_Limit) {
    if (++Time_200ms_Wring == 200) {
      Time_200ms_Wring = 0;
      Led_Blink_Flag_Wring ^= 1;
    }
  } else {
    Time_200ms_Wring = 0;
    Led_Blink_Flag_Wring = 0;
  }
  Seg_Pos = (++Seg_Pos) % 8;
  Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
  for (i = 0; i < 8; i++) Led_Disp(i, ucLed[i]);
}
void main() {
  System_Init();
  Timer0Init();
  Timer1Init();
  Set_Rtc(ucRtc);
  while (1) {
    Key_Proc();
    Seg_Proc();
    Led_Proc();
  }
}