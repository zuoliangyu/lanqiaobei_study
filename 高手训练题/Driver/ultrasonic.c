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
	unsigned int time;//ʱ�䴢�����
	CMOD = 0x00;//����PCA����ģʽ
	CH = CL = 0;//��λ����ֵ���ȴ��������źŷ���
	Ut_Wave_Init();//���ͳ������ź�
	CR = 1;//��ʼ��ʱ
	while((Rx ==1)&&(CF == 0));//�ȴ����ܷ����źŻ��߶�ʱ�����
	CR = 0;//ֹͣ��ʱ
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
