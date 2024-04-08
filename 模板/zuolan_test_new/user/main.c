#include "main.h"
/* LED��ʾ */
uchar ucLed[8] = {0,0,0,0,0,0,0,0};
/* �������ʾ */
uchar Seg_Buf[8]={10,10,10,10,10,10,10,10};
uchar Seg_Point[8]={0,0,0,0,0,0,0,0};
uchar Seg_Pos;

/* ������� */
uchar Uart_Buf[10];
uchar Uart_Rx_Index;
/* ʱ�� */
uint time_all_1s;

/* ���ݴ��� */
void Data_Porc()
{
	if(time_all_1s%100==0)
	{
		//ʱ���ȡ
	}
		if(time_all_1s%200==0)
	{
		//AD��ȡ
	}
}

/* ���������� */
void Key_Porc()
{
	static unsigned char Key_Val,Key_Old,Key_Down,Key_Up;
	if(time_all_1s % 10)
		return ;
	Key_Val = Key_Scan();
	Key_Down = Key_Val & (Key_Old ^ Key_Val);
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
	Key_Old = Key_Val;
}

/* �������ʾ������ */
void Seg_Porc()
{
	if(time_all_1s % 20)
		return ;
}
/* ������ʾ���� */
void Led_Porc()
{
}
/* ���ڴ�����*/
void Uart_Porc()
{
	if(time_all_1s % 200)
		return ;
	if(Uart_Buf[0]=="O"&&Uart_Buf[1]=="K")
	{
		//ִ��OK�յ���ĺ���
		printf("hello");
		memset(Uart_Buf,0,10);
		Uart_Rx_Index=0;
	}
}
void Timer0_Init(void)		//1����@12.000MHz
{
	AUXR &= 0x7F;			//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;			//���ö�ʱ��ģʽ
	TL0 = 0x18;				//���ö�ʱ��ʼֵ
	TH0 = 0xFC;				//���ö�ʱ��ʼֵ
	TF0 = 0;				//���TF0��־
	TR0 = 1;				//��ʱ��0��ʼ��ʱ
	ET0 = 1;				//ʹ�ܶ�ʱ��0�ж�
	EA = 1;
}

void Timer0_Isr(void) interrupt 1
{
	if(++time_all_1s == 1000)
		time_all_1s = 0;
	if(++Seg_Pos==8)
		Seg_Pos=0;
	Seg_Disp(Seg_Pos,Seg_Buf[Seg_Pos],Seg_Point[Seg_Pos]);
	Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}
void Uart1_Isr(void) interrupt 4
{
	if(RI)
	{
		Uart_Buf[Uart_Rx_Index] = SBUF;
		Uart_Rx_Index++;
		RI=0;
	}
}
void main()
{
	System_Init();
	Timer0_Init();
	Uart1_Init();
	while(1)
	{
		Key_Porc();
		Data_Porc();
		Seg_Porc();
		Led_Porc();
		Uart_Porc();
	}
}
