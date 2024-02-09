/* 头文件声明区 */
#include <STC15F2K60S2.H> //单片机寄存器专用头文件
#include <Init.h>         //初始化底层驱动专用头文件
#include <Led.h>          //Led底层驱动专用头文件
#include <Key.h>          //按键底层驱动专用头文件
#include <Seg.h>          //数码管底层驱动专用头文件
#include <stdio.h>        //标准库底层驱动专用头文件
#include <iic.h>          //数模转换底层驱动
#include "onewire.h"      //温度传感器驱动
#include "ds1302.h"       //时钟底层驱动

/* 变量声明区 */
idata unsigned char Key_Val, Key_Down, Key_Old, Key_Up;              // 按键专用变量
idata unsigned char Key_Slow_Down;                                   // 按键减速专用变量
idata unsigned char Seg_Buf[8]   = {10, 10, 10, 10, 10, 10, 10, 10}; // 数码管显示数据存放数组
idata unsigned char Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};         // 数码管小数点数据存放数组
idata unsigned char Seg_Pos;                                         // 数码管扫描专用变量
idata unsigned char Seg_Slow_Down;                                   // 数码管减速专用变量
idata unsigned char ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};             // Led显示数据存放数组

idata unsigned short time_3s      = 0;                  // 3s计时
idata unsigned short time_2s      = 0;                  // 2s计时
idata unsigned short time_1s      = 0;                  // 1s计时
idata unsigned char ucRtc[3]      = {0x23, 0x11, 0x59}; // 设置时钟的上电初始状态
idata unsigned char Seg_show_mode = 0;                  // 数码管显示的模式
// 0显示时分秒，1显示C温度，2显示H湿度，3显示F触发次数+采集的时分，4显示P温度参数
idata unsigned char show_flag      = 0; // 时间->回显->参数----->温湿度采集显示
idata unsigned char re_show_flag   = 0; // 温度->湿度->时间
idata unsigned char light_flag     = 0; // 1是从光照变为黑暗
idata unsigned char temp_show_flag = 0; // 记录到达温湿度采集的上一刻的显示

/*温度界面*/
idata float max_temperature           = 0; // 最大温度
 idata float sum_temperature           = 0; // 总温度
 idata unsigned char index_temperature = 0; // 温度计序号
   idata float temp_temperature;
  
  idata  float aver_temperature;
   
/*湿度界面*/
idata unsigned short count_f       = 0; // 频率计数
idata unsigned short dat_f         = 0; // 1s的频率
idata float max_humidity           = 0; // 最大湿度
idata float sum_humidity           = 0; // 总湿度
idata unsigned char index_humidity = 0; // 湿度计序号
 idata float temp_humidity;
 idata float aver_humidity;
/*参数界面*/
idata unsigned char temperature_ref = 30; // 参考温度
                                          /*触发次数+时间显示*/
idata unsigned char triggers_number = 0;  // 触发次数

