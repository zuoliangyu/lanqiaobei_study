#include "uart.h"
void Uart1_Init(void)	//9600bps@12.000MHz
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x01;		//����1ѡ��ʱ��2Ϊ�����ʷ�����
	AUXR |= 0x04;		//��ʱ��ʱ��1Tģʽ
	T2L = 0xC7;			//���ö�ʱ��ʼֵ
	T2H = 0xFE;			//���ö�ʱ��ʼֵ
	AUXR |= 0x10;		//��ʱ��2��ʼ��ʱ
	ES = 1;				//ʹ�ܴ���1�ж�
	EA = 1;
}
extern char putchar (char ch)
{
	SBUF = ch;
	while(TI==0);
	TI = 0;
	return ch;
}