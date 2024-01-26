#include "iic.h"

#define DELAY_TIME 5

#include "reg52.h"
#include "intrins.h"

sbit SDA = P2^1;
sbit SCL = P2^0;

//I2C总线内部延时函数
void IIC_Delay(unsigned char i)
{
    do{_nop_();}
    while(i--);        
}

//I2C总线启动信号
void IIC_Start(void)
{
    SDA = 1;
    SCL = 1;
    IIC_Delay(DELAY_TIME);
    SDA = 0;
    IIC_Delay(DELAY_TIME);
    SCL = 0;	
}

//I2C总线停止信号
void IIC_Stop(void)
{
    SDA = 0;
    SCL = 1;
    IIC_Delay(DELAY_TIME);
    SDA = 1;
    IIC_Delay(DELAY_TIME);
}

//发送应答或非应答信号
void IIC_SendAck(bit ackbit)
{
    SCL = 0;
    SDA = ackbit;  					
    IIC_Delay(DELAY_TIME);
    SCL = 1;
    IIC_Delay(DELAY_TIME);
    SCL = 0; 
    SDA = 1;
    IIC_Delay(DELAY_TIME);
}

//等待应答
bit IIC_WaitAck(void)
{
    bit ackbit;
	
    SCL  = 1;
    IIC_Delay(DELAY_TIME);
    ackbit = SDA;
    SCL = 0;
    IIC_Delay(DELAY_TIME);
    return ackbit;
}

//I2C总线发送一个字节数据
void IIC_SendByte(unsigned char byt)
{
    unsigned char i;

    for(i=0; i<8; i++)
    {
        SCL  = 0;
        IIC_Delay(DELAY_TIME);
        if(byt & 0x80) SDA  = 1;
        else SDA  = 0;
        IIC_Delay(DELAY_TIME);
        SCL = 1;
        byt <<= 1;
        IIC_Delay(DELAY_TIME);
    }
    SCL  = 0;  
}

//I2C总线接收一个字节数据
unsigned char IIC_RecByte(void)
{
    unsigned char i, da;
    for(i=0; i<8; i++)
    {   
    	SCL = 1;
	IIC_Delay(DELAY_TIME);
	da <<= 1;
	if(SDA) da |= 1;
	SCL = 0;
	IIC_Delay(DELAY_TIME);
    }
    return da;    
}


unsigned char Ad_Read(unsigned char addr)
{
	unsigned char temp;
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	temp=IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	return temp;
}

void E2PROM_Write(unsigned char* EEPROM_String, unsigned char addr, unsigned char num)
{
	IIC_Start();
	IIC_SendByte(0xA0);
	IIC_WaitAck();
	
	IIC_SendByte(addr);
	IIC_WaitAck();

	while(num--)
	{
		IIC_SendByte(*EEPROM_String++);
		IIC_WaitAck();	
		IIC_Delay(200);	
	}
	IIC_Stop();
}


void E2PROM_Read(unsigned char* EEPROM_String, unsigned char addr, unsigned char num)
{
	IIC_Start();
	IIC_SendByte(0xA0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();	

	IIC_Start();
	IIC_SendByte(0xA1);
	IIC_WaitAck();
	
	while(num--)
	{
		*EEPROM_String++ = IIC_RecByte();
		if(num) IIC_SendAck(0);
			else IIC_SendAck(1);
	}
	
	IIC_Stop();
}

void Da_Write(unsigned char dat)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x41);
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}