/* 键盘处理函数 */
void Key_Proc() // 10ms扫描一次
{
    if (Key_Slow_Down) return;
    Key_Slow_Down = 1;                              // 键盘减速程序
    Key_Val       = Key_Read();                     // 实时读取键码值
    Key_Down      = Key_Val & (Key_Old ^ Key_Val);  // 捕捉按键下降沿
    Key_Up        = ~Key_Val & (Key_Old ^ Key_Val); // 捕捉按键上降沿
    Key_Old       = Key_Val;                        // 辅助扫描变量
    // 按下了9，且处于时间回显
    if (Key_Val == 9 && re_show_flag == 2) {
        time_2s++;
        triggers_number = 0;
    } else {
        time_2s = 0;
    }
    // 长按2s
    if (time_2s >= 100) {
        triggers_number = 0;
    }
    // 如果经历了一次温度采集
    if (temp_show_flag != 0) {
        show_flag      = temp_show_flag;
        temp_show_flag = 0;
    }
    // 按下界面按钮
    if (Key_Down == 4) {
        show_flag++;
        if (show_flag >= 3) {
            show_flag = 0;
        }
        if (show_flag == 0) {
            re_show_flag = 0;
        }
    }
    if (Key_Down == 5 && show_flag == 1) {
        re_show_flag++;
        if (re_show_flag >= 3) {
            re_show_flag = 0;
        }
    }
    if (Key_Down == 8 && show_flag == 2) {
        temperature_ref++;
        if (temperature_ref >= 100) {
            temperature_ref = 0;
        }
    }
    if (Key_Down == 9 && show_flag == 2) {
        temperature_ref--;
        if (temperature_ref < 0) {
            temperature_ref = 99;
        }
    }

    if (light_flag) {
        temp_show_flag = show_flag;
        time_3s        = 0;   // 从现在开始记录3s
        show_flag      = 100; // 这个时候触发采集，我们进入温湿度界面
        light_flag     = 0;
    }
}
/* 信息处理函数 */
void Seg_Proc()
{
    unsigned char int_temp_temperature; // 整数部分温度
    unsigned char int_temp_humidity;    // 整数部分湿度

    if (Seg_Slow_Down) return;
    Seg_Slow_Down = 1; // 数码管减速程序

    /*数码管读取时间*/
    Read_Rtc(ucRtc); // 读取DS1302里面的时间

    /*数码管读取温度并处理*/
    temp_temperature = rd_temperature();
    index_temperature++;
    // 我们这里为了防止芯片读取最初的那个85°
    if (temp_temperature == 85) {
        temp_temperature = 0;
        index_temperature--;
    }
    sum_temperature += temp_temperature;
    aver_temperature = sum_temperature / index_temperature; // 计算平均温度
    if (max_temperature < temp_temperature) {
        max_temperature = temp_temperature;
    }
    int_temp_temperature = temp_temperature * 10;

    /*数码管读取湿度并处理*/
    if (dat_f < 200 || dat_f > 2000) {
        temp_humidity = 0; // 无效数据
    } else {
        temp_humidity = dat_f * 2 / 45;
        if (max_humidity < temp_humidity) {
            max_humidity = temp_humidity;
        }
        index_humidity++;
        sum_humidity += temp_humidity;
        aver_humidity        = sum_humidity / index_humidity;
        int_temp_temperature = temp_humidity * 10;
    }
    /*数码管读取光暗数值并处理*/
    // triggers_number++;
    if (triggers_number >= 100) {
        triggers_number = 0;
    }
    /*数码管显示*/
    switch (Seg_show_mode) // 数码管显示模式
    {
        case 0:
            Seg_Buf[0] = ucRtc[0] / 16;
            Seg_Buf[1] = ucRtc[0] % 16;
            Seg_Buf[3] = ucRtc[1] / 16;
            Seg_Buf[4] = ucRtc[1] % 16;
            Seg_Buf[6] = ucRtc[2] / 16;
            Seg_Buf[7] = ucRtc[2] % 16;
            Seg_Buf[2] = Seg_Buf[5] = 12; //-
            Seg_Point[6]            = 0;
            break;
        case 1:
            Seg_Buf[0]   = 13; // C
            Seg_Buf[1]   = 10;
            Seg_Buf[2]   = (int)max_temperature / 10;
            Seg_Buf[3]   = (int)max_temperature % 10;
            Seg_Buf[4]   = 12; //-
            Seg_Buf[5]   = int_temp_temperature / 100;
            Seg_Buf[6]   = int_temp_temperature % 100 / 10;
            Seg_Point[6] = 1;
            Seg_Buf[7]   = int_temp_temperature % 10;
            break;
        case 2:
            Seg_Buf[0]   = 14; // H
            Seg_Buf[1]   = 10;
            Seg_Buf[2]   = (int)max_humidity / 10;
            Seg_Buf[3]   = (int)max_humidity % 10;
            Seg_Buf[4]   = 12; //-
            Seg_Buf[5]   = int_temp_humidity / 100;
            Seg_Buf[6]   = int_temp_humidity % 100 / 10;
            Seg_Point[6] = 1;
            Seg_Buf[7]   = int_temp_humidity % 10;
            break;
        case 3:
            Seg_Buf[0] = 15; // F
            Seg_Buf[1] = triggers_number / 10;
            Seg_Buf[2] = triggers_number % 10;
            if (triggers_number == 0) {
                Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 10; // 灭
            } else {
                Seg_Buf[3] = ucRtc[0] / 16;
                Seg_Buf[4] = ucRtc[0] % 16;
                Seg_Buf[6] = ucRtc[1] / 16;
                Seg_Buf[7] = ucRtc[1] % 16;
            }
            Seg_Buf[5]   = 12; //-
            Seg_Point[6] = 0;
            break;
        case 4:
            Seg_Buf[0]   = 16; // P
            Seg_Point[6] = 0;
            Seg_Buf[6]   = temperature_ref / 10;
            Seg_Buf[7]   = temperature_ref % 10;
            Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10; // 灭
            break;
        case 5:
            Seg_Buf[0] = 17; // E
            Seg_Buf[3] = (int)temp_temperature / 10;
            Seg_Buf[4] = (int)temp_temperature % 10;
            Seg_Buf[5] = 12;              //-
            Seg_Buf[1] = Seg_Buf[2] = 10; // 灭
            break;
        default:
            break;
    }
}

