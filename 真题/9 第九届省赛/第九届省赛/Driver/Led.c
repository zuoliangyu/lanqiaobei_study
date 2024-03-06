#include "Led.h"
/// @brief Led显示
/// @param addr 需要书写的Led的位置0-7
/// @param enable 是否点亮 0灭 1亮
void Led_Disp(unsigned char addr, unsigned char enable)
{
    static unsigned char temp = 0x00;
    static unsigned char temp_old = 0xff;
    // 指定位置点亮
    if (enable)
        temp |= 0x01 << addr;
    else
        temp &= ~(0x01 << addr);
    if (temp != temp_old)
    {
        P0 = ~temp;
        P2 = P2 & 0x1f | 0x80;
        P2 &= 0x1f;
        temp_old = temp;
    }
}