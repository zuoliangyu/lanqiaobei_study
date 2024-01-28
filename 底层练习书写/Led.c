#include "Led.h"

void Led_Porc(unsigned char addr, bit enable)
{
    static unsigned char temp = 0x00;
    static unsigned char temp_old = 0xff;
    if (enable)
        temp = temp | (0x01 << addr);
    else
        temp = temp & ~(0x01 << addr);
    if (temp != temp_old)
    {
        P2 = P2 & 0x1f | 0x80;
        P0 = temp;
        P2 = P2 & 0x1f;
        temp_old = temp;
    }
}

void Buzz_Porc(bit enable)
{
    static unsigned char temp = 0x00;
    static unsigned char temp_old = 0xff;
    if (enable)
        temp |= 0x40;
    else
        temp &= ~0x40;
    if (temp != temp_old)
    {
        P2 = P2 & 0x1f | 0x50;
        P0 = temp;
        P2 &= 0x1f;
        temp_old = temp;
    }
}
void Relay_Proc(bit enable)
{
    static unsigned char temp = 0x00;
    static unsigned char temp_old = 0xff;
    if (enable)
        temp |= 0x10;
    else
        temp &= ~0x10;
    if (temp != temp_old)
    {
        P2 = P2 & 0x1f | 0x50;
        P0 = temp;
        P2 &= 0x1f;
        temp_old = temp;
    }
}