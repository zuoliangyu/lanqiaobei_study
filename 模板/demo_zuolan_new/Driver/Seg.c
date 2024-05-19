#include "Seg.h"
// 段选 0 1 2 3 4 5 6 7 8 9 灭 A
code unsigned char seg_dula[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92,
                                 0x82, 0xf8, 0x80, 0x90, 0xff, 0x88};
void Seg_Disp(unsigned char wela, unsigned char dula, unsigned char point) {
  // 手动消抖
  P0 = 0xff;
  P2 = P2 & 0x1f | 0xe0;

  // 选择显示位数
  P0 = 0x01 << wela;
  P2 = P2 & 0x1f | 0xc0;
  P2 &= 0x1f;

  // 选择显示的数据
  P0 = seg_dula[dula];
  if (point) P0 &= 0x7f;
  P2 = P2 & 0x1f | 0xe0;
  P2 &= 0x1f;
}