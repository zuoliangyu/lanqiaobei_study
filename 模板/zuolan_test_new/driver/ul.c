#include "ul.h"
#include "intrins.h"
sbit Tx = P1 ^ 0;
sbit Rx = P1 ^ 1;
void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 33;//38
	while (--i);
}
void Wave_Init()
{
	unsigned char i;
	for ( i =0;i<8;i++)
	{
		Tx = 1;
		Delay12us();
		Tx = 0;
		Delay12us();
	}
}
unsigned char Wave_Data()
{
	unsigned int time;
	CH = CL = 0;
	CMOD = 0x00;
	
	EA = 0;
	Wave_Init();
	EA = 1;
	CR = 1;
	while(Rx && !CF);
	if(CF == 0)
	{
		time = CH << 8 | CL;//us
		return (time*0.017);
	}
	else
	{
		CF = 0;
		return 0;
	}
}

	

