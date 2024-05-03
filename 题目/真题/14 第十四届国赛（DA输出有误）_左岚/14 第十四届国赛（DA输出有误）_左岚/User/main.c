#include "main.h"
/* LED��ʾ */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* �������ʾ */                                     // ����ܼ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ��ֵ
uchar Seg_Pos;                                       // �����ָʾ
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};       // ĳλ�Ƿ���ʾС����
/* ���� */
uint T_value_10x;    // �¶Ȳ���ֵ10��
uint Dis_value;      // Ĭ��Ϊcm
uchar Para_Dis;      // �������
uchar Para_T;        // �¶Ȳ���
char Dis_Cail;       // ����У׼ֵ
uint Speed;          // �����ٶ�
uchar DAC_limit_10x; // 10��dac����
uint Record_Data[12];
uchar Record_Data_Index;
/* ��ʾ */
uchar Seg_show_mode;  // 0 ��� 1 ���� 2 ����
uchar Dis_show_mode;  // 0 cm 1 m
uchar Para_show_mode; // 0 ���� 1 �¶�
uchar FAC_show_mode;  // 0 У׼ 1 ���� 2 DAC����
/* ʱ�� */
uint time_all_1s;
uint time_6s;
uint time_2s;
uchar time_100ms;
uchar time_500ms;
/* �ж� */
bit Record_flag;        // ���ڼ�¼�ı�־
bit Key_lock;           // ������ס
bit Key_Two_Press;      // ˫��������
bit Led_Blink_flag;     // LED��˸��־
bit Output_Record_flag; // �����¼����
void Recover_Sys()
{
    Seg_show_mode = 0;
    Para_Dis = 40;
    Para_T = 30;
    Dis_Cail = 0;
    Speed = 340;
    DAC_limit_10x = 10;
}
/* ���ݴ����� */
void Data_Proc()
{
    uchar DAC_temp_100x;
    if (time_all_1s % 500 == 0)
    {
        // ����ȡ
        Dis_value = Ut_Wave_Data(Dis_Cail, Speed);
        if (Record_flag)
        {
            if (Dis_value < 10)
                DAC_temp_100x = DAC_limit_10x * 10;
            else if (Dis_value >= 90)
                DAC_temp_100x = 500;
            else
                DAC_temp_100x = (5 - DAC_limit_10x) * (Dis_value - 90) * 100 / 80 + 500;
            Record_Data[Record_Data_Index] = DAC_temp_100x;
            Record_Data_Index = (++Record_Data_Index) % 12;
        }
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
    // ���ڼ�¼��ֱ�������ж���ʧЧ
    if (Record_flag)
        return;
    if (time_all_1s % 10)
        return;
    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;
    if (Key_Old == 89)
    {
        // ���ڿ�ʼ����
        Key_lock = 1;
        Key_Two_Press = 1;
        if (time_2s > 2000)
            Recover_Sys();
    }
    // �����̣����������
    if (Key_lock && Key_Old)
        return;
    Key_lock = 0;
    if (Key_Down == 4)
    {
        Seg_show_mode = (++Seg_show_mode) % 3;
        Dis_show_mode = FAC_show_mode = Para_show_mode = 0;
    }
    switch (Seg_show_mode)
    {
    case 0:
        /* ��� */
        if (Key_Down == 5)
            Dis_show_mode = (++Dis_show_mode) % 2;
        if (Key_Down == 8)
            Record_flag = 1;
        if (Key_Down == 9)
        {
            Output_Record_flag = 1;
            Record_Data_Index = 11;
        }
        break;
    case 1:
        /* ���� */
        if (Key_Down == 5)
            Para_show_mode = (++Para_show_mode) % 2;
        switch (Para_show_mode)
        {
        case 0:
            /* ���� */
            if (Key_Down == 8)
                Para_Dis = (Para_Dis == 90) ? 10
                                            : Para_Dis + 10;
            else if (Key_Down == 9)
                Para_Dis = (Para_Dis == 10) ? 90
                                            : Para_Dis - 10;
            break;

        case 1:
            /* �¶� */
            if (Key_Down == 8)
                Para_T = (Para_T == 80) ? 0
                                        : Para_T + 1;
            else if (Key_Down == 9)
                Para_T = (Para_T == 0) ? 80
                                       : Para_T - 1;
            break;
        }
        break;
    case 2:
        /* ���� */
        if (Key_Down == 5)
            FAC_show_mode = (++FAC_show_mode) % 3;
        switch (FAC_show_mode)
        {
        case 0:
            /* У׼ֵ */
            if (Key_Down == 8)
                Dis_Cail = (Dis_Cail == 90) ? -90
                                            : Dis_Cail + 5;
            else if (Key_Down == 9)
                Dis_Cail = (Dis_Cail == -90) ? 90
                                             : Dis_Cail - 5;
            break;
        case 1:
            /* ���� */
            if (Key_Down == 8)
                Speed = (Speed == 9990) ? 0
                                        : Speed + 10;
            else if (Key_Down == 9)
                Speed = (Speed == 10) ? 9990
                                      : Speed - 10;
            break;
        case 2:
            /* DAC���� */
            if (Key_Down == 8)
                DAC_limit_10x = (DAC_limit_10x == 20) ? 1
                                                      : DAC_limit_10x + 1;
            else if (Key_Down == 9)
                DAC_limit_10x = (DAC_limit_10x == 1) ? 20
                                                     : DAC_limit_10x - 1;
            break;
        }
        break;
    }
}
/* ����ܴ����� */
void Seg_Proc()
{
    uchar Dis_Cail_Temp;
    Dis_Cail_Temp = -Dis_Cail;
    if (time_all_1s % 20)
        return;
    switch (Seg_show_mode)
    {
    case 0:
        /* ��� */
        Seg_Point[1] = 1;
        Seg_Point[6] = 0;
        Seg_Buf[0] = T_value_10x / 100 % 10;
        Seg_Buf[1] = T_value_10x / 10 % 10;
        Seg_Buf[2] = T_value_10x % 10;
        Seg_Buf[3] = 11; //-
        if (Dis_show_mode == 0)
        {
            Seg_Point[5] = 0;
            // ��ʾcm
            Seg_Buf[4] = (Dis_value / 1000 == 0) ? 10
                                                 : Dis_value / 1000;
            Seg_Buf[5] = (Seg_Buf[4] == 10 && Dis_value / 100 % 10 == 0) ? 10
                                                                         : Dis_value / 100 % 10;
            Seg_Buf[6] = (Seg_Buf[5] == 10 && Dis_value / 10 % 10 == 0) ? 10
                                                                        : Dis_value / 10 % 10;
            Seg_Buf[7] = Dis_value % 10;
        }
        else
        {
            Seg_Point[5] = 1;
            // ��ʾm
            Seg_Buf[4] = (Dis_value < 1000) ? 10
                                            : Dis_value / 1000;
            Seg_Buf[5] = Dis_value / 100 % 10;
            Seg_Buf[6] = Dis_value / 10 % 10;
            Seg_Buf[7] = Dis_value % 10;
        }
        break;

    case 1:
        Seg_Point[1] = Seg_Point[5] = Seg_Point[6] = 0;
        Seg_Buf[0] = 12; // P
        Seg_Buf[1] = Para_show_mode + 1;
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
        Seg_Buf[6] = (Para_show_mode == 0) ? Para_Dis / 10
                                           : Para_T / 10;
        Seg_Buf[7] = (Para_show_mode == 0) ? Para_Dis % 10
                                           : Para_T % 10;

        break;
    case 2:
        Seg_Buf[0] = 13; // F
        Seg_Buf[1] = FAC_show_mode + 1;
        switch (FAC_show_mode)
        {
        case 0:
            Seg_Point[1] = Seg_Point[5] = Seg_Point[6] = 0;
            /* У׼ */
            Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
            if (Dis_Cail <= -10)
            {
                Seg_Buf[5] = 11; //-
                Seg_Buf[6] = Dis_Cail_Temp / 10 % 10;
                Seg_Buf[7] = Dis_Cail_Temp % 10;
            }
            else if (Dis_Cail < 0)
            {
                Seg_Buf[5] = 10;
                Seg_Buf[6] = 11; //-
                Seg_Buf[7] = Dis_Cail_Temp % 10;
            }
            else
            {
                Seg_Buf[5] = 50;
                Seg_Buf[6] = (Dis_Cail / 10 % 10 == 0) ? 10
                                                       : Dis_Cail / 10 % 10;
                Seg_Buf[7] = Dis_Cail % 10;
            }
            break;

        case 1:
            Seg_Point[1] = Seg_Point[5] = Seg_Point[6] = 0;
            /* ���� */
            Seg_Buf[2] = Seg_Buf[3] = 10;
            Seg_Buf[4] = (Speed / 1000 % 10 == 0) ? 10
                                                  : Speed / 1000 % 10;
            Seg_Buf[5] = (Seg_Buf[4] == 10 && Speed / 100 % 10 == 0) ? 10
                                                                     : Speed / 100 % 10;
            Seg_Buf[6] = Speed / 10 % 10;
            Seg_Buf[7] = Speed % 10;
            break;
        case 2:
            Seg_Point[1] = Seg_Point[5] = 0;
            Seg_Point[6] = 1;
            /* DAC���� */
            Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10;
            Seg_Buf[6] = DAC_limit_10x / 10 % 10;
            Seg_Buf[7] = DAC_limit_10x % 10;
            break;
        }
        break;
    }
}

/* LED������ */
void Led_Proc()
{
    uchar i;
    if (Output_Record_flag)
        Da_Write(Record_Data[11 - Record_Data_Index] * 51 / 100);
    switch (Seg_show_mode)
    {
    case 0:
        /* ��� */
        if (Dis_value > 255)
            memset(ucLed, 1, 8);
        else
        {
            for (i = 0; i < 8; i++)
            {
                ucLed[i] = Dis_value & (0x01 << i);
            }
        }
        break;

    case 1:
        /*  ���� */
        memset(ucLed, 0, 7);
        ucLed[7] = 1;
        break;

    case 2:
        /* ���� */
        memset(ucLed + 1, 0, 7);
        ucLed[0] = Led_Blink_flag;
        break;
    }
    Relay(Dis_value <= Para_Dis + 5 &&
          Dis_value >= Para_Dis - 5 &&
          T_value_10x < Para_T * 10);
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
    uchar i;
    if (++time_all_1s == 1000)
        time_all_1s = 0;
    Seg_Pos = (++Seg_Pos) % 8;
    if (Output_Record_flag)
    {
        if (Record_Data_Index == 0)
        {
            Output_Record_flag = 0;
            time_500ms = 0;
        }
        // �����ʱ
        if (++time_500ms == 500)
        {
            Record_Data_Index--;
            time_500ms = 0;
        }
    }
    else
    {
        time_500ms = 0;
    }
    // ��˫�������µ�ʱ��ʼ��ʱ
    if (Key_Two_Press)
    {
        if (++time_2s >= 2000)
            time_2s = 2001;
    }
    else
    {
        time_2s = 0;
    }
    // ��ʼ��¼
    if (Record_flag)
    {
        if (++time_6s == 6000)
        {
            time_6s = 0;
            Record_flag = 0;
        }
    }
    else
    {
        time_6s = 0;
    }
    if (++time_100ms == 100)
    {
        time_100ms = 0;
        Led_Blink_flag ^= 1;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    for (i = 0; i < 8; i++)
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
    Recover_Sys();
    rd_temperature();
    Delay750ms();
    Timer0_Init();
    while (1)
    {
        Data_Proc();
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}