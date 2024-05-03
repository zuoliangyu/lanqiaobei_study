#include "Ultrasonic.h"
#include "intrins.h"

sbit Tx = P1 ^ 0;
sbit Rx = P1 ^ 1;
void Delay12us(void) //@12.000MHz
{
    unsigned char data i;

    _nop_();
    _nop_();
    i = 33; // 38
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

unsigned int Ut_Wave_Data(char Cail, unsigned int speed)
{
    unsigned int time;
    CH = CL = 0; // 清空PCA寄存器计数值
    CCON = 0;    // 初始化PCA控制寄存器/PCA定时器停止/清除CF/

   // EA = 0;           // 关闭总中断
    Ut_Wave_Init();   // 发送超声波驱动信号
  //  EA = 1;           // 开启总中断
    CR = 1;           // PCA开始计数
    while (Rx && !CF) // TF1
        ;             // 等待接收
    CR = 0;           // PCA停止计数
    if (CF == 0)
    {
        time = CH << 8 | CL; // 单位为毫秒
        // V->m/s T->us
        return ((float)time / 20000 * speed + Cail); // 返回的单位是cm
    }
    else
    {
        CF = 0;
        return 0;
    }
}