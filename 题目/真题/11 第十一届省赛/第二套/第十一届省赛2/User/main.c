#include "main.h"
/* LED��ʾ */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* �������ʾ */
uchar Seg_Slow_Down;                                // ����ܼ���
uchar Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ��ֵ
uchar Seg_Pos;                                      // �����ָʾ
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // ĳλ�Ƿ���ʾС����

/* ���̷��� */
uchar Key_Slow_Down;

/* ��ʾ */
uchar Seg_show_mode; // 0 ������ʾ 1 ��������
/* ���� */
uchar T_value;                          // �¶Ȳ���
uchar T_para_max = 30, T_para_min = 20; // �¶Ȳ���
uchar T_para_max_set, T_para_min_set;   // �¶Ȳ�������
/* �ж� */
bit para_mode;      // 0 ѡ���¶����� 1ѡ���¶�����
bit error_data_set; // ������������
#define N 10
uint data_array[N]; // ���ڴ�С
uint sum_temp;      // �ܺ�
uchar index_temp;   // ����
uchar arr_count;    // ������������

uint filter(uint new_data)
{
    sum_temp -= data_array[index_temp];
    data_array[index_temp] = new_data;
    sum_temp += data_array[index_temp];
    index_temp = (++index_temp) % N;                    // ��֤index_temp��0~N-1֮����ת
    arr_count = (++arr_count == N + 1) ? N : arr_count; // ���������е�Ԫ�ظ���
    return sum_temp / arr_count;
}

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
    {
        Seg_show_mode = (++Seg_show_mode) % 2;
        para_mode = 0;
        if (Seg_show_mode == 1)
        {
            // ������ֵ������ֵ
            T_para_max_set = T_para_max;
            T_para_min_set = T_para_min;
        }
        if (Seg_show_mode == 0)
        {
            // ���в��������ж�
            if (T_para_max_set >= T_para_min_set)
            {
                error_data_set = 0;
                T_para_max = T_para_max_set;
                T_para_min = T_para_min_set;
            }
            else
            {
                error_data_set = 1;
            }
        }
    }
    if (Key_Down == 5)
        para_mode ^= 1;
    if (Seg_show_mode == 1)
    {
        if (para_mode == 0)
        {
            // �����¶�����
            if (Key_Down == 6)
                T_para_min_set = (++T_para_min_set) % 100;
            if (Key_Down == 7)
                T_para_min_set = (T_para_min_set == 0) ? 99 : T_para_min_set - 1;
        }
        else
        {
            // �����¶�����
            if (Key_Down == 6)
                T_para_max_set = (++T_para_max_set) % 100;
            if (Key_Down == 7)
                T_para_max_set = (T_para_max_set == 0) ? 99 : T_para_max_set - 1;
        }
    }
}
/* ����ܴ����� */
void Seg_Proc()
{
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    T_value = filter(rd_temperature());
    if (T_value > T_para_max)
        Da_Write(4 * 51);
    else if (T_value < T_para_min)
        Da_Write(2 * 51);
    else
        Da_Write(3 * 51);
    switch (Seg_show_mode)
    {
    case 0:
        /* ������ʾ */
        Seg_Buf[0] = 11; // C
        memset(Seg_Buf + 1, 10, 5);
        Seg_Buf[6] = T_value / 10 % 10;
        Seg_Buf[7] = T_value % 10;
        break;

    case 1:
        /* �������� */
        Seg_Buf[0] = 12; // P
        Seg_Buf[1] = Seg_Buf[2] = 10;
        Seg_Buf[3] = T_para_max_set / 10 % 10;
        Seg_Buf[4] = T_para_max_set % 10;
        Seg_Buf[5] = 10;
        Seg_Buf[6] = T_para_min_set / 10 % 10;
        Seg_Buf[7] = T_para_min_set % 10;
        break;
    }
}

/* LED������ */
void Led_Proc()
{
    ucLed[0] = (T_value > T_para_max);
    ucLed[1] = (T_value < T_para_min);
    ucLed[2] = (T_value < T_para_max && T_value > T_para_min);
    ucLed[3] = error_data_set;
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
    rd_temperature();
    Delay750ms();
    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}