/* 其他显示函数 */
void Led_Proc()
{
    if (Seg_show_mode == 0) {
        ucLed[0] = 1;
    } else {
        ucLed[0] = 0;
    }
    if (Seg_show_mode == 1 || Seg_show_mode == 2 || Seg_show_mode == 3) {
        ucLed[1] = 1;
    } else {
        ucLed[1] = 0;
    }
    if (Seg_show_mode == 5) {
        ucLed[2] = 1;
    } else {
        ucLed[2] = 0;
    }
}

/* 定时器0中断初始化函数 */
void Timer0Init(void) // 1毫秒@12.000MHz
{
    AUXR &= 0x7F; // 定时器时钟12T模式
    TMOD &= 0xF0; // 设置定时器模式
    TL0 = 0x18;   // 设置定时初始值
    TH0 = 0xFC;   // 设置定时初始值
    TF0 = 0;      // 清除TF0标志
    TR0 = 1;      // 定时器0开始计时
    ET0 = 1;      // 定时器中断0打开
    EA  = 1;      // 总中断打开
}
/*计数器1中断初始化函数*/
void Timer1Init(void)
{
    TH1 = 0XFF;
    TL1 = 0XFF;
    TR1 = 1;
    ET1 = 1;
    EA  = 1;
}
void Timer1Server() interrupt 3
{
    count_f++;
}
/* 定时器0中断服务函数 */
void Timer0Server() interrupt 1
{
    if (++Key_Slow_Down == 10) Key_Slow_Down = 0;  // 键盘减速专用
    if (++Seg_Slow_Down == 200) Seg_Slow_Down = 0; // 数码管减速专用
    if (++Seg_Pos == 8) Seg_Pos = 0;               // 数码管显示专用
    if (++time_3s >= 3000)                         // 当计时3s时
    {
        time_3s    = 3001; // 防止溢出
        light_flag = 0;
    }
    if (++time_1s >= 1000) {
        // 现在1s,我们来统计一下频率
        time_1s = 0;
        dat_f   = count_f; // 取出这1s的频率
        count_f = 0;       // 计数值归零
    }
    Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);
    Led_Disp(Seg_Pos, ucLed[Seg_Pos]);
}

void deal_mode()
{

    if (show_flag == 0) {
        Seg_show_mode = 0;
    } else if (show_flag == 1) {
        if (re_show_flag == 0) {
            Seg_show_mode = 1;
        } else if (re_show_flag == 1) {
            Seg_show_mode = 2;
        } else if (re_show_flag == 2) {
            Seg_show_mode = 3;
        }
    } else if (show_flag == 2) {
        Seg_show_mode = 4;
    } else if (show_flag == 100) {
        Seg_show_mode = 5;
    }
}
/* Main */
void main()
{
    System_Init();
    Timer0Init();
    Timer1Init();
    Set_Rtc(ucRtc); // 上电初始化时钟
    while (1) {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
        deal_mode();
    }
}