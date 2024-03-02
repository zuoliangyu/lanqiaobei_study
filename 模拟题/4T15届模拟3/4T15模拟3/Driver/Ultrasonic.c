#include "Ultrasonic.h"
#include "intrins.h"

sbit Tx = P1 ^ 0;
sbit Rx = P1 ^ 1;
void Delay12us(void) //@12.000MHz
{
    unsigned char data i;

    _nop_();
    _nop_();
    i = 33;
    while (--i)
        ;
}

void Ut_Wave_Init()
{
    unsigned char i;
    // 发送8个40kHz的方波
    // f=40kHz->T=1/f=0.000025s=25us
    // 12.5us 1 12.5us 0
    for (i = 0; i < 8; i++)
    {
        Tx = 1;
        Delay12us();
        Tx = 0;
        Delay12us();
    }
}

unsigned char Ut_Wave_Data()
{
    unsigned int time;
    TMOD &= 0x0f;
    TH1 = TL1 = 0;
    Ut_Wave_Init();
    TR1 = 1;
    while ((Rx == 1) && (TF1 == 0))
        ; // 数据读取未结束，并且没有溢出
    TR1 = 0;
    // 没有溢出，数据读取结束
    if (TF1 == 0)
    {
        time = TH1 << 8 | TL1; // 单位为毫秒
        // L=V*T/2
        return (time * 0.017); // 返回的单位是cm
    }
    // 数据溢出，认定此次测量无效
    else
    {
        TF1 = 0;
        return 0;
    }
}