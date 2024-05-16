#include "main.h"

/* LED显示 */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* 数码管显示 */
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10};  // 数码管显示的值
uchar Seg_Pos;                                        // 数码管指示
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};  // 某位是否显示小数点

/* 串口方面 */
uchar Uart_Buf[6];    // 串口接收到的数据
uchar Uart_Rx_Index;  // 串口接收到的数据的指针

/* 时间方面 */
uint time_all_1s;
uint long_press_1s_12;  // 长按1s的计时器
uint long_press_1s_13;  // 长按1s的计时器
uchar Sys_Tick;         // 嘀嗒计时，超时解析
/* 界面 */
uchar Seg_show_mode;   // 0 数据 1 参数
uchar Data_show_mode;  // 0 温度 1 距离 2 变更次数
uchar Para_show_mode;  // 0 温度 1 距离
uchar Out_mode;        // 0 启动 1 停止
/* 数据 */
uint T_value_100x;    // 温度的100倍
uchar Dis_value;      // 距离
uint Change_times;    // 变更次数
uchar Para_T = 30;    // 温度参数
uchar Para_Dis = 35;  // 距离参数
uchar Para_T_temp;
uchar Para_Dis_temp;
/* 判断*/
bit press_12;   // 按下12
bit press_13;   // 按下13
bit Uart_flag;  // 是否接收到信息
bit out_range;  // 是否超出范围
bit hot_T;      // 是否温度过高
/* 数据处理函数 */
void Data_Proc() {
  if (time_all_1s % 100 == 0) {
    // 距离读取
    Dis_value = Ut_Wave_Data();
    out_range = Dis_value > Para_Dis;
  }
  if (time_all_1s % 500 == 0) {
    // 温度读取
    T_value_100x = rd_temperature() * 100;
    hot_T = (T_value_100x > Para_T * 100);
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

  if (Key_Down == 12) {
    press_12 = 1;  // 按下12
  }
  // 到达时间
  if (long_press_1s_12 >= 1000) {
    press_12 = 0;
    // 执行长按1s的操作
    Change_times = 0;
  }
  // 抬起12，并且时间没有到达1s
  if (Key_Up == 12 && press_12) {
    press_12 = 0;  // 松开12
    // 时间未到就松开，代表短按
    switch (Seg_show_mode) {
      case 0:
        /* 数据 */
        Data_show_mode = (++Data_show_mode) % 3;
        break;

      case 1:
        /* 参数 */
        Para_show_mode = (++Para_show_mode) % 2;
        break;
    }
  }
  if (Key_Down == 13) {
    press_13 = 1;  // 按下13
  }
  // 到达时间
  if (long_press_1s_13 >= 1000) {
    press_13 = 0;
    // 执行长按操作
    Out_mode = (++Out_mode) % 2;
  }
  // 抬起13，并且时间没有到达1s
  if (Key_Up == 13 & press_13) {
    press_13 = 0;  // 松开13
    // 时间未到就松开，代表短按
    Seg_show_mode = (++Seg_show_mode) % 2;
    Data_show_mode = 0;
    Para_show_mode = 0;
    if (Seg_show_mode == 1) {
      // 进入参数页面
      Para_Dis_temp = Para_Dis;
      Para_T_temp = Para_T;
    } else {
      // 退出参数页面
      if (Para_Dis_temp != Para_Dis || Para_T_temp != Para_T) {
        Change_times = (Change_times == 65535) ? 65535 : Change_times + 1;
        EEPROM_Write(&Change_times, 0, 2);
      }
      Para_Dis = Para_Dis_temp;
      Para_T = Para_T_temp;
    }
  }
  switch (Para_show_mode) {
    case 0:
      /* 温度 */
      if (Key_Down == 16) Para_T = (Para_T == 0) ? 99 : Para_T - 2;
      if (Key_Down == 17) Para_T = (Para_T == 99) ? 0 : Para_T + 2;
      break;

    case 1:
      /* 距离 */
      if (Key_Down == 16) Para_Dis = (Para_Dis == 0) ? 99 : Para_Dis - 5;
      if (Key_Down == 17) Para_Dis = (Para_Dis == 99) ? 0 : Para_Dis + 5;
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
          /* 温度 */
          Seg_Point[5] = 1;
          Seg_Buf[0] = 11;  // C
          Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = 10;
          Seg_Buf[4] = T_value_100x / 1000 % 10;
          Seg_Buf[5] = T_value_100x / 100 % 10;
          Seg_Buf[6] = T_value_100x / 10 % 10;
          Seg_Buf[7] = T_value_100x % 10;
          break;

        case 1:
          /* 距离 */
          Seg_Point[5] = 0;
          Seg_Buf[0] = 12;  // L
          Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
          Seg_Buf[6] = Dis_value / 10 % 10;
          Seg_Buf[7] = Dis_value % 10;
          break;

        case 2:
          /* 变更次数 */
          Seg_Point[5] = 0;
          Seg_Buf[0] = 13;  // n
          Seg_Buf[1] = Seg_Buf[2] = 10;
          Seg_Buf[3] =
              (Change_times / 10000 % 10 == 0) ? 10 : Change_times / 10000 % 10;
          Seg_Buf[4] = (Seg_Buf[3] == 10 && (Change_times / 1000 % 10 == 0))
                           ? 10
                           : Change_times / 1000 % 10;
          Seg_Buf[5] = (Seg_Buf[4] == 10 && (Change_times / 100 % 10 == 0))
                           ? 10
                           : Change_times / 100 % 10;
          Seg_Buf[6] = (Seg_Buf[5] == 10 && (Change_times / 10 % 10 == 0))
                           ? 10
                           : Change_times / 10 % 10;
          Seg_Buf[7] = Change_times % 10;
          break;
      }
      break;

    case 1:
      /* 参数 */
      Seg_Point[5] = 0;
      Seg_Buf[0] = 14;  // P
      Seg_Buf[1] = Seg_Buf[2] = 10;
      Seg_Buf[3] = Para_show_mode + 1;
      Seg_Buf[4] = Seg_Buf[5] = 10;
      switch (Para_show_mode) {
        case 0:
          /* 温度 */
          Seg_Buf[6] = Para_T_temp / 10 % 10;
          Seg_Buf[7] = Para_T_temp % 10;
          break;

        case 1:
          /* 距离 */
          Seg_Buf[6] = Para_Dis_temp / 10 % 10;
          Seg_Buf[7] = Para_Dis_temp % 10;
          break;
      }
      break;
  }
}

