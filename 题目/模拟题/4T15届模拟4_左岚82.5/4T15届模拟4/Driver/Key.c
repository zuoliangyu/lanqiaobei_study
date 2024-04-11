#include "Key.h"
unsigned char Key_Read()
{
    unsigned char temp = 0;
    ET0 = 0;
    P44 = 0;
    P42 = 1;
    P35 = 1;
    if (P33 == 0)
        temp = 4;
    if (P32 == 0)
        temp = 5;
    if (P31 == 0)
        temp = 6;
    if (P30 == 0)
        temp = 7;
    P44 = 1;
    P42 = 0;
    P35 = 1;
    if (P33 == 0)
        temp = 8;
    if (P32 == 0)
        temp = 9;
    if (P31 == 0)
        temp = 10;
    if (P30 == 0)
        temp = 11;
    P44 = 1;
    P42 = 1;
    P35 = 0;
    if (P33 == 0)
        temp = 12;
    if (P32 == 0)
        temp = 13;
    if (P31 == 0)
        temp = 14;
    if (P30 == 0)
        temp = 15;
    return temp;
}