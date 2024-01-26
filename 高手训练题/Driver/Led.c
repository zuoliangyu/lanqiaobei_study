#include <Led.h>
//亮码地址0-7，是否使能
void Led_Disp(unsigned char addr,enable)
{
    //保证这两个变量的值不会因为函数结束而改变
    static unsigned char temp = 0x00;
    static unsigned char temp_old = 0xff;
    //更改当前状态
    if(enable)
        temp |= 0x01 << addr;
    else
        temp &= ~(0x01 << addr);
    //当前状态与之前的状态不同则进行操作更新led
    if(temp != temp_old)
    {
        P0 = ~temp;
        P2 = P2 & 0x1f | 0x80;
        P2 &= 0x1f;
        temp_old = temp;
    }
}


void Beep(unsigned char flag)
{
    static unsigned char temp = 0x00;
    static unsigned char temp_old = 0xff;
    if(flag)
        temp |= 0x40;
    else
        temp &= ~0x40;
    if(temp != temp_old)
    {
        P0 = temp;
        P2 = P2 & 0x1f | 0xa0;
        P2 &= 0x1f;
        temp_old = temp;
    }
}

void Relay(unsigned char flag)
{
    static unsigned char temp = 0x00;
    static unsigned char temp_old = 0xff;
    if(flag)
        temp |= 0x10;
    else
        temp &= ~0x10;
    if(temp != temp_old)
    {
        P0 = temp;
        P2 = P2 & 0x1f | 0xa0;
        P2 &= 0x1f;
        temp_old = temp;
    }
}