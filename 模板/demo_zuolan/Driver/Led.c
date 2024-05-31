#include "led.h"

void Led_Disp(unsigned char addr,unsigned char enable)
{
	static unsigned char temp = 0x00;
	static unsigned char temp_old = 0xff;
	if(enable) temp |= 0x01 <<addr;
	else temp &= ~(0x01 <<addr);
	if(temp != temp_old)
	{
		P0 = ~temp;
		P2 = P2 & 0x1f | 0x80;
		P2 &= 0x1f;
		temp_old = temp;
	}
}

unsigned char temp_0 = 0x00;
unsigned char temp_old_0 = 0xff;
void Beep(bit enable)
{
	if(enable) temp_0 |= 0x40;
	else temp_0 &= ~(0x40);
	if(temp_0 != temp_old_0)
	{
		P0 = temp_0;
		P2 = P2 & 0x1f | 0xa0;
		P2 &= 0x1f;
		temp_old_0 = temp_0;
	}
}
void Relay(bit enable)
{
	if(enable) temp_0 |= 0x10;
	else temp_0 &= ~(0x10);
	if(temp_0 != temp_old_0)
	{
		P0 = temp_0;
		P2 = P2 & 0x1f | 0xa0;
		P2 &= 0x1f;
		temp_old_0 = temp_0;
	}
}
void MOTOR(bit enable)
{
	if(enable) temp_0 |= 0x20;
	else temp_0 &= ~(0x20);
	if(temp_0 != temp_old_0)
	{
		P0 = temp_0;
		P2 = P2 & 0x1f | 0xa0;
		P2 &= 0x1f;
		temp_old_0 = temp_0;
	}
}