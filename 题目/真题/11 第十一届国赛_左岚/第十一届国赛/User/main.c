#include "main.h"

/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */                                      // 数码管减速
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10};  // 数码管显示的值
uchar Seg_Pos;                                        // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};  // 某位是否显示小数点

/* 时间方面 */
uchar ucRtc[3] = {16, 59, 50};  // 初始化时间16:59:50

/* 时间方面 */
uint time_all_1s;
uint time_light_3s;
uint time_dark_3s;
/* 界面 */
uchar Seg_show_mode;   // 0 数据 1 参数
uchar Data_show_mode;  // 0 时间 1 温度 2 光暗状态
uchar Para_show_mode;  // 0 时间 1 温度 2 指示灯
/* 数据 */
uint T_value_10x;      // 温度的10倍
uint V_value_100x;     // 光敏电阻分压100倍
uchar Para_hour = 17;  // 小时参数(0-23)
uchar Para_T = 25;     // 温度参数(0-99)
uchar Para_Led = 3;    // 指示灯参数 实际(3-7) -> 显示 (4-8)
uchar Para_hour_temp;  // 设置小时参数(0-23)
uchar Para_T_temp;     // 设置温度参数(0-99)
uchar Para_Led_temp;   // 设置指示灯参数 实际(3-7) -> 显示 (4-8)

/* 判断 */
bit dark_flag;  // 0 亮 1 暗
bit cold_flag;  // 0 正常 1 低于参数
/* 数据处理函数 */
void Data_Proc() {
  if (time_all_1s % 99 == 0) {
    // 时间读取
    Read_Rtc(ucRtc);
  }
  if (time_all_1s % 199 == 0) {
    // AD读取
    V_value_100x = Ad_Read(0x01) * 100 / 51;
    // 当光敏电阻分压小于1V时，判断为暗
    dark_flag = (V_value_100x < 100);
  }
  if (time_all_1s % 499 == 0) {
    // 温度读取
    T_value_10x = rd_temperature() * 10;
    cold_flag = (Para_T * 10 > T_value_10x);
  }
}
/* 键盘处理函数 */
void Key_Proc() {
  static uchar Key_Val, Key_Down, Key_Up, Key_Old;
  if (time_all_1s % 10) return;
  Key_Val = Key_Read();
  Key_Down = Key_Val & (Key_Old ^ Key_Val);
  Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
  Key_Old = Key_Val;
  if (Key_Down == 4) {
    Seg_show_mode = (++Seg_show_mode) % 2;
    Data_show_mode = 0;
    Para_show_mode = 0;
    // 现在进入参数界面，将全部值给过去
    if (Seg_show_mode == 1) {
      Para_hour_temp = Para_hour;
      Para_T_temp = Para_T;
      Para_Led_temp = Para_Led;
    }
    // 现在进入数据界面，将全部值给过去
    else {
      Para_hour = Para_hour_temp;
      Para_T = Para_T_temp;
      Para_Led = Para_Led_temp;
    }
  }
  switch (Seg_show_mode) {
    case 0:
      /* 数据 */
      if (Key_Down == 5) Data_show_mode = (++Data_show_mode) % 3;
      break;

    case 1:
      /* 参数 */
      if (Key_Down == 5) Para_show_mode = (++Para_show_mode) % 3;
      switch (Para_show_mode) {
        case 0:
          /* 时间 */
          if (Key_Down == 8)
            Para_hour_temp = (Para_hour_temp == 0) ? 23 : Para_hour_temp - 1;
          else if (Key_Down == 9)
            Para_hour_temp = (Para_hour_temp == 23) ? 0 : Para_hour_temp + 1;
          break;
        case 1:
          /* 温度 */
          if (Key_Down == 8)
            Para_T_temp = (Para_T_temp == 0) ? 99 : Para_T_temp - 1;
          else if (Key_Down == 9)
            Para_T_temp = (Para_T_temp == 99) ? 0 : Para_T_temp + 1;
          break;

        case 2:
          /* Led指示灯*/
          if (Key_Down == 8)
            Para_Led_temp = (Para_Led_temp == 3) ? 7 : Para_Led_temp - 1;
          else if (Key_Down == 9)
            Para_Led_temp = (Para_Led_temp == 7) ? 3 : Para_Led_temp + 1;
          break;
      }
      break;
  }
}
/* 数码管处理函数 */
void Seg_Proc() {
  if (time_all_1s % 20) return;
  switch (Seg_show_mode) {
    case 0:
      /* 数据 */
      switch (Data_show_mode) {
        case 0:
          /* 时间 */
          Seg_Point[2] = 0;
          Seg_Point[6] = 0;
          Seg_Buf[0] = ucRtc[0] / 10;
          Seg_Buf[1] = ucRtc[0] % 10;
          Seg_Buf[2] = 11;  //-
          Seg_Buf[3] = ucRtc[1] / 10;
          Seg_Buf[4] = ucRtc[1] % 10;
          Seg_Buf[5] = 11;  //-
          Seg_Buf[6] = ucRtc[2] / 10;
          Seg_Buf[7] = ucRtc[2] % 10;
          break;

        case 1:
          /* 温度 */
          Seg_Point[2] = 0;
          Seg_Point[6] = 1;
          Seg_Buf[0] = 12;  // C
          Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
          Seg_Buf[5] = T_value_10x / 100 % 10;
          Seg_Buf[6] = T_value_10x / 10 % 10;
          Seg_Buf[7] = T_value_10x % 10;
          break;

        case 2:
          /* 光暗状态 */
          Seg_Point[2] = 1;
          Seg_Point[6] = 0;
          Seg_Buf[0] = 13;  // E
          Seg_Buf[1] = 10;
          Seg_Buf[2] = V_value_100x / 100 % 10;
          Seg_Buf[3] = V_value_100x / 10 % 10;
          Seg_Buf[4] = V_value_100x % 10;
          Seg_Buf[5] = Seg_Buf[6] = 10;
          Seg_Buf[7] = dark_flag;
          break;
      }
      break;

    case 1:
      /* 参数 */
      Seg_Point[2] = 0;
      Seg_Point[6] = 0;
      Seg_Buf[0] = 14;  // P
      Seg_Buf[1] = Para_show_mode + 1;
      Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
      switch (Para_show_mode) {
        case 0:
          /* 时间 */
          Seg_Buf[6] = Para_hour_temp / 10;
          Seg_Buf[7] = Para_hour_temp % 10;
          break;

        case 1:
          /* 温度 */
          Seg_Buf[6] = Para_T_temp / 10;
          Seg_Buf[7] = Para_T_temp % 10;
          break;

        case 2:
          /* 指示灯 */
          Seg_Buf[6] = 10;
          Seg_Buf[7] = Para_Led_temp + 1;
          break;
      }
      break;
  }
}

