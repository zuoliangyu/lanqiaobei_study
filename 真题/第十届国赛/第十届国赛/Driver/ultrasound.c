#include <ultrasound.h>
#include "intrins.h"

sbit Tx = P1 ^ 0;
sbit Rx = P1 ^ 1;

void Delay12us() //@12.000MHz
{
    unsigned char i;

    _nop_();
    _nop_();
    i = 38;
    while (--i)
        ;
}

void Ut_Wave_Init() // ��������ʼ������ ����8��40Mhz�ķ����ź�
{
    unsigned char i;
    for (i = 0; i < 8; i++)
    {
        Tx = 1;
        Delay12us();
        Tx = 0;
        Delay12us();
    }
}

unsigned char Ut_Wave_Data() // 超声波距离读取函数
{
    unsigned int time;
    TMOD &= 0X0F;
    TH1 = TL1 = 0;
    Wave_Init();
    TR1 = 1;
    while ((RX == 1) && (TF1 == 0))
        ;
    TR1 = 0;
    if (TF1 == 0)
    {
        time = TH1 << 8 | TL1;
        return (time * 0.017);
    }
    else
    {
        TF1 = 0;
        return 0;
    }
}
