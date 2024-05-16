#include "main.h"

/* LED��ʾ */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* �������ʾ */
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10};  // �������ʾ��ֵ
uchar Seg_Pos;                                        // �����ָʾ
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};  // ĳλ�Ƿ���ʾС����

/* ���ڷ��� */
uchar Uart_Buf[6];    // ���ڽ��յ�������
uchar Uart_Rx_Index;  // ���ڽ��յ������ݵ�ָ��

/* ʱ�䷽�� */
uint time_all_1s;
uint long_press_1s_12;  // ����1s�ļ�ʱ��
uint long_press_1s_13;  // ����1s�ļ�ʱ��
uchar Sys_Tick;         // ��઼�ʱ����ʱ����
/* ���� */
uchar Seg_show_mode;   // 0 ���� 1 ����
uchar Data_show_mode;  // 0 �¶� 1 ���� 2 �������
uchar Para_show_mode;  // 0 �¶� 1 ����
uchar Out_mode;        // 0 ���� 1 ֹͣ
/* ���� */
uint T_value_100x;    // �¶ȵ�100��
uchar Dis_value;      // ����
uint Change_times;    // �������
uchar Para_T = 30;    // �¶Ȳ���
uchar Para_Dis = 35;  // �������
uchar Para_T_temp;
uchar Para_Dis_temp;
/* �ж�*/
bit press_12;   // ����12
bit press_13;   // ����13
bit Uart_flag;  // �Ƿ���յ���Ϣ
bit out_range;  // �Ƿ񳬳���Χ
bit hot_T;      // �Ƿ��¶ȹ���
/* ���ݴ����� */
void Data_Proc() {
  if (time_all_1s % 100 == 0) {
    // �����ȡ
    Dis_value = Ut_Wave_Data();
    out_range = Dis_value > Para_Dis;
  }
  if (time_all_1s % 500 == 0) {
    // �¶ȶ�ȡ
    T_value_100x = rd_temperature() * 100;
    hot_T = (T_value_100x > Para_T * 100);
  }
}
/* ���̴����� */
void Key_Proc() {
  static uchar Key_Val, Key_Down, Key_Up, Key_Old;
  if (time_all_1s % 10) return;
  Key_Val = Key_Read();
  Key_Down = Key_Val & (Key_Old ^ Key_Val);
  Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
  Key_Old = Key_Val;

  if (Key_Down == 12) {
    press_12 = 1;  // ����12
  }
  // ����ʱ��
  if (long_press_1s_12 >= 1000) {
    press_12 = 0;
    // ִ�г���1s�Ĳ���
    Change_times = 0;
  }
  // ̧��12������ʱ��û�е���1s
  if (Key_Up == 12 && press_12) {
    press_12 = 0;  // �ɿ�12
    // ʱ��δ�����ɿ�������̰�
    switch (Seg_show_mode) {
      case 0:
        /* ���� */
        Data_show_mode = (++Data_show_mode) % 3;
        break;

      case 1:
        /* ���� */
        Para_show_mode = (++Para_show_mode) % 2;
        break;
    }
  }
  if (Key_Down == 13) {
    press_13 = 1;  // ����13
  }
  // ����ʱ��
  if (long_press_1s_13 >= 1000) {
    press_13 = 0;
    // ִ�г�������
    Out_mode = (++Out_mode) % 2;
  }
  // ̧��13������ʱ��û�е���1s
  if (Key_Up == 13 & press_13) {
    press_13 = 0;  // �ɿ�13
    // ʱ��δ�����ɿ�������̰�
    Seg_show_mode = (++Seg_show_mode) % 2;
    Data_show_mode = 0;
    Para_show_mode = 0;
    if (Seg_show_mode == 1) {
      // �������ҳ��
      Para_Dis_temp = Para_Dis;
      Para_T_temp = Para_T;
    } else {
      // �˳�����ҳ��
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
      /* �¶� */
      if (Key_Down == 16) Para_T = (Para_T == 0) ? 99 : Para_T - 2;
      if (Key_Down == 17) Para_T = (Para_T == 99) ? 0 : Para_T + 2;
      break;

    case 1:
      /* ���� */
      if (Key_Down == 16) Para_Dis = (Para_Dis == 0) ? 99 : Para_Dis - 5;
      if (Key_Down == 17) Para_Dis = (Para_Dis == 99) ? 0 : Para_Dis + 5;
      break;
  }
}
/* ����ܴ����� */
void Seg_Proc() {
  if (time_all_1s % 20) return;
  switch (Seg_show_mode) {
    case 0:
      /* ���� */
      switch (Data_show_mode) {
        case 0:
          /* �¶� */
          Seg_Point[5] = 1;
          Seg_Buf[0] = 11;  // C
          Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = 10;
          Seg_Buf[4] = T_value_100x / 1000 % 10;
          Seg_Buf[5] = T_value_100x / 100 % 10;
          Seg_Buf[6] = T_value_100x / 10 % 10;
          Seg_Buf[7] = T_value_100x % 10;
          break;

        case 1:
          /* ���� */
          Seg_Point[5] = 0;
          Seg_Buf[0] = 12;  // L
          Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
          Seg_Buf[6] = Dis_value / 10 % 10;
          Seg_Buf[7] = Dis_value % 10;
          break;

        case 2:
          /* ������� */
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
      /* ���� */
      Seg_Point[5] = 0;
      Seg_Buf[0] = 14;  // P
      Seg_Buf[1] = Seg_Buf[2] = 10;
      Seg_Buf[3] = Para_show_mode + 1;
      Seg_Buf[4] = Seg_Buf[5] = 10;
      switch (Para_show_mode) {
        case 0:
          /* �¶� */
          Seg_Buf[6] = Para_T_temp / 10 % 10;
          Seg_Buf[7] = Para_T_temp % 10;
          break;

        case 1:
          /* ���� */
          Seg_Buf[6] = Para_Dis_temp / 10 % 10;
          Seg_Buf[7] = Para_Dis_temp % 10;
          break;
      }
      break;
  }
}

/* LED������ */
void Led_Proc() {
  switch (Out_mode) {
    case 0:
      /* ���� */
      if (out_range)
        Da_Write(4 * 51);  // ������Χ�����4V
      else
        Da_Write(2 * 51);  // û����Χ�����2V
      break;

    case 1:
      /* ֹͣ */
      Da_Write(0.4 * 51);  // ֹͣ״̬�����0.4V
      break;
  }
  ucLed[0] = hot_T;
  ucLed[1] = !out_range;
  ucLed[2] = !Out_mode;
}

/* ���ڴ����� */
void Uart_Proc() {
  if (Uart_Rx_Index == 0) return;
  // ����10msû���յ�����
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

/* ��ʱ��0�жϳ�ʼ�� */
void Timer0_Init(void)  // 1����@12.000MHz
{
  AUXR &= 0x7F;  // ��ʱ��ʱ��12Tģʽ
  TMOD &= 0xF0;  // ���ö�ʱ��ģʽ
  TL0 = 0x18;    // ���ö�ʱ��ʼֵ
  TH0 = 0xFC;    // ���ö�ʱ��ʼֵ
  TF0 = 0;       // ���TF0��־
  TR0 = 1;       // ��ʱ��0��ʼ��ʱ
  ET0 = 1;
  EA = 1;
}

/* ��ʱ��0�жϺ��� */
void Timer0_ISR(void) interrupt 1 {
  uchar i;
  if (++time_all_1s == 1000) time_all_1s = 0;
  // ������ʱ����
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

  // ������લ���
  if (Uart_flag) Sys_Tick++;
  Seg_Pos = (++Seg_Pos) % 8;
  Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
  for (i = 0; i < 8; i++) Led_Disp(i, ucLed[i]);
}

/* �����жϷ����� */
void Uart_ISR(void) interrupt 4 {
  if (RI == 1)  // ���ڽ��յ�����
  {
    Uart_flag = 1;
    Sys_Tick = 0;  // ���ܼ�ʱ
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
  EEPROM_Read(&input_passwd, 8, 1);  // �ò���д��ĵط���У��
  if (input_passwd != passwd)  // У��ʧ�ܣ�֮ǰδд������1/256���ʳ�����
  {
    EEPROM_Write(&passwd, 8, 1);
  } else  // У��ͨ������ȡ������Ҫ������
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