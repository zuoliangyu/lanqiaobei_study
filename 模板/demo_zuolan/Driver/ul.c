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
void Ut_Wave_Init()
{
	unsigned char i;
	for (i = 0; i < 8; i++)
	{
		Tx =1;
		Delay12us();
		Rx =1;
		Delay12us();
	}
}
unsigned char  Ut_Wave_Data()
{
	unsigned int time;
	CH = CL = 0;
	CMOD = 0x00;
	
	EA = 0;
	Ut_Wave_Init();
	EA = 1;
	
	CR = 1;
	while(Rx && !CF);
	CR = 0;
	if(!CF)
	{//us -> s 10^(-6)
		//m -> cm 10^2
		// 10^(-4)
		//L = V*T/2=340*time/2=170*10^(-4)*time=0.017*time
		time = CH << 8 | CL; //µ¥Î»Îªus
		return (0.017*time);
	}
	else
	{
		CF = 0;
		return 0;
	}
}