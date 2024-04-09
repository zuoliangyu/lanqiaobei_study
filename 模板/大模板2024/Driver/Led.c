#include <Led.h>

void Led_Disp(unsigned char addr,enable)
{
	static unsigned char temp = 0x00;
	static unsigned char temp_old = 0xff;
	if(enable)
		temp |= 0x01 << addr;
	else
		temp &= ~(0x01 << addr);
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