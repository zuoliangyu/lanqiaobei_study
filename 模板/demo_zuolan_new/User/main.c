#include "main.h"
/* LED��ʾ */
uchar ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/* �������ʾ */
uchar Seg_Slow_Down;                                // ����ܼ���
uchar Seg_Buf[8] = {5, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ��ֵ
uchar Seg_Pos;                                      // �����ָʾ
uchar Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};      // ĳλ�Ƿ���ʾС����

/* ���ڷ��� */
uchar Uart_Slow_Down;
uchar Uart_Buf[20];  // ���ڽ��յ�������
uchar Uart_Rx_Index; // ���ڽ��յ������ݵ�ָ��

/* ʱ�䷽�� */
uchar ucRtc[3] = {0x13, 0x11, 0x11}; // ��ʼ��ʱ��13:11:11

/* ���̷��� */
uchar Key_Slow_Down;

/* ʱ�䷽�� */
uint time_all_1s;
/* ���� */
uint int_data; // ��������

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
/* ���ݴ����� */
void Data_Proc()
{
    if (time_all_1s % 50 == 0)
    {
        // ʱ���ȡ
    }
    if (time_all_1s % 100 == 0)
    {
        // AD��ȡ
    }
    if (time_all_1s % 500 == 0)
    {
        // �¶ȶ�ȡ
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
        EEPROM_Write(&int_data, 0, 2);
}
/* ����ܴ����� */
void Seg_Proc()
{
    if (time_all_1s % 20)
        return;
}

/* LED������ */
void Led_Proc()
{
}

/* ���ڴ����� */
void Uart_Proc()
{
    if (time_all_1s % 200)
        return;
    if (Uart_Buf[0] == 'O' && Uart_Buf[1] == 'K')
    {
        // ִ����غ���

        memset(Uart_Buf, 0, 20);
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
    if (++Key_Slow_Down == 10)
        Key_Slow_Down = 0;
    if (++Seg_Slow_Down == 500)
        Seg_Slow_Down = 0;
    if (++Uart_Slow_Down == 200)
        Uart_Slow_Down = 0;
    if (++Seg_Pos == 8)
        Seg_Pos = 0;
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
uchar passwd = 123;
uchar input_passwd;
void main()
{

    Timer0_Init();
    Uart1_Init();
    Set_Rtc(ucRtc);
    rd_temperature();
    Delay750ms();
    EEPROM_Read(&input_passwd, 8, 1); // �ò���д��ĵط���У��
    if (input_passwd != passwd)       // У��ʧ�ܣ�֮ǰδд������1/256���ʳ�����
    {
        EEPROM_Write(&passwd, 8, 1);
    }
    else // У��ͨ������ȡ������Ҫ������
    {
        EEPROM_Read(&int_data, 0, 2);
    }

    while (1)
    {
        Key_Proc();
        Seg_Proc();
        Uart_Proc();
        Led_Proc();
    }
}