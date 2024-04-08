#include "main.h"
/* LED��ʾ */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* �������ʾ */
uchar Seg_Buf[8] = {10 , 10, 10, 10, 10, 10, 10, 10}; // �������ʾ��ֵ
uchar Seg_Pos;                                      // �����ָʾ
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // ĳλ�Ƿ���ʾС����

/* ���ڷ��� */
uchar Uart_Slow_Down;
uchar Uart_Buf[3];   // ���ڽ��յ�������
uchar Uart_Rx_Index; // ���ڽ��յ������ݵ�ָ��

/* ʱ�䷽�� */
uint time_all_1s;
uchar time_100ms;

/* ���� */
uchar Seg_show_mode; // 0 �¶���ʾ 1 ��ѹ��ʾ

/* ���� */
uint T_value_10x;  // 10���¶�ֵ
uint V_value_100x; // 100����ѹֵ

/* �ж� */
bit Data_send_flag;   // �ж��Ƿ�Ӧ�÷�������
bit Lock_uart_change; // �����޸�ҳ�湦���Ƿ�����
bit Led_blink_flag;   // ��˸��־

/* ���ݴ����� */
void Data_Proc()
{
    if (time_all_1s % 100 == 0)
    {
        // AD��ȡ
        V_value_100x = (float)Ad_Read(0x03) / 51.0 * 100;
    }
    if (time_all_1s % 500 == 0)
    {
        // �¶ȶ�ȡ
        T_value_10x = rd_temperature() * 10;
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
    if (Key_Down == 12)
        Data_send_flag = 1;
    if (Key_Down == 4)
        Lock_uart_change = 1;
    if (Lock_uart_change && Key_Down == 5)
        Lock_uart_change = 0;
}
/* ����ܴ����� */
void Seg_Proc()
{
    if (time_all_1s % 20)
        return;
    Seg_Buf[0] = 11; // U
    Seg_Buf[1] = Seg_show_mode + 1;
    Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
    if (Seg_show_mode == 0)
    {
        Seg_Point[5] = 0;
        Seg_Point[6] = 1;
        Seg_Buf[5] = T_value_10x / 100 % 10;
        Seg_Buf[6] = T_value_10x / 10 % 10;
        Seg_Buf[7] = T_value_10x % 10;
    }
    else
    {
        Seg_Point[5] = 1;
        Seg_Point[6] = 0;
        Seg_Buf[5] = V_value_100x / 100 % 10;
        Seg_Buf[6] = V_value_100x / 10 % 10;
        Seg_Buf[7] = V_value_100x % 10;
    }
}

/* LED������ */
void Led_Proc()
{
    ucLed[0] = (Seg_show_mode == 0);
    ucLed[1] = (Seg_show_mode == 1);
    ucLed[2] = Led_blink_flag;
    Relay(T_value_10x >= 280);
    Beep(V_value_100x > 360);
}

/* ���ڴ����� */
void Uart_Proc()
{
    if (time_all_1s % 200)
        return;
    if (Data_send_flag)
    {
        Data_send_flag = 0;
        if (Seg_show_mode == 0)
            printf("TEMP:%0.1f��", (float)T_value_10x / 10.0);
        else
            printf("Voltage:%0.2fV", (float)V_value_100x / 100.0);
    }
    if (Lock_uart_change == 0)
    {
        // δ������
        if (Uart_Buf[0] == 'A')
            Seg_show_mode = 0;
        if (Uart_Buf[0] == 'B')
            Seg_show_mode = 1;
        memset(Uart_Buf, 0, 3);
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
    if (Lock_uart_change)
    {
        if (++time_100ms == 100)
        {
            time_100ms = 0;
            Led_blink_flag ^= 1;
        }
    }
    else
    {
        time_100ms = 0;
        Led_blink_flag = 0;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

/* �����жϷ����� */
void Uart_ISR(void) interrupt 4
{
    if (RI == 1) // ���ڽ��յ�����
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
void main()
{
    System_Init();
    Timer0_Init();
    Uart1_Init();
    rd_temperature();
    Delay750ms();
    T_value_10x = rd_temperature() * 10;
		Delay750ms();
    while (1)
    {
        Key_Proc();
        Data_Proc();
        Seg_Proc();
        Uart_Proc();
        Led_Proc();
    }
}