#include <Key.h>

unsigned char Key_Read()
{
	unsigned char temp = 0;
	P44 = 0;
	P42 = 1;
	P35 = 1;
	P34 = 1;
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
	P34 = 1;
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
	P34 = 1;
	if (P33 == 0)
		temp = 12;
	if (P32 == 0)
		temp = 13;
	if (P31 == 0)
		temp = 14;
	if (P30 == 0)
		temp = 15;
	P44 = 1;
	P42 = 1;
	P35 = 1;
	P34 = 0;
	if (P33 == 0)
		temp = 16;
	if (P32 == 0)
		temp = 17;
	if (P31 == 0)
		temp = 18;
	if (P30 == 0)
		temp = 19;
	return temp;
}
/*

	进阶版矩阵键盘底层v2.0

	更新内容：
		1.修复行列相反的bug
		2.将检测的宏定义写入key.h

	注意：
		！使用时一定也要将key.h替换！

	调用举例：
		void Key_Proc()
		{
			if(Key_Slow_Down) return;
			Key_Slow_Down = 1;

			Key_Val = Key_Read();
			Key_Down = Key_Val & (Key_Old ^ Key_Val);
			Key_Up = ~Key_Val & (Key_Old ^ Key_Val);
			Key_Old = Key_Val;

			if(KeyDown_Num(4))
			{
				if(!KeyDown_Num(9)) //排除组合按键
				{
					//S4单按键功能区
				}
			}

			if(KeyDown_Num(9)) //按键S9产生下降沿
			{
				if(!KeyDown_Num(4)) //排除组合按键
				{
					//S9单按键功能区
				}
			}

			if((KeyDown_Num(4)) && (KeyDown_Num(9)))
			{
				//S4+S9组合按键功能区
			}
		}

*/
/*
#include <Key.h>

unsigned int Key_Read()
{
	unsigned int temp = 0; // 16bit对应16个按键
	int row, col;

	for (col = 0; col < 4; col++)
	{
		P44 = (col != 0);
		P42 = (col != 1);
		P35 = (col != 2);
		P34 = (col != 3);

		for (row = 0; row < 4; row++)
		{
			if (!(P3 & (0x08 >> row))) // 第0行-P33 第1行-P32 第2行-P31 第3行-P30 顺序相反
				temp |= (1 << (col * 4 + row));
		}
	}
	return temp;
}*/
