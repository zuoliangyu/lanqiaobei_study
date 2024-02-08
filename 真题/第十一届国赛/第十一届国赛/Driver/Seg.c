#include <Seg.h>
// 段选 0 1 2 3 4 5 6 7 8 9 灭 - C E P
unsigned char seg_dula[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0xff,
                            0xbf, 0xc6, 0x86, 0x8c};
// 位选 2^0,2^1,2^2,2^3,2^4,2^5，2^6,2^7，一共八个按键
unsigned char seg_wela[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

void Seg_Disp(unsigned char wela, dula, point)
{
    // 手动消抖
    P0 = 0xff;
    P2 = P2 & 0x1f | 0xe0;
    P0 = seg_wela[wela];
    P2 = P2 & 0x1f | 0xc0;
    P2 &= 0x1f;

    P0 = seg_dula[dula];
    if (point)
        P0 &= 0x7f;
    P2 = P2 & 0x1f | 0xe0;
    P2 &= 0x1f;
}