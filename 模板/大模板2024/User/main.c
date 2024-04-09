/* ͷ�ļ������� */
// ��������Ӳ��������ͷ�ļ�����I2Cͨ�š�1-Wireͨ�š�����������������Ƭ���Ĵ���������ܺʹ���ͨ�ŵȡ�
#include "ds1302.h"
#include "iic.h"
#include "onewire.h"
#include "ultrasound.h"
#include "string.h"
#include "stdio.h"
#include <Init.h>		  // ��ʼ���ײ�����ר��ͷ�ļ�
#include <Key.h>		  // �����ײ�����ר��ͷ�ļ�
#include <Led.h>		  // LED�ײ�����ר��ͷ�ļ�
#include <STC15F2K60S2.H> // ��Ƭ���Ĵ���ר��ͷ�ļ�
#include <Seg.h>		  // ����ܵײ�����ר��ͷ�ļ�
#include <Uart.h>		  // ���ڵײ�����ר��ͷ�ļ�

/* ���������� */
// �������ȫ�ֱ�������������״̬���������ʾ���ݡ�LED��ʾ���ݡ����ڽ������ݵȡ�
unsigned char Key_Val, Key_Down, Key_Old, Key_Up;			 // ����״̬����
unsigned char Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ����
unsigned char Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		 // �����С��������
unsigned char Seg_Pos;										 // �����ɨ��λ��
unsigned char ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			 // LED��ʾ����
unsigned char Uart_Recv[10];								 // ���ڽ������ݻ�����
unsigned char Uart_Recv_Index;								 // ���ڽ�����������
unsigned char ucRtc[3] = {11, 12, 13};						 // ʵʱʱ������
unsigned int Slow_Down;										 // ���ټ�����
bit Seg_Flag, Key_Flag;										 // ����ܺͰ����ı�־λ
bit Uart_Flag;												 // ���ڱ�־λ
unsigned int Time_1s;										 // 1���Ӽ�����
unsigned int Freq;											 // Ƶ�ʼ������
unsigned int Sys_Tick;										 // ϵͳʱ�Ӽ���

/* ���̴����� */
// ���������룬��ⰴ�����½��غ������أ������°���״̬��
void Key_Proc()
{
	if (Key_Flag)return;
	Key_Flag = 1;							  // ���ñ�־λ����ֹ�ظ�����
	Key_Val = Key_Read();					  // ��ȡ����ֵ
	Key_Down = Key_Val & (Key_Old ^ Key_Val); // ����½���
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // ���������
	Key_Old = Key_Val;						  // ���°���״̬
}

/* ��Ϣ������ */
// �����������ʾ������
void Seg_Proc()
{
	if (Seg_Flag)return;
	Seg_Flag = 1; // ���ñ�־λ
}

/* ������ʾ���� */
// LED��ʾ������������û�о���ʵ�֡�
void Led_Proc() {}

/* ���ڴ����� */
// �����ڽ��յ������ݣ������յ�����ʱ���½��������ͻ�������
void Uart_Proc()
{
	if (Uart_Recv_Index == 0)return;
	if (Sys_Tick >= 10)
	{
		Sys_Tick = Uart_Flag = 0;
		// �߼�����

		
		
		
		memset(Uart_Recv, 0, Uart_Recv_Index);
		Uart_Recv_Index = 0; // ���ý�������
	}
}

/* ��ʱ��0��ʼ������ */
// ��ʼ����ʱ��0�����ڲ���1ms��ʱ���жϡ�
void Timer0_Init(void)
{
	AUXR &= 0x7F; // ���ö�ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0; // ���ö�ʱ��ģʽΪ16λ��ʱ��
	TMOD |= 0x05;
	TL0 = 0; // ���ö�ʱ����ʼֵ
	TH0 = 0; // ���ö�ʱ����ʼֵ
	TF0 = 0; // ���TF0��־λ
	TR0 = 1; // ������ʱ��
}

/* ��ʱ��1��ʼ������ */
// ��ʼ����ʱ��1�����ڲ���1ms��ʱ���жϣ��������жϡ�
void Timer1_Init(void)
{
	AUXR &= 0xBF; // ���ö�ʱ��ʱ��12Tģʽ
	TMOD &= 0x0F; // ���ö�ʱ��ģʽΪ16λ��ʱ��
	TL1 = 0x18;	  // ���ö�ʱ����ʼֵ
	TH1 = 0xFC;	  // ���ö�ʱ����ʼֵ
	TF1 = 0;	  // ���TF1��־λ
	TR1 = 1;	  // ������ʱ��
	ET1 = 1;	  // ʹ�ܶ�ʱ��1�ж�
	EA = 1;		  // ����ȫ���ж�
}

/* ��ʱ��1�жϷ����� */
// ��ʱ��1���жϷ����������ڸ���ϵͳʱ�ӡ����������������ʾ��
void Timer1_Isr(void) interrupt 3
{
	if (++Slow_Down == 400)
	{
		Seg_Flag = Slow_Down = 0; // �����������ʾ��־λ
	}
	if (Slow_Down % 10 == 0)
	{
		Key_Flag = 0; // ���°��������־λ
	}
	if (Uart_Flag)
		Sys_Tick++;
	if (++Time_1s == 1000)
	{
		Time_1s = 0;		   // ����1���Ӽ�����
		Freq = TH0 << 8 | TL0; // ����Ƶ��
		TH0 = 0;			   // ���ö�ʱ��0��ֵ
		TL0 = 0;
	}
	Seg_Disp(Slow_Down % 8, Seg_Buf[Slow_Down % 8], Seg_Point[Slow_Down % 8]); // �����������ʾ
	Led_Disp(Slow_Down % 8, ucLed[Slow_Down % 8]);							   // ����LED��ʾ
}

/* ����1�жϷ����� */
// ����1���жϷ����������ڴ����ڽ��յ������ݡ�
void Uart1Server() interrupt 4
{
	if (RI == 1) // ��⵽���ڽ����ж�
	{
		Uart_Flag = 1;					   // ���ô��ڱ�־λ
		Sys_Tick = 0;					   // ����ϵͳʱ��
		Uart_Recv[Uart_Recv_Index] = SBUF; // ������յ�������
		Uart_Recv_Index++;				   // ���½�������
		RI = 0;							   // ����жϱ�־λ
	}
	if (Uart_Recv_Index > 10)
		Uart_Recv_Index = 0;
}

/* ������ */
// ϵͳ��ʼ�������ö�ʱ���ʹ��ڣ�Ȼ�������ѭ����
void main()
{
	System_Init();	// ϵͳ��ʼ��
	Timer1_Init();	// ��ʼ����ʱ��1
	Set_Rtc(ucRtc); // ����ʵʱʱ��
	UartInit();		// ��ʼ������
	while (1)
	{
		Key_Proc();	 // ������
		Seg_Proc();	 // �����������ʾ
		Led_Proc();	 // ����LED��ʾ
		Uart_Proc(); // ����������
	}
}