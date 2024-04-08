#include "key.h"
#define ROW1 P30
#define ROW2 P31
#define ROW3 P32
#define ROW4 P33
#define COL1 P44
#define COL2 P42
#define COL3 P35
#define COL4 P34
unsigned char Key_Scan()
{
	unsigned char temp;
	COL1=1;COL2=0;COL3=0;COL4=0;
	if(ROW1==0)
		temp=7;
	if(ROW2==0)
		temp=6;
	if(ROW3==0)
		temp=5;
	if(ROW4==0)
		temp=4;
	COL1=0;COL2=1;COL3=0;COL4=0;
	if(ROW1==0)
		temp=11;
	if(ROW2==0)
		temp=10;
	if(ROW3==0)
		temp=9;
	if(ROW4==0)
		temp=8;
	COL1=0;COL2=0;COL3=1;COL4=0;
	if(ROW1==0)
		temp=15;
	if(ROW2==0)
		temp=14;
	if(ROW3==0)
		temp=13;
	if(ROW4==0)
		temp=12;
	COL1=0;COL2=0;COL3=0;COL4=1;
	if(ROW1==0)
		temp=19;
	if(ROW2==0)
		temp=18;
	if(ROW3==0)
		temp=17;
	if(ROW4==0)
		temp=16;
	return temp;
}