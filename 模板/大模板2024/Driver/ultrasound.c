#include <ultrasound.h>
#include "intrins.h"

sbit Tx = P1 ^ 0;
sbit Rx = P1 ^ 1;

void Delay12us() //@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 33;
	while (--i)
		;
}

void Ut_Wave_Init() // ��������ʼ������ ����8��40Mhz�ķ����ź�
{
	unsigned char i;
	for (i = 0; i < 8; i++)
	{
		Tx = 1;
		Delay12us();
		Tx = 0;
		Delay12us();
	}
}

unsigned char Ut_Wave_Data() // �����������ȡ����
{
	unsigned int time; // ʱ�䴢�����
	CMOD = 0x00;
	CH = CL = 0;	// ��λ����ֵ �ȴ��������źŷ���
	Ut_Wave_Init(); // ���ͳ������ź�
	CR = 1;			// ��ʼ��ʱ
	while ((Rx == 1) && (CF == 0))
		;		 // �ȴ����ܷ����źŻ��߶�ʱ�����
	CR = 0;		 // ֹͣ��ʱ
	if (CF == 0) // ��ʱ��û�����
	{
		time = CH << 8 | CL;   // ��ȡ��ǰʱ��
		return (time * 0.017); // ���ؾ���ֵ
	}
	else
	{
		CF = 0; // ��������־λ
		return 0;
	}
}