#include "seg.h"
unsigned char seg_dula[]={0xc0,0xf9};
void Seg_Disp(unsigned char wela , unsigned char dula ,unsigned char point)
{
	P0 = 0xff;
	P2 = P2 & 0x1f | 0xe0;
	P2 &= 0x1f;
	
	P0 = 0x01 << wela;
	P2 = P2 & 0x1f | 0xc0;
	P2 &= 0x1f;
	
	P0 = seg_dula[dula];
	if(point)
		P0 &= 0x7f;
	P2 = P2 & 0x1f | 0xe0;
	P2 &= 0x1f;
}