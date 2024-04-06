#include "main.h"
/* LED��ʾ */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* �������ʾ */
uchar Seg_Slow_Down;                                 // ����ܼ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ��ֵ
uchar Seg_Pos;                                       // �����ָʾ
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};       // ĳλ�Ƿ���ʾС����

/* ���̷��� */
uchar Key_Slow_Down;

/* ʱ�䷽�� */
uint time_all_1s;
uchar time_200ms;
/* ���� */
uchar Dis_value;
uchar Dis_para = 30;
/* ��ʾ */
uchar Seg_show_mode; // 0 ������ʾ 1 ������ʾ

/* �ж� */
bit Send_data_flag;
bit Wring_flag;
bit Led_blink;
/* ���ݴ����� */
void Data_Proc()
{
    if (time_all_1s % 100 == 0)
    {
        // ��������ȡ
        Dis_value = Ut_Wave_Data();
        if (Dis_value > Dis_para)
            Wring_flag = 1;
        else
            Wring_flag = 0;
    }
}
/* ���̴����� */
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
        Seg_show_mode = (++Seg_show_mode) % 2;
    if (Seg_show_mode == 0)
        if (Key_Down == 8)
            Dis_para = Dis_value;
    if (Seg_show_mode == 1)
    {
        if (Key_Down == 12)
            Dis_para = (Dis_para >= 245) ? 255
                                         : Dis_para + 10;
        else if (Key_Down == 16)
            Dis_para = (Dis_para <= 10) ? 0
                                        : Dis_para - 10;
    }
    if (Key_Down == 9)
        Send_data_flag = 1;
}
/* ����ܴ����� */
void Seg_Proc()
{
    uchar i;
    if (time_all_1s % 100)
        return;
    Seg_Buf[0] = 11; // U
    Seg_Buf[1] = Seg_show_mode + 1;
    Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
    Seg_Buf[5] = (Seg_show_mode == 0) ? Dis_value / 100 % 10
                                      : Dis_para / 100 % 10;
    Seg_Buf[6] = (Seg_show_mode == 0) ? Dis_value / 10 % 10
                                      : Dis_para / 10 % 10;
    Seg_Buf[7] = (Seg_show_mode == 0) ? Dis_value % 10
                                      : Dis_para % 10;
    i = 5;
    while (i < 7)
    {
        if (Seg_Buf[i] == 0 && Seg_Buf[i - 1] == 10)
            Seg_Buf[i] = 10;
        else
            break;
        i++;
    }
}

/* LED������ */
void Led_Proc()
{
    ucLed[0] = (Seg_show_mode == 0);
    ucLed[1] = (Seg_show_mode == 1);
    ucLed[2] = Led_blink;
}

/* ���ڴ����� */
void Uart_Proc()
{
    if (time_all_1s % 200)
        return;
    if (Send_data_flag)
    {
        Send_data_flag = 0;
        printf("Distance:%bucm", Dis_value);
    }
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
    if (++time_all_1s == 1000)
        time_all_1s = 0;
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
    if (Wring_flag)
    {
        if (++time_200ms == 200)
        {
            time_200ms = 0;
            Led_blink ^= 1;
        }
    }
    else
    {
        time_200ms = 0;
        Led_blink = 0;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}
void Uart1_Isr(void) interrupt 4
{
}

void main()
{
    System_Init();
    Timer0_Init();
    Uart1_Init();
    while (1)
    {
        Key_Proc();
        Data_Proc();
        Seg_Proc();
        Uart_Proc();
        Led_Proc();
    }
}