/* LED处理函数 */
void Led_Proc() {
  uchar i;
  if (Para_hour <= 8) {
    // 小时小于8并且大于参数，或者小时等于参数但是不是整点
    if ((ucRtc[0] < 8 && ucRtc[0] > Para_hour) ||
        !(ucRtc[0] == Para_hour && ucRtc[1] == 0 && ucRtc[2] == 0))
      ucLed[0] = 1;
    else
      ucLed[0] = 0;
  } else {
    // 小时大于参数，或者小时小于8，或者小时等于参数但不是整点
    if (ucRtc[0] < 8 || ucRtc[0] > Para_hour ||
        !(ucRtc[0] == Para_hour && ucRtc[1] == 0 && ucRtc[2] == 0))
      ucLed[0] = 1;
    else
      ucLed[0] = 0;
  }
  ucLed[1] = cold_flag;
  ucLed[2] = (time_dark_3s >= 3000 || time_light_3s <= 3000);
  for (i = 3; i < 8; i++) {
    ucLed[i] = (dark_flag) ? (i == Para_Led) : 0;
  }
}

/* 定时器0中断初始化 */
void Timer0_Init(void)  // 1毫秒@12.000MHz
{
  AUXR &= 0x7F;  // 定时器时钟12T模式
  TMOD &= 0xF0;  // 设置定时器模式
  TL0 = 0x18;    // 设置定时初始值
  TH0 = 0xFC;    // 设置定时初始值
  TF0 = 0;       // 清除TF0标志
  TR0 = 1;       // 定时器0开始计时
  ET0 = 1;
  EA = 1;
}

/* 定时器0中断函数 */
void Timer0_ISR(void) interrupt 1 {
  uchar i;
  if (++time_all_1s == 1000) time_all_1s = 0;
  Seg_Pos = (++Seg_Pos) % 8;
  if (dark_flag) {
    // 处于黑暗状态开始计时，停止光明计时，并且将黑暗计时卡死
    time_light_3s = 0;
    if (++time_dark_3s >= 3000) {
      time_dark_3s = 3001;
    }
  } else {
    // 处于光明状态开始计时，停止黑暗计时，并且将光明计时卡死
    time_dark_3s = 0;
    if (++time_light_3s >= 3000) {
      time_light_3s = 3001;
    }
  }
  Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
  for (i = 0; i < 8; i++) {
    Led_Disp(i, ucLed[i]);
  }
}

void Delay750ms(void)  //@12.000MHz
{
  unsigned char data i, j, k;

  _nop_();
  _nop_();
  i = 35;
  j = 51;
  k = 182;
  do {
    do {
      while (--k);
    } while (--j);
  } while (--i);
}
void main() {
  System_Init();
  Timer0_Init();
  Set_Rtc(ucRtc);
  rd_temperature();
  Delay750ms();
  while (1) {
    Data_Proc();
    Key_Proc();
    Seg_Proc();
    Led_Proc();
  }
}