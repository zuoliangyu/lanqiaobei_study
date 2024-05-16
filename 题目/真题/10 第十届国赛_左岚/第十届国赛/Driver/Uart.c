#include "Uart.h"
void Uart1_Init(void)  // 4800bps@12.000MHz
{
  SCON = 0x50;   // 8λ����,�ɱ䲨����
  AUXR &= 0xBF;  // ��ʱ��ʱ��12Tģʽ
  AUXR &= 0xFE;  // ����1ѡ��ʱ��1Ϊ�����ʷ�����
  TMOD &= 0x0F;  // ���ö�ʱ��ģʽ
  TL1 = 0xCC;    // ���ö�ʱ��ʼֵ
  TH1 = 0xFF;    // ���ö�ʱ��ʼֵ
  ET1 = 0;       // ��ֹ��ʱ���ж�
  TR1 = 1;       // ��ʱ��1��ʼ��ʱ
  ES = 1;        // ʹ�ܴ���1�ж�
  EA = 1;
}

extern char putchar(char ch) {
  SBUF = ch;        // ��chд��SBUF����������
  while (TI == 0);  // �ȴ��������
  TI = 0;           // ���������ɱ�־
  return ch;
}