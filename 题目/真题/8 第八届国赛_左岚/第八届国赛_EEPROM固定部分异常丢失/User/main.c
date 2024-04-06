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
uchar Dis_Data[10];     // �洢����
uchar Dis_show_index;   // ��ʾ�����±�
uchar Dis_Data_index;   // �洢�����±�
uchar Dis_new, Dis_old; // ��һ�κ���һ�β����Ľ��
uchar Blind_area;       // ����ä��
uchar Led_blink_cnt;    // ��˸����
/* ��ʾ */
uchar Seg_show_mode; // 0 �����ʾ 1 ���� 2 ��������
/* �ж� */
bit Led_blink_start; // �Ƿ�ʼ��˸
bit Led_blink_flag;  // ��˸��־λ
bit Work_mode;       // 0 ��һ�� 1 ��һ��+��һ��
/* ���̴����� */
void Key_Proc()
{
    static uchar Key_Val, Key_Down, Key_Up, Key_Old;
    uint DA_out = 0;
    if (time_all_1s % 10)
        return;
    Key_Val = Key_Read();
    Key_Down = Key_Val & (Key_Old ^ Key_Val);
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
    Key_Old = Key_Val;
    switch (Seg_show_mode)
    {
    case 0:
        /* ������ */
        if (Key_Down == 4)
        {
            Led_blink_start = 1;
            Dis_new = Ut_Wave_Data();
            Dis_Data[Dis_Data_index] = Dis_new;
            Dis_Data_index = (++Dis_Data_index) % 10; // 0-9
            if (Dis_Data_index == 0)
            {
                // ����д��Ϊ�˷�ֹ���
                EEPROM_Write(&Dis_new, 10, 1);
                // ���︳ֵ����Ϊ������һ�ζϵ�������ݱ��棬����Ҫ���ϴε�����
                Dis_old = Dis_Data[9];
            }
            else
            {
                EEPROM_Write(&Dis_new, Dis_Data_index, 1);
                // ���︳ֵ����Ϊ������һ�ζϵ�������ݱ��棬����Ҫ���ϴε�����
                Dis_old = Dis_Data[Dis_Data_index - 1];
            }

            if (Dis_new < Blind_area)
                DA_out = 0;
            else
                DA_out = (Dis_new - Blind_area) * 51 * 0.02;
            if (DA_out >= 255)
                DA_out = 255;
            Da_Write(DA_out);
        }
        if (Key_Down == 7)
            Work_mode ^= 1;
        if (Key_Down == 5)
            // �л�����
            Seg_show_mode = 1;
        if (Key_Down == 6)
            // �л�����
            Seg_show_mode = 2;
        break;

    case 1:
        /* ���Խ��� */
        if (Key_Down == 5)
            // �л����
            Seg_show_mode = 0;
        if (Key_Down == 7)
            // ��ҳ
            Dis_show_index = (++Dis_show_index) % 10;
        break;
    case 2:
        /* �������ý��� */
        if (Key_Down == 6)
        {
            // �л����
            Seg_show_mode = 0;
            EEPROM_Write(&Blind_area, 0, 1);
        }
        if (Key_Down == 7)
            // ѭ�����
            Blind_area = (Blind_area == 90) ? 0 : Blind_area + 10;
        break;
    }
}
/* ����ܴ����� */
void Seg_Proc()
{
    uint Work_data = 0;

    if (time_all_1s % 20)
        return;
    switch (Seg_show_mode)
    {
    case 0:
        /* ������ */
        Seg_Buf[0] = Work_mode;
        Seg_Buf[1] = 10;
        if (Work_mode)
            Work_data = Dis_old + Dis_new;
        else
            Work_data = Dis_old;
        Seg_Buf[2] = (Work_data / 100 % 10 == 0) ? 10
                                                 : Work_data / 100 % 10;
        Seg_Buf[3] = (Work_data / 10 % 10 == 0 &&
                      Seg_Buf[2] == 10)
                         ? 10
                         : Work_data / 10 % 10;
        Seg_Buf[4] = Work_data % 10;
        Seg_Buf[5] = Dis_new / 100 % 10;
        Seg_Buf[6] = Dis_new / 10 % 10;
        Seg_Buf[7] = Dis_new % 10;
        break;

    case 1:
        /* ���Խ��� */
        Seg_Buf[0] = (Dis_show_index + 1) / 10 % 10;
        Seg_Buf[1] = (Dis_show_index + 1) % 10;
        Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
        Seg_Buf[5] = Dis_Data[Dis_show_index] / 100 % 10;
        Seg_Buf[6] = Dis_Data[Dis_show_index] / 10 % 10;
        Seg_Buf[7] = Dis_Data[Dis_show_index] % 10;
        break;
    case 2:
        /* �������� */
        Seg_Buf[0] = 11; // F
        Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = 10;
        Seg_Buf[5] = (Blind_area / 100 % 10 == 0) ? 10
                                                  : Blind_area / 100 % 10;
        Seg_Buf[6] = (Blind_area / 10 % 10 == 0 &&
                      Seg_Buf[5] == 10)
                         ? 10
                         : Blind_area / 10 % 10;
        Seg_Buf[7] = Blind_area % 10;
        break;
    }
}

/* LED������ */
void Led_Proc()
{

    ucLed[0] = Led_blink_flag;
    ucLed[6] = (Seg_show_mode == 2);
    ucLed[7] = (Seg_show_mode == 1);
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
    if (Led_blink_start)
    {
        if (++time_200ms == 200)
        {
            time_200ms = 0;
            Led_blink_flag ^= 1;
            Led_blink_cnt++;
        }
        if (Led_blink_cnt == 20)
            Led_blink_start = 0; // ֹͣ��˸
    }
    else
    {
        time_200ms = 0;
        Led_blink_cnt = 0;
        Led_blink_flag = 0;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

void Delay200ms(void) //@12.000MHz
{
    unsigned char data i, j, k;

    _nop_();
    _nop_();
    i = 10;
    j = 31;
    k = 147;
    do
    {
        do
        {
            while (--k)
                ;
        } while (--j);
    } while (--i);
}

uchar passwd = 123;
uchar input_passwd;
void main()
{
    uchar i;
    System_Init();
    Timer0_Init();
    EEPROM_Read(&input_passwd,16,1);
    if (input_passwd != passwd) // У��ʧ�ܣ�֮ǰδд������1/256���ʳ�����
    {
        EEPROM_Write(&passwd, 16, 1);
    }
    else // У��ͨ������ȡ������Ҫ������
    {
        EEPROM_Read(Dis_Data, 1, 7);
        EEPROM_Read(&Dis_Data[8], 8, 3);
    }

    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}