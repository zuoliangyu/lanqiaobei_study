#include "main.h"
/* LED��ʾ */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* �������ʾ */
uchar Seg_Slow_Down;                                // ����ܼ���
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ��ֵ
uchar Seg_Pos;                                      // �����ָʾ
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // ĳλ�Ƿ���ʾС����

/* ʱ�䷽�� */
uchar ucRtc[3] = {0x13, 0x11, 0x11}; // ��ʼ��ʱ��13:11:11

/* ���̷��� */
uchar Key_Slow_Down;

/* ��ʾ����� */
uchar Seg_show_mode;  // 0 �¶� 1 ʱ�� 2 ����
bit Control_mode;     // ����ģʽ 0 �¶� 1 ʱ��
bit Time_show_mode;   // ʱ����ʾģʽ 0 ʱ�� 1 ����
bit Temperature_mode; // �¶ȿ��Ƶ���
bit Time_mode;        // ʱ����Ƶ���
bit Time_Led;         // �������
bit Led_show;         // ��˸
/* ���� */
uint Temperature_value_10x;  // �¶Ȳ���ֵ 10��
uchar Temperature_para = 23; // �¶Ȳ���

/* ʱ�� */
uint time_5s;
uint time_5s_led;
uchar time_100ms;

/* ���̴����� */
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
    if (Key_Down == 12)
        Seg_show_mode = (++Seg_show_mode) % 3;
    if (Key_Down == 13)
        Control_mode ^= 1;
    if (Seg_show_mode == 2)
    {
        if (Key_Down == 16)
            Temperature_para = (++Temperature_para > 99) ? 99 : Temperature_para;
        if (Key_Down == 17)
            Temperature_para = (--Temperature_para < 10) ? 10 : Temperature_para;
    }
    if (Seg_show_mode == 2)
    {
        if (Key_Old == 17)
            Time_show_mode = 1;
        if (Key_Up == 17)
            Time_show_mode = 0;
    }
}
/* ����ܴ����� */
void Seg_Proc()
{
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    Seg_Buf[0] = 11; // U
    Seg_Buf[1] = Seg_show_mode + 1;
    Read_Rtc(ucRtc);
    Temperature_value_10x = rd_temperature() * 10;
    switch (Seg_show_mode)
    {
    case 0:
        /* �¶� */
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
        Seg_Buf[5] = Temperature_value_10x / 100 % 10;
        Seg_Buf[6] = Temperature_value_10x / 10 % 10;
        Seg_Buf[7] = Temperature_value_10x % 10;
        Seg_Point[6] = 1;
        break;
    case 1:
        /* ʱ�� */

        Seg_Buf[2] = 10;
        Seg_Buf[3] = Time_show_mode ? ucRtc[1] / 16 : ucRtc[0] / 16;
        Seg_Buf[4] = Time_show_mode ? ucRtc[1] % 16 : ucRtc[0] % 16;
        Seg_Buf[5] = 12; //-
        Seg_Buf[6] = Time_show_mode ? ucRtc[2] / 16 : ucRtc[1] / 16;
        Seg_Buf[7] = Time_show_mode ? ucRtc[2] % 16 : ucRtc[1] % 16;
        Seg_Point[6] = 0;
        break;
    case 2:
        /* ���� */
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
        Seg_Buf[6] = Temperature_para / 10 % 10;
        Seg_Buf[7] = Temperature_para % 10;
        Seg_Point[6] = 0;
        break;
    }
}

/* LED������ */
void Led_Proc()
{
    // ʱ�����
    if (Control_mode)
    {
        Temperature_mode = 0; // �ر��¶ȱ���������ͻȻ�л�����bug
        // ���ֺ���ͬʱ��0x00��ʱ��
        if ((ucRtc[1] || ucRtc[2]) == 0)
            Time_mode = 1; // ��������״̬
    }
    // �¶ȿ���
    else
    {
        Time_mode = 0; // �ر�����״̬������ͻȻ�л�����bug
        if (Temperature_value_10x >= Temperature_para * 10)
            Temperature_mode = 1; // �¶ȱ���
        else
            Temperature_mode = 0; // �¶�����
    }
    Relay(Temperature_mode || Time_mode); // �����¶ȱ���������״̬��ʱ������

    // ���ֺ���ͬʱ��0x00��ʱ��
    if ((ucRtc[1] || ucRtc[2]) == 0)
        Time_Led = 1;
    ucLed[0] = Time_Led; // LED������5s
    ucLed[1] = (Control_mode == 0);
    ucLed[2] = Led_show;
}

/* ��ʱ��0�жϳ�ʼ�� */
void Timer0_Init(void) // 1����@12.000MHz
{
    AUXR &= 0x7F; // ��ʱ��ʱ��12Tģʽ
    TMOD &= 0xF0; // ���ö�ʱ��ģʽ
    TL0 = 0x18;   // ���ö�ʱ��ʼֵ
    TH0 = 0xFC;   // ���ö�ʱ��ʼֵ
    TF0 = 0;      // ���TF0��־
    TR0 = 1;      // ��ʱ��0��ʼ��ʱ
    ET0 = 1;
    EA = 1;
}

/* ��ʱ��0�жϺ��� */
void Timer0_ISR(void) interrupt 1
{
    if (++Key_Slow_Down == 10)
        Key_Slow_Down = 0;
    if (++Seg_Slow_Down == 500)
        Seg_Slow_Down = 0;
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
    if (Time_mode)
    {
        if (++time_5s == 5000)
        {
            time_5s = 0;
            Time_mode = 0;
        }
    }
    // ��ʱ��û�п��Ƶ�ʱ��������ʱ��Ϊ0
    else
    {
        time_5s = 0;
    }

    if (Time_Led)
    {
        if (++time_5s_led == 5000)
        {
            time_5s_led = 0;
            Time_mode = 0;
        }
    }

    if (Temperature_mode || Time_mode)
    {
        if (++time_100ms == 100)
        {
            time_100ms = 0;
            Led_show ^= 1;
        }
    }
    else
    {
        time_100ms = 0;
        Led_show = 0;
    }
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
    Set_Rtc(ucRtc);
    Delay750ms();
    rd_temperature();
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}