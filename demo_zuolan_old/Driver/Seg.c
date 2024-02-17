#include "Seg.h"
// ��ѡ
unsigned char seg_dula[] = {0xc0};
// λѡ
unsigned char seg_wela[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
void Seg_Disp(unsigned char wela, unsigned char dula, unsigned char point)
{
    // �ֶ�����
    P0 = 0xff;
    P2 = P2 & 0x1f | 0xe0;

    // ѡ����ʾλ��
    P0 = seg_wela[wela];
    P2 = P2 & 0x1f | 0xc0;
    P2 &= 0x1f;

    // ѡ����ʾ������
    P0 = seg_dula[dula];
    if (point)
        P0 &= 0x7f;
    P2 = P2 & 0x1f | 0xc0;
    P2 &= 0x1f;
}