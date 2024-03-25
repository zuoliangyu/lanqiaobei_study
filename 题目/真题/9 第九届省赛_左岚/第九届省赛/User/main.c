#include "main.h"
/* LED��ʾ */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* �������ʾ */
uchar Seg_Slow_Down;                                // ����ܼ���
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ��ֵ
uchar Seg_Pos;                                      // �����ָʾ
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // ĳλ�Ƿ���ʾС����

/* ���̷��� */
uchar Key_Slow_Down;

/* ���� */

/* ��ʾ */
uchar Led_show_mode;      // 0 ���ҵ�ѭ�� 1 �ҵ���ѭ�� 2 .. 3 ..
uint Flow_interval = 400; // 400-1200
uchar Seg_show_mode;      // 0 �����ȫ�� 1 ģʽ������� 2 ��ת�������
uchar Led_level;          // 0 1 2 3
uchar Led_show_index;     // ��ʾ������ܵĵڼ��ԣ�������ת12ģʽֱ��ʹ�ã�34ģʽ�任ʹ��

/* �ж� */
bit Flow_flag;   // �Ƿ�����ת
bit Seg_flicker; // �������˸
bit Show_level;  // �Ƿ���ʾ�ȼ�
uchar Pwm_value;
/* EEPROM���� */
uchar old_passwd;
uchar passwd = 123;

/* ʱ�� */
uint time_800ms;
uint time_flow; // ��ת��ʱ
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
    if (Key_Down == 7)
        Flow_flag ^= 1;
    if (Key_Down == 6)
        Seg_show_mode = (++Seg_show_mode) % 3;
    if (Key_Down == 5)
    {
        if (Seg_show_mode == 1)
        {
            Led_show_mode = (Led_show_mode + 1) % 4;
        }
        else if (Seg_show_mode == 2)
        {
            Flow_interval = (Flow_interval + 100 > 1200) ? 400 : Flow_interval + 100;
        }
    }

    if (Key_Down == 4)
    {
        if (Seg_show_mode == 1)
        {
            Led_show_mode = (Led_show_mode == 0) ? 3 : Led_show_mode - 1;
        }
        else if (Seg_show_mode == 2)
        {
            Flow_interval = (Flow_interval - 100 < 400) ? 1200 : Flow_interval - 100;
        }
    }
    if (Seg_show_mode == 0)
        if (Key_Old == 4)
        {
            Show_level = 1;
        }
        // �������û�а���S4
        else
        {
            Show_level = 0;
        }
}
/* ����ܴ����� */
void Seg_Proc()
{
    uchar i;
    if (Seg_Slow_Down)
        return;
    Seg_Slow_Down = 1;
    Led_level = Ad_Read(0x03) / 64;
    if (Seg_show_mode == 0)
    {
        if (Show_level)
        {
            memset(Seg_Buf, 10, 6);     //
            Seg_Buf[6] = 11;            //-
            Seg_Buf[7] = Led_level + 1; //
        }
        else
            memset(Seg_Buf, 10, 8); // ȫ��
    }
    else
    {
        Seg_Buf[0] = 11; //-
        Seg_Buf[1] = Led_show_mode + 1;
        Seg_Buf[2] = 11; //-
        Seg_Buf[3] = 10;
        Seg_Buf[4] = (Flow_interval / 1000 == 0) ? 10 : Flow_interval / 1000 % 10;
        Seg_Buf[5] = Flow_interval / 100 % 10;
        Seg_Buf[6] = Flow_interval / 10 % 10;
        Seg_Buf[7] = Flow_interval % 10;
        if (Seg_show_mode == 1)
        {
            for (i = 0; i < 3; i++)
                Seg_Buf[i] = (Seg_flicker) ? Seg_Buf[i] : 10;
        }
        else if (Seg_show_mode == 2)
        {
            for (i = 4; i < 8; i++)
                Seg_Buf[i] = (Seg_flicker) ? Seg_Buf[i] : 10;
        }
    }
}

/* LED������ */
void Led_Proc()
{
    uchar i;
    if (3 * (Led_level) > Pwm_value + 1)
    {
        if (Flow_flag)
        {
            switch (Led_show_mode)
            {
            case 0:
                for (i = 0; i < 8; i++)
                    ucLed[i] = (i == Led_show_index);
                break;
            case 1:
                for (i = 0; i < 8; i++)
                    ucLed[i] = (7 - i == Led_show_index);
                break;
            case 2:
                for (i = 0; i < 4; i++)
                {
                    ucLed[i] = (i == Led_show_index);
                    ucLed[7 - i] = (i == Led_show_index);
                }
                break;
            case 3:
                for (i = 0; i < 4; i++)
                {
                    ucLed[3 - i] = (i == Led_show_index);
                    ucLed[4 + i] = (i == Led_show_index);
                }
                break;
            }
        }
        else
        {
            memset(ucLed, 0, 8);
        }
    }
    else
    {
        memset(ucLed, 0, 8);
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
    if (++Key_Slow_Down == 10)
        Key_Slow_Down = 0;
    if (++Seg_Slow_Down == 200)
        Seg_Slow_Down = 0;
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
    if (++Pwm_value == 12)
        Pwm_value = 0;
    if (++time_800ms == 800)
    {
        Seg_flicker ^= 1;
        time_800ms = 0;
    }
    if (Flow_flag)
    {
        if (++time_flow >= Flow_interval)
        {
            time_flow = 0;
            Led_show_index = (++Led_show_index) % 8;
        }
    }
    else
    {
        time_flow = 0;
        Led_show_index = 0;
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

void main()
{
    System_Init();
    Timer0_Init();

    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
    }
}