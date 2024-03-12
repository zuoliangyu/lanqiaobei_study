#include "STC15F2K60S2.H"

unsigned char Ad_Read(unsigned char addr);

// 函数名：写EEPROM函数
// 入口参数：需要写入的字符串，写入的地址(务必为8的倍数)，写入数量
// 返回值：无
// 函数功能：向EERPOM的某个地址写入字符串中特定数量的字符。
void EEPROM_Write(unsigned char *EEPROM_String, unsigned char addr, unsigned char num);

// 函数名：读EEPROM函数
// 入口参数：读到的数据需要存储的字符串，读取的地址(务必为8的倍数)，读取的数量
// 返回值：无
// 函数功能：读取EERPOM的某个地址中的数据，并存放在字符串数组中。
void EEPROM_Read(unsigned char *EEPROM_String, unsigned char addr, unsigned char num);