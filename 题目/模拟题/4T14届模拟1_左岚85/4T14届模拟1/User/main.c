#include "main.h"
/* LED��ʾ */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* �������ʾ */
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ��ֵ
uchar Seg_Pos;                                      // �����ָʾ
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // ĳλ�Ƿ���ʾС����

/* ���ڷ��� */
uchar Uart_Buf[10];  // ���ڽ��յ�������
uchar Uart_Rx_Index; // ���ڽ��յ������ݵ�ָ��

/* ʱ�䷽�� */
uint time_all_1s;
uchar Sys_Tick;
uchar time_100ms;
/* �ж� */
bit Uart_Flag;
bit Wring_Flag;
bit Led_Blink_Flag;
/* ��ʾ */
uchar Seg_show_mode; // 0 �����ֱ� 1 ������ʾ
/* ���� */
uint Noise_Value_10x;
uchar Noise_Para = 65;
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
    if (Key_Down == 12)
        Seg_show_mode = (++Seg_show_mode) % 2;
    if (Seg_show_mode == 1)
    {
        if (Key_Down == 16)
            Noise_Para = (Noise_Para == 90) ? 0 : Noise_Para + 5;
        else if (Key_Down == 17)
            Noise_Para = (Noise_Para == 0) ? 90 : Noise_Para - 5;
    }
}
/* ����ܴ����� */
void Seg_Proc()
{
    if (time_all_1s % 50)
        return;
    Noise_Value_10x = Ad_Read(0x03) * 180 / 51;
    Wring_Flag = Noise_Value_10x > Noise_Para * 10;

    Seg_Buf[0] = 11; // U
    Seg_Buf[1] = Seg_show_mode + 1;
    switch (Seg_show_mode)
    {
    case 0:
        /* �����ֱ���ʾ */
        Seg_Point[6] = 1;
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
        Seg_Buf[5] = (Noise_Value_10x / 100 % 10 == 0) ? 10
                                                       : Noise_Value_10x / 100 % 10;
        Seg_Buf[6] = Noise_Value_10x / 10 % 10;
        Seg_Buf[7] = Noise_Value_10x % 10;
        break;

    case 1:
        /* �ֱ����� */
        Seg_Point[6] = 0;
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
        Seg_Buf[6] = (Noise_Para / 10 % 10 == 0) ? 10
                                                 : Noise_Para / 10 % 10;
        Seg_Buf[7] = Noise_Para % 10;
        break;
    }
}

/* LED������ */
void Led_Proc()
{
    ucLed[0] = (Seg_show_mode == 0);
    ucLed[1] = (Seg_show_mode == 1);
    ucLed[7] = Led_Blink_Flag;
}

/* ���ڴ����� */
void Uart_Proc()
{
    if (Uart_Rx_Index == 0)
        // û�н��յ��κ�����
        return;
    if (Sys_Tick >= 10)
    {
        Sys_Tick = Uart_Flag = 0;
        if (Seg_show_mode == 0)
        {
            if (Uart_Buf[0] == 'R' && Uart_Buf[1] == 'e' && Uart_Buf[2] == 't' &&
                Uart_Buf[3] == 'u' && Uart_Buf[4] == 'r' && Uart_Buf[5] == 'n')
                printf("Noises:%0.1fdB", (float)Noise_Value_10x / 10.0);
        }
        memset(Uart_Buf, 0, Uart_Rx_Index);
        Uart_Rx_Index = 0;
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
    if (Uart_Flag)
        Sys_Tick++;
    if (Wring_Flag)
    {
        if (++time_100ms == 100)
        {
            time_100ms = 0;
            Led_Blink_Flag ^= 1;
        }
    }
    else
    {
        time_100ms = 0;
        Led_Blink_Flag = 0;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

/* �����жϷ����� */
void Uart_ISR(void) interrupt 4
{
    if (RI == 1) // ���ڽ��յ�����
    {
        Uart_Flag = 1;
        Sys_Tick = 0;
        Uart_Buf[Uart_Rx_Index] = SBUF;
        Uart_Rx_Index++;
        RI = 0;
    }
    if (Uart_Rx_Index > 10)
        Uart_Rx_Index = 0;
}
void main()
{
    System_Init();
    Timer0_Init();
    Uart1_Init();

    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Uart_Proc();
        Led_Proc();
    }
}