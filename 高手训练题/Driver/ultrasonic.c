#include <ultrasonic.h>
#include <intrins.h>

sbit Tx = P1^0;
sbit Rx = P1^1;

void Delay12us(void)	//@12.000MHz
{
	unsigned char data i;

	_nop_();
	_nop_();
	i = 38;
	while (--i);
}

void Ut_Wave_Init()
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		Tx = 1;
		Delay12us();
		Tx = 0;
		Delay12us();
	}
}

unsigned char Ut_Wave_Data()
{
	unsigned int time;//时间储存变量
	CMOD = 0x00;//配置PCA工作模式
	CH = CL = 0;//复位计数值，等待超声波信号发出
	Ut_Wave_Init();//发送超声波信号
	CR = 1;//开始计时
	while((Rx ==1)&&(CF == 0));//等待接受返回信号或者定时器溢出
	CR = 0;//停止计时
	if(CF == 0)
	{
		time = CH << 8 |CL;
		
		return (time * 0.017);
		
	}
	else
	{
		CF = 0;
		return 0;
	}
}
