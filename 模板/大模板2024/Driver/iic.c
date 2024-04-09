/*	#   I2C����Ƭ��˵��
	1. 	���ļ������ṩ���������빩����ѡ����ɳ�����Ʋο���
	2. 	����ѡ�ֿ������б�д��ش�����Ըô���Ϊ������������ѡ��Ƭ�����͡������ٶȺ�����
		�жԵ�Ƭ��ʱ��Ƶ�ʵ�Ҫ�󣬽��д�����Ժ��޸ġ�
*/
#include "iic.h"
#include "intrins.h"
#define DELAY_TIME 10
sbit scl = P2 ^ 0;
sbit sda = P2 ^ 1;
//
static void I2C_Delay(unsigned char n)
{
	do
	{
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
	} while (n--);
}

//
void I2CStart(void)
{
	sda = 1;
	scl = 1;
	I2C_Delay(DELAY_TIME);
	sda = 0;
	I2C_Delay(DELAY_TIME);
	scl = 0;
}

//
void I2CStop(void)
{
	sda = 0;
	scl = 1;
	I2C_Delay(DELAY_TIME);
	sda = 1;
	I2C_Delay(DELAY_TIME);
}

//
void I2CSendByte(unsigned char byt)
{
	unsigned char i;

	for (i = 0; i < 8; i++)
	{
		scl = 0;
		I2C_Delay(DELAY_TIME);
		if (byt & 0x80)
		{
			sda = 1;
		}
		else
		{
			sda = 0;
		}
		I2C_Delay(DELAY_TIME);
		scl = 1;
		byt <<= 1;
		I2C_Delay(DELAY_TIME);
	}

	scl = 0;
}

//
unsigned char I2CReceiveByte(void)
{
	unsigned char da;
	unsigned char i;
	for (i = 0; i < 8; i++)
	{
		scl = 1;
		I2C_Delay(DELAY_TIME);
		da <<= 1;
		if (sda)
			da |= 0x01;
		scl = 0;
		I2C_Delay(DELAY_TIME);
	}
	return da;
}

//
unsigned char I2CWaitAck(void)
{
	unsigned char ackbit;

	scl = 1;
	I2C_Delay(DELAY_TIME);
	ackbit = sda;
	scl = 0;
	I2C_Delay(DELAY_TIME);

	return ackbit;
}

//
void I2CSendAck(unsigned char ackbit)
{
	scl = 0;
	sda = ackbit;
	I2C_Delay(DELAY_TIME);
	scl = 1;
	I2C_Delay(DELAY_TIME);
	scl = 0;
	sda = 1;
	I2C_Delay(DELAY_TIME);
}

unsigned char Ad_Read(unsigned char addr)
{
	unsigned char temp;
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(addr);
	I2CWaitAck();
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	temp = I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	return temp;
}

void Da_Write(unsigned char dat)
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x41);
	I2CWaitAck();
	I2CSendByte(dat);
	I2CWaitAck();
	I2CStop();
}

// ��������дEEPROM����
// ��ڲ�������Ҫд����ַ�����д��ĵ�ַ(���Ϊ8�ı���)��д������
// ����ֵ����
// �������ܣ���EERPOM��ĳ����ַд���ַ������ض��������ַ���
void EEPROM_Write(unsigned char *EEPROM_String, unsigned char addr, unsigned char num)
{
	I2CStart();		   // ���Ϳ����ź�
	I2CSendByte(0xA0); // ѡ��EEPROMоƬ��ȷ��д��ģʽ
	I2CWaitAck();	   // �ȴ�EEPROM����

	I2CSendByte(addr); // д��Ҫ�洢�����ݵ�ַ
	I2CWaitAck();	   // �ȴ�EEPROM����

	while (num--)
	{
		I2CSendByte(*EEPROM_String++); // ��Ҫд�����Ϣд��
		I2CWaitAck();				   // �ȴ�EEPROM����
		I2C_Delay(200);
	}
	I2CStop(); // ֹͣ����
}

// ����������EEPROM����
// ��ڲ�����������������Ҫ�洢���ַ�������ȡ�ĵ�ַ(���Ϊ8�ı���)����ȡ������
// ����ֵ����
// �������ܣ���ȡEERPOM��ĳ����ַ�е����ݣ���������ַ��������С�
void EEPROM_Read(unsigned char *EEPROM_String, unsigned char addr, unsigned char num)
{
	I2CStart();		   // ���Ϳ����ź�
	I2CSendByte(0xA0); // ѡ��EEPROMоƬ��ȷ��д��ģʽ
	I2CWaitAck();	   // �ȴ�EEPROM����

	I2CSendByte(addr); // д��Ҫ��ȡ�����ݵ�ַ
	I2CWaitAck();	   // �ȴ�EEPROM����

	I2CStart();		   // ���Ϳ����ź�
	I2CSendByte(0xA1); // ѡ��EEPROMоƬ��ȷ������ģʽ
	I2CWaitAck();	   // �ȴ�EEPROM����

	while (num--)
	{
		*EEPROM_String++ = I2CReceiveByte(); // ��Ҫд�����Ϣд��
		if (num)
			I2CSendAck(0); // ����Ӧ��
		else
			I2CSendAck(1); // ��Ӧ��
	}

	I2CStop(); // ֹͣ����
}