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

/* ���� */
uchar Work_mode;   // 0 ˯�߷�20 1 ��Ȼ��30 2 ����70
bit Seg_show_mode; // 0 ��ʾ���� 1 ��ʾ�¶�

/* ʱ�� */
uchar Work_time;       // ����ʱ��
uchar time_mode_index; // 0->0�� 1->1�� 2->2��
uchar time_mode[3] = {0, 60, 120};
uchar Work_mode_P34[3] = {2, 3, 7};
uchar time_1ms;
uint time_1s;
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

    if (Key_Down == 4)
        Work_mode = (++Work_mode) % 3;
    if (Key_Down == 5)
    {
        time_mode_index = (++time_mode_index) % 3;
        Work_time = time_mode[time_mode_index];
    }
    if (Key_Down == 6)
        Work_time = 0;
    if (Key_Down == 7)
        Seg_show_mode ^= 1;
}
/* ����ܴ����� */
void Seg_Proc()
{
    uchar T_value;
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    Seg_Buf[0] = 11; //-
    Seg_Buf[2] = 11; //-
    Seg_Buf[3] = 10; // ��
    T_value = rd_temperature();
    if (Seg_show_mode == 0)
    {
        // �����Ƿ�����ʾҳ��
        Seg_Buf[1] = Work_mode + 1;
        Seg_Buf[4] = Work_time / 1000 % 10;
        Seg_Buf[5] = Work_time / 100 % 10;
        Seg_Buf[6] = Work_time / 10 % 10;
        Seg_Buf[7] = Work_time % 10;
    }
    else
    {
        // �������¶���ʾҳ��
        Seg_Buf[1] = 4;
        Seg_Buf[4] = 10; // ��
        Seg_Buf[5] = T_value / 10 % 10;
        Seg_Buf[6] = T_value % 10;
        Seg_Buf[7] = 12; // C
    }
}

/* LED������ */
void Led_Proc()
{
    if (Work_time)
    {
        // ����ʱû�н���
        ucLed[0] = (Work_mode == 0);
        ucLed[1] = (Work_mode == 1);
        ucLed[2] = (Work_mode == 2);
    }
    else
        memset(ucLed, 0, 3);
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

void Timer1_Init(void) // 100΢��@12.000MHz
{
    AUXR |= 0x40; // ��ʱ��ʱ��1Tģʽ
    TMOD &= 0x0F; // ���ö�ʱ��ģʽ
    TL1 = 0x50;   // ���ö�ʱ��ʼֵ
    TH1 = 0xFB;   // ���ö�ʱ��ʼֵ
    TF1 = 0;      // ���TF1��־
    TR1 = 1;      // ��ʱ��1��ʼ��ʱ
    ET1 = 1;      // ʹ�ܶ�ʱ��1�ж�
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
    if (Work_time)
    {
        if (++time_1s == 1000)
        {
            time_1s = 0;
            Work_time--;
        }
    }
    // ��ֹ�����趨����ֲ���1s��bug
    else
    {
        time_1s = 0;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}
void Timer1_Isr(void) interrupt 3
{
    // ������ʱ��û��Ϊ0��ʱ��
    if (Work_time)
    {
        if (++time_1ms == 10)
            time_1ms = 0;
        // ����ʱС�����ǵ�ʱ�򣬾������ߵ�ƽ������͵�ƽ������ռ�ձ�
        if (time_1ms < Work_mode_P34[Work_mode])
            P34 = 1;
        else
            P34 = 0;
    }
    // ����ʱ����㣬Ϊ�˱���bug�����ǰ�ȫ������
    else
    {
        time_1ms = 0;
        P34 = 0;
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
    Timer1_Init();
    rd_temperature();
    Delay750ms();
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}