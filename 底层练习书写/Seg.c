#include "Seg.h"

unsigned char seg_dula[] = {};
unsigned char seg_wela[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

void Seg_Proc(unsigned char dula, unsigned char wela, bit point)
{
    P0 = 0xff; // 消隐
    P2 = P2 & 0x1f | 0xe0;
    P2 &= 0x1f;

    P0 = seg_wela[wela];
    P2 = P2 & 0x1f | 0xc0;
    P2 &= 0x1f;

    P0 = seg_dula[dula];
    P2 = P2 & 0x1f | 0xe0;
    if (point)
        P2 &= 0x7f;
    P2 = P2 & 0x1f;
}