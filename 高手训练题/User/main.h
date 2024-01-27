/* 头文件声明区 */
#include <STC15F2K60S2.H> //单片机寄存器专用头文件
#include <Init.h>         //初始化底层驱动专用头文件
#include <Led.h>          //Led底层驱动专用头文件
#include <Key.h>          //按键底层驱动专用头文件
#include <Seg.h>          //数码管底层驱动专用头文件
#include "ds1302.h"
#include "onewire.h"
#include <ultrasonic.h>
#include "iic.h"
#include <Uart.h>
#include <stdio.h>
#include <string.h>
#include <intrins.h>

#define uchar unsigned char
#define uint unsigned int
