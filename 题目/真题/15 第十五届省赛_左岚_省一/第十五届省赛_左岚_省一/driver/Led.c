#include "Led.h"
void Led_Disp(unsigned char addr,unsigned char enable)
{
	static unsigned char temp=0x00;
	static unsigned char temp_old=0xff;
	if(enable)
		temp |= 1<<addr;
	else 
		temp &= ~(1<<addr);
	if(temp!=temp_old)
	{
		P0 = ~temp;
		P2 = P2 & 0x1f| 0x80;
		P2 &= 0x1f;
		temp_old = temp;
	}
}