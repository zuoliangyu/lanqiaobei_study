#include "main.h"
/* LED��ʾ */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* �������ʾ */
uchar Seg_Slow_Down;                                 // ����ܼ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ��ֵ
uchar Seg_Pos;                                       // �����ָʾ
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};       // ĳλ�Ƿ���ʾС����

/* ʱ�䷽�� */
uchar ucRtc[3] = {0x23, 0x59, 0x50};    // ��ʼ��ʱ��ʱ��23:59:50
uchar alarmRtc[3] = {0x00, 0x00, 0x00}; // ��ʼ������ʱ��00:00:00
uchar setUcRtc[3];                      // ʱ���������� ʮ����
uchar setAlarmRtc[3];                   // ������������ ʮ����
uchar setUcRtc_index;
uchar setAlarmRtc_index;
/* ���̷��� */
uchar Key_Slow_Down;

/* ʱ�䷽�� */
uint time_all_1s;
uint time_blink_1s; // 1s��˸
uchar time_200ms;
uint time_5s;
/* ���ݷ��� */
uchar T_value; // �¶�ֵ
/* ��ʾ���� */
uchar Seg_show_mode; // 0 ʱ����ʾ 1 ʱ������ 2 �������� 3 �¶���ʾ
/* �ж� */
bit Seg_blink_flag; // �������˸��־
bit Led_blink_flag; // LED��˸��־
bit ring_flag;      // ���Ӵ�����־
/* ���ݴ����� */
void Data_Proc()
{
    if (time_all_1s % 500 == 0)
    {
        // ʱ���ȡ
        Read_Rtc(ucRtc);
        if (ucRtc[0] == alarmRtc[0] && ucRtc[1] == alarmRtc[1] && ucRtc[2] == alarmRtc[2])
            // ���Ӵ���
            ring_flag = 1;
    }
    if (time_all_1s % 500 == 0)
    {
        // �¶ȶ�ȡ
        T_value = rd_temperature();
    }
}
/* ���̴����� */
void Key_Proc()
{
    static uchar Key_Val, Key_Down, Key_Up, Key_Old;
    uchar i;
    if (time_all_1s % 10)
        return;
    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;
    switch (Seg_show_mode)
    {
    case 0:
        if (Key_Down == 7)
        {
            // ����ʱ������
            for (i = 0; i < 3; i++)
            {
                setUcRtc[i] = ucRtc[i] / 16 * 10 + ucRtc[i] % 16;
            }
            Seg_show_mode = 1;
        }
        else if (Key_Down == 6)
        {
            // ������������
            for (i = 0; i < 3; i++)
            {
                setAlarmRtc[i] = alarmRtc[i] / 16 * 10 + alarmRtc[i] % 16;
            }
            Seg_show_mode = 2;
        }
        if (Key_Old == 4)
            // �����¶���ʾ
            Seg_show_mode = 3;
        break;

    case 1:
        if (Key_Down == 7)
            setUcRtc_index++;
        if (setUcRtc_index >= 3)
        {
            // ����ʱ����ʾ
            Seg_show_mode = 0;
            setUcRtc_index = 0;
            for (i = 0; i < 3; i++)
                ucRtc[i] = setUcRtc[i] / 10 << 4 | setUcRtc[i] % 10;
            Set_Rtc(ucRtc);
        }
        if (Key_Down == 5)
        {
            if (setUcRtc_index == 0)
                // �޸�Сʱ
                setUcRtc[setUcRtc_index] = (setUcRtc[setUcRtc_index] >= 24) ? 24
                                                                            : setUcRtc[setUcRtc_index] + 1;
            else
                // �޸ķ���
                setUcRtc[setUcRtc_index] = (setUcRtc[setUcRtc_index] >= 60) ? 60
                                                                            : setUcRtc[setUcRtc_index] + 1;
        }
        else if (Key_Down == 4)
            // �޸�ʱ����
            setUcRtc[setUcRtc_index] = (setUcRtc[setUcRtc_index] == 0) ? 0
                                                                       : setUcRtc[setUcRtc_index] - 1;
        break;
    case 2:
        if (Key_Down == 6)
            setAlarmRtc_index++;
        if (setAlarmRtc_index >= 3)
        {
            // ����ʱ����ʾ
            Seg_show_mode = 0;
            setAlarmRtc_index = 0;
            for (i = 0; i < 3; i++)
                alarmRtc[i] = setAlarmRtc[i] / 10 << 4 | setAlarmRtc[i] % 10;
        }
        if (Key_Down == 5)
        {
            if (setAlarmRtc_index == 0)
                // �޸�Сʱ
                setAlarmRtc[setAlarmRtc_index] = (setAlarmRtc[setAlarmRtc_index] >= 24) ? 24
                                                                                        : setAlarmRtc[setAlarmRtc_index] + 1;
            else
                // �޸ķ���
                setAlarmRtc[setUcRtc_index] = (setAlarmRtc[setUcRtc_index] >= 60) ? 60
                                                                                  : setAlarmRtc[setAlarmRtc_index] + 1;
        }
        else if (Key_Down == 4)
            // �޸�ʱ����
            setAlarmRtc[setUcRtc_index] = (setAlarmRtc[setAlarmRtc_index] == 0) ? 0
                                                                                : setAlarmRtc[setAlarmRtc_index] - 1;
        break;
    case 3:
        if (Key_Up == 4)
            // ����ʱ����ʾ
            Seg_show_mode = 0;
        break;
    }
    if (ring_flag && (Key_Down != 0))
        ring_flag = 0;
}
/* ����ܴ����� */
void Seg_Proc()
{
    if (time_all_1s % 20)
        return;
    switch (Seg_show_mode)
    {
    case 0:
        /* ʱ����ʾ */
        Seg_Buf[0] = ucRtc[0] / 16;
        Seg_Buf[1] = ucRtc[0] % 16;
        Seg_Buf[2] = 11; //-
        Seg_Buf[3] = ucRtc[1] / 16;
        Seg_Buf[4] = ucRtc[1] % 16;
        Seg_Buf[5] = 11; //-
        Seg_Buf[6] = ucRtc[2] / 16;
        Seg_Buf[7] = ucRtc[2] % 16;
        break;

    case 1:
        /* ʱ������ */
        Seg_Buf[0] = setUcRtc[0] / 10;
        Seg_Buf[1] = setUcRtc[0] % 10;
        Seg_Buf[2] = 11; //-
        Seg_Buf[3] = setUcRtc[1] / 10;
        Seg_Buf[4] = setUcRtc[1] % 10;
        Seg_Buf[5] = 11; //-
        Seg_Buf[6] = setUcRtc[2] / 10;
        Seg_Buf[7] = setUcRtc[2] % 10;
        Seg_Buf[setUcRtc_index * 3] = Seg_blink_flag ? 10
                                                     : Seg_Buf[setUcRtc_index * 3];
        Seg_Buf[setUcRtc_index * 3 + 1] = Seg_blink_flag ? 10
                                                         : Seg_Buf[setUcRtc_index * 3 + 1];
        break;
    case 2:
        /* �������� */
        Seg_Buf[0] = setAlarmRtc[0] / 10;
        Seg_Buf[1] = setAlarmRtc[0] % 10;
        Seg_Buf[2] = 11; //-
        Seg_Buf[3] = setAlarmRtc[1] / 10;
        Seg_Buf[4] = setAlarmRtc[1] % 10;
        Seg_Buf[5] = 11; //-
        Seg_Buf[6] = setAlarmRtc[2] / 10;
        Seg_Buf[7] = setAlarmRtc[2] % 10;
        Seg_Buf[setAlarmRtc_index * 3] = Seg_blink_flag ? 10
                                                        : Seg_Buf[setAlarmRtc_index * 3];
        Seg_Buf[setAlarmRtc_index * 3 + 1] = Seg_blink_flag ? 10
                                                            : Seg_Buf[setAlarmRtc_index * 3 + 1];
        break;
        break;
    case 3:
        /* �¶���ʾ */
        memset(Seg_Buf, 10, 5);
        Seg_Buf[5] = T_value / 10;
        Seg_Buf[6] = T_value % 10;
        Seg_Buf[7] = 12; // C
        break;
    }
}

/* LED������ */
void Led_Proc()
{
    ucLed[0] = Led_blink_flag;
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
    if (Seg_show_mode == 1 || Seg_show_mode == 2)
    {
        // ���������ý����ʱ�����ǿ�ʼ���м�ʱ
        if (++time_blink_1s == 1000)
        {
            // 1s��˸
            time_blink_1s = 0;
            Seg_blink_flag ^= 1;
        }
    }
    else
    {
        // �������ý����ʱ�����ǲ����м�ʱ
        time_blink_1s = 0;
        Seg_blink_flag = 0;
    }
    if (ring_flag)
    {
        // �����Ӵ�����ʱ�����ǿ�ʼ��ʱ
        if (++time_200ms == 200)
            Led_blink_flag ^= 1;
        if (++time_5s == 5000)
            ring_flag = 0;
    }
    else
    {
        // ��ֹ��Ϊ���°�����ڶ��μ�ʱ��bug
        Led_blink_flag = 0;
        time_5s = 0;
        time_200ms = 0;
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
    rd_temperature();
    Delay750ms();
    while (1)
    {
        Data_Proc();
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}