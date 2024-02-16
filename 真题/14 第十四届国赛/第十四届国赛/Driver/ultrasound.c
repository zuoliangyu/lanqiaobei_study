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

void Ut_Wave_Init() // 生成8个40kHz的方波->25us->12us 12us
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

unsigned char Ut_Wave_Data(char calibration_value, unsigned char transmission_speed) // 超声波距离读取函数
{
    unsigned int time;
    TMOD &= 0X0F;
    TH1 = TL1 = 0;
    Ut_Wave_Init();
    TR1 = 1;
    while ((Rx == 1) && (TF1 == 0))
        ;
    TR1 = 0;
    if (TF1 == 0)
    {
        time = TH1 << 8 | TL1;
        return ((transmission_speed * time / 2000 / 100 + calibration_value));
    }
    else
    {
        TF1 = 0;
        return 0;
    }
}
