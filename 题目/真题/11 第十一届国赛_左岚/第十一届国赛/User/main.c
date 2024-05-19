#include "main.h"

/* LED��ʾ */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* �������ʾ */                                      // ����ܼ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10};  // �������ʾ��ֵ
uchar Seg_Pos;                                        // �����ָʾ
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};  // ĳλ�Ƿ���ʾС����

/* ʱ�䷽�� */
uchar ucRtc[3] = {16, 59, 50};  // ��ʼ��ʱ��16:59:50

/* ʱ�䷽�� */
uint time_all_1s;
uint time_light_3s;
uint time_dark_3s;
/* ���� */
uchar Seg_show_mode;   // 0 ���� 1 ����
uchar Data_show_mode;  // 0 ʱ�� 1 �¶� 2 �ⰵ״̬
uchar Para_show_mode;  // 0 ʱ�� 1 �¶� 2 ָʾ��
/* ���� */
uint T_value_10x;      // �¶ȵ�10��
uint V_value_100x;     // ���������ѹ100��
uchar Para_hour = 17;  // Сʱ����(0-23)
uchar Para_T = 25;     // �¶Ȳ���(0-99)
uchar Para_Led = 3;    // ָʾ�Ʋ��� ʵ��(3-7) -> ��ʾ (4-8)
uchar Para_hour_temp;  // ����Сʱ����(0-23)
uchar Para_T_temp;     // �����¶Ȳ���(0-99)
uchar Para_Led_temp;   // ����ָʾ�Ʋ��� ʵ��(3-7) -> ��ʾ (4-8)

/* �ж� */
bit dark_flag;  // 0 �� 1 ��
bit cold_flag;  // 0 ���� 1 ���ڲ���
/* ���ݴ����� */
void Data_Proc() {
  if (time_all_1s % 99 == 0) {
    // ʱ���ȡ
    Read_Rtc(ucRtc);
  }
  if (time_all_1s % 199 == 0) {
    // AD��ȡ
    V_value_100x = Ad_Read(0x01) * 100 / 51;
    // �����������ѹС��1Vʱ���ж�Ϊ��
    dark_flag = (V_value_100x < 100);
  }
  if (time_all_1s % 499 == 0) {
    // �¶ȶ�ȡ
    T_value_10x = rd_temperature() * 10;
    cold_flag = (Para_T * 10 > T_value_10x);
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
  if (Key_Down == 4) {
    Seg_show_mode = (++Seg_show_mode) % 2;
    Data_show_mode = 0;
    Para_show_mode = 0;
    // ���ڽ���������棬��ȫ��ֵ����ȥ
    if (Seg_show_mode == 1) {
      Para_hour_temp = Para_hour;
      Para_T_temp = Para_T;
      Para_Led_temp = Para_Led;
    }
    // ���ڽ������ݽ��棬��ȫ��ֵ����ȥ
    else {
      Para_hour = Para_hour_temp;
      Para_T = Para_T_temp;
      Para_Led = Para_Led_temp;
    }
  }
  switch (Seg_show_mode) {
    case 0:
      /* ���� */
      if (Key_Down == 5) Data_show_mode = (++Data_show_mode) % 3;
      break;

    case 1:
      /* ���� */
      if (Key_Down == 5) Para_show_mode = (++Para_show_mode) % 3;
      switch (Para_show_mode) {
        case 0:
          /* ʱ�� */
          if (Key_Down == 8)
            Para_hour_temp = (Para_hour_temp == 0) ? 23 : Para_hour_temp - 1;
          else if (Key_Down == 9)
            Para_hour_temp = (Para_hour_temp == 23) ? 0 : Para_hour_temp + 1;
          break;
        case 1:
          /* �¶� */
          if (Key_Down == 8)
            Para_T_temp = (Para_T_temp == 0) ? 99 : Para_T_temp - 1;
          else if (Key_Down == 9)
            Para_T_temp = (Para_T_temp == 99) ? 0 : Para_T_temp + 1;
          break;

        case 2:
          /* Ledָʾ��*/
          if (Key_Down == 8)
            Para_Led_temp = (Para_Led_temp == 3) ? 7 : Para_Led_temp - 1;
          else if (Key_Down == 9)
            Para_Led_temp = (Para_Led_temp == 7) ? 3 : Para_Led_temp + 1;
          break;
      }
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
          /* ʱ�� */
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
          /* �¶� */
          Seg_Point[2] = 0;
          Seg_Point[6] = 1;
          Seg_Buf[0] = 12;  // C
          Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
          Seg_Buf[5] = T_value_10x / 100 % 10;
          Seg_Buf[6] = T_value_10x / 10 % 10;
          Seg_Buf[7] = T_value_10x % 10;
          break;

        case 2:
          /* �ⰵ״̬ */
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
      /* ���� */
      Seg_Point[2] = 0;
      Seg_Point[6] = 0;
      Seg_Buf[0] = 14;  // P
      Seg_Buf[1] = Para_show_mode + 1;
      Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
      switch (Para_show_mode) {
        case 0:
          /* ʱ�� */
          Seg_Buf[6] = Para_hour_temp / 10;
          Seg_Buf[7] = Para_hour_temp % 10;
          break;

        case 1:
          /* �¶� */
          Seg_Buf[6] = Para_T_temp / 10;
          Seg_Buf[7] = Para_T_temp % 10;
          break;

        case 2:
          /* ָʾ�� */
          Seg_Buf[6] = 10;
          Seg_Buf[7] = Para_Led_temp + 1;
          break;
      }
      break;
  }
}

/* LED������ */
void Led_Proc() {
  uchar i;
  if (Para_hour <= 8) {
    // СʱС��8���Ҵ��ڲ���������Сʱ���ڲ������ǲ�������
    if ((ucRtc[0] < 8 && ucRtc[0] > Para_hour) ||
        !(ucRtc[0] == Para_hour && ucRtc[1] == 0 && ucRtc[2] == 0))
      ucLed[0] = 1;
    else
      ucLed[0] = 0;
  } else {
    // Сʱ���ڲ���������СʱС��8������Сʱ���ڲ�������������
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
  Seg_Pos = (++Seg_Pos) % 8;
  if (dark_flag) {
    // ���ںڰ�״̬��ʼ��ʱ��ֹͣ������ʱ�����ҽ��ڰ���ʱ����
    time_light_3s = 0;
    if (++time_dark_3s >= 3000) {
      time_dark_3s = 3001;
    }
  } else {
    // ���ڹ���״̬��ʼ��ʱ��ֹͣ�ڰ���ʱ�����ҽ�������ʱ����
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