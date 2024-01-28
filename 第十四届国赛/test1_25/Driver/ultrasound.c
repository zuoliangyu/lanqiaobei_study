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

void Ut_Wave_Init() // 初始化，产生8个40khz的方波
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

float Ut_Wave_Data(unsigned int speed, char cali_value) // 超声波距离读取函数
{
    unsigned int time; // 时间储存变量
    TMOD = 0x00;       // 配置PCA工作模式
    TH1 = TL1 = 0;     // 复位计数值 等待超声波信号发出
    Ut_Wave_Init();    // 发送超声波信号
    TR1 = 1;           // 开始计时
    while ((Rx == 1) && (TF1 == 0))
        ;         // 等待接受返回信号或者定时器溢出
    TR1 = 0;      // 停止计时
    if (TF1 == 0) // 定时器没有溢出
    {
        time = TH1 << 8 | TL1;                         // 读取当前时间
        return (time / 1000 * speed / 2 + cali_value); // 返回距离值
    }
    else
    {
        TF1 = 0; // 清除溢出标志位
        return 0;
    }
}
