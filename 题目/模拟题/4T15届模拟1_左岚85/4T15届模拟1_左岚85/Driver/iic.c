/*	#   I2C代码片段说明
    1. 	本文件夹中提供的驱动代码供参赛选手完成程序设计参考。
    2. 	参赛选手可以自行编写相关代码或以该代码为基础，根据所选单片机类型、运行速度和试题
        中对单片机时钟频率的要求，进行代码调试和修改。
*/
#include "iic.h"
#include "intrins.h"
#define DELAY_TIME 5
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

/// @brief EEPROM 写函数
/// @param EEPROM_String 写入的数组指针（如果只写入一个数据，那么要用&data）
/// @param addr 8的倍数
/// @param num 写入的数组大小
void EEPROM_Write(unsigned char *EEPROM_String, unsigned char addr, unsigned char num)
{
    // 选择芯片为EEPROM，模式为写
    I2CStart();
    I2CSendByte(0xA0);
    I2CWaitAck();

    I2CSendByte(addr); // 写入的数据的地址
    I2CWaitAck();
    while (num--)
    {
        I2CSendByte(*EEPROM_String++);
        I2CWaitAck();
        I2C_Delay(200);
    }
    I2CStop();
}
/// @brief EEPROM 读函数
/// @param EEPROM_String 读取的数组指针（如果只读取一个数据，那么要用&data）
/// @param addr 8的倍数
/// @param num 读取的数组大小
void EEPROM_Read(unsigned char *EEPROM_String, unsigned char addr, unsigned char num)
{
    // 选择芯片为EEPROM，模式为写
    I2CStart();
    I2CSendByte(0xA0);
    I2CWaitAck();

    I2CSendByte(addr);
    I2CWaitAck();

    // 选择芯片为EEPROM，模式为读
    I2CStart();
    I2CSendByte(0xA1);
    I2CWaitAck();
    while (num--)
    {
        *EEPROM_String++ = I2CReceiveByte();
        if (num)
            I2CSendAck(0);
        else
            I2CSendAck(1); // 停止读取
    }
    I2CStop();
}