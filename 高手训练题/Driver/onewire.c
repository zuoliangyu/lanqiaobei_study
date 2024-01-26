#include "onewire.h"
#include "reg52.h"

sbit DQ = P1 ^ 4;

// 单总线内部延时函数
void Delay_OneWire(unsigned int t)
{
    t *= 12;
    while (t--)
        ;
}

// 单总线写操作
void Write_DS18B20(unsigned char dat)
{
    unsigned char i;
    for (i = 0; i < 8; i++)
    {
        DQ = 0;
        DQ = dat & 0x01;
        Delay_OneWire(5);
        DQ = 1;
        dat >>= 1; // 一个一个写入
    }
    Delay_OneWire(5);
}

// 单总线读操作
unsigned char Read_DS18B20(void)
{
    unsigned char i;
    unsigned char dat;

    for (i = 0; i < 8; i++)
    {
        DQ = 0;
        dat >>= 1;
        DQ = 1;
        if (DQ)
        {
            dat |= 0x80; // 一个一个读取
        }
        Delay_OneWire(5);
    }
    return dat;
}

// DS18B20初始化
bit init_ds18b20(void)
{
    bit initflag = 0;

    DQ = 1;
    Delay_OneWire(12);
    DQ = 0;
    Delay_OneWire(80);
    DQ = 1;
    Delay_OneWire(10);
    initflag = DQ;
    Delay_OneWire(5);

    return initflag;
}

void start_Convert()
{
    init_ds18b20();
    Write_DS18B20(0xcc); // 跳过ROM
    Write_DS18B20(0x44); // 启动温度转换
}
float read_temperture()
{
    unsigned char low, high;
    init_ds18b20();
    Write_DS18B20(0xcc);   // 跳过ROM
    Write_DS18B20(0xbe);   // 读取温度值
    low = Read_DS18B20();  // 低位
    high = Read_DS18B20(); // 高位
    return ((high << 8) | low) / 16.0;
}
float rd_temperature()
{
    unsigned char low, high;
    init_ds18b20();        // 初始化
    Write_DS18B20(0xcc);   // 跳过ROM
    Write_DS18B20(0x44);   // 进行温度转换
    init_ds18b20();        // 初始化
    Write_DS18B20(0xcc);   // 跳过ROM
    Write_DS18B20(0xbe);   // 读取温度
    low = Read_DS18B20();  // 低位
    high = Read_DS18B20(); // 高位
    return ((high << 8) | low) / 16.0;
}