/* LED处理函数 */
void Led_Proc() {
  switch (Out_mode) {
    case 0:
      /* 启动 */
      if (out_range)
        Da_Write(4 * 51);  // 超出范围，输出4V
      else
        Da_Write(2 * 51);  // 没超范围，输出2V
      break;

    case 1:
      /* 停止 */
      Da_Write(0.4 * 51);  // 停止状态，输出0.4V
      break;
  }
  ucLed[0] = hot_T;
  ucLed[1] = !out_range;
  ucLed[2] = !Out_mode;
}

/* 串口处理函数 */
void Uart_Proc() {
  if (Uart_Rx_Index == 0) return;
  // 超过10ms没有收到数据
  if (Sys_Tick >= 10) {
    Sys_Tick = Uart_flag = 0;
    if (Uart_Buf[0] == 'S' && Uart_Buf[1] == 'T') {
      printf("$%bu,%5.2f\r\n", Dis_value, (float)T_value_100x / 100.0);
    } else if (Uart_Buf[0] == 'P' && Uart_Buf[1] == 'A' && Uart_Buf[2] == 'R' &&
               Uart_Buf[3] == 'A') {
      printf("#%bu,%bu\r\n", Para_Dis, Para_T);
    } else {
      printf("ERROR\r\n");
    }
    memset(Uart_Buf, 0, Uart_Rx_Index);
    Uart_Rx_Index = 0;
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
  // 长按计时部分
  if (press_12) {
    if (++long_press_1s_12 >= 1000) {
      long_press_1s_12 = 1001;
    }
  } else {
    long_press_1s_12 = 0;
  }
  if (press_13) {
    if (++long_press_1s_13 >= 1000) {
      long_press_1s_13 = 1001;
    }
  } else {
    long_press_1s_13 = 0;
  }

  // 串口嘀嗒部分
  if (Uart_flag) Sys_Tick++;
  Seg_Pos = (++Seg_Pos) % 8;
  Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
  for (i = 0; i < 8; i++) Led_Disp(i, ucLed[i]);
}

/* 串口中断服务函数 */
void Uart_ISR(void) interrupt 4 {
  if (RI == 1)  // 串口接收到数据
  {
    Uart_flag = 1;
    Sys_Tick = 0;  // 接受计时
    Uart_Buf[Uart_Rx_Index] = SBUF;
    Uart_Rx_Index++;
    RI = 0;
  }
  if (Uart_Rx_Index > 6) Uart_Rx_Index = 0;
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
uchar passwd = 155;
uchar input_passwd;
void main() {
  System_Init();
  Timer0_Init();
  Uart1_Init();
  rd_temperature();
  Delay750ms();
  EEPROM_Read(&input_passwd, 8, 1);  // 用不会写入的地方做校验
  if (input_passwd != passwd)  // 校验失败，之前未写入数据1/256概率出问题
  {
    EEPROM_Write(&passwd, 8, 1);
  } else  // 校验通过，读取我们需要的数据
  {
    EEPROM_Read(&Change_times, 0, 2);
  }
	T_value_100x = rd_temperature() * 100;
  while (1) {
    Data_Proc();
    Key_Proc();
    Seg_Proc();
    Uart_Proc();
    Led_Proc();
  }
}