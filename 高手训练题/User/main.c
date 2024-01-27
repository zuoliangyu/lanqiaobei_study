/* 头文件声明区 */
#include <main.h> //单片机寄存器专用头文件

/* 变量声明区 */

unsigned char Key_Slow_Down;										  // 按键减速专用变量
unsigned int Uart_Slow_Down;										  // 按键减速专用变量
idata unsigned char Seg_Buf[8] = {10, 10, 10, 10, 10, 10, 10, 10};	  // 数码管显示数据存放数组
idata unsigned char Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};		  // 数码管小数点数据存放数组
unsigned char Seg_Pos;												  // 数码管扫描专用变量
unsigned int Seg_Slow_Down;											  // 数码管减速专用变量
idata unsigned char ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};			  // Led显示数据存放数组
unsigned char Seg_Disp_Mode;										  // 模式界面 0-系统初始界面 1-主界面 时钟  信息
unsigned char Pass_Word[8] = {1, 2, 3, 4, 5, 6, 7, 8};				  // 初始密码
idata unsigned char Pass_Input[8] = {11, 11, 11, 11, 11, 11, 11, 11}; // 密码输入
unsigned char Pass_Input_Index;										  // 密码输入指针
unsigned int Timer_3000Ms;											  // 长按计时
bit Key_Flag;														  // 计时标志位
idata unsigned char Clock_Crl[3] = {23, 59, 55};					  // 时钟
unsigned int Timer_500Ms;											  // 时钟间隔符闪烁
bit Seg_Star_Flag;													  // 闪烁标志位
unsigned char ultrasonic;											  // 超声波测距
unsigned char ultrasonic_Ture;										  // 超声波测距
float AD_Output;													  // AD输出
float Temp;															  // 温度
bit Sun_Flag;														  // 是否有光标志位
unsigned int Timer_2000Ms;											  // 长按计时
bit Key_Flag1;														  // 计时标志位
idata unsigned char Uart_Recv[10];									  // 串口接收数组
unsigned char Uart_Recv_Index;										  // 串口接收数组指针
unsigned int Syt_Flag;												  // 系统计时
bit Time_Syt_Flag;													  // 系统计时开始标志位
bit Uart_Enable_Flag;												  // 串口使能标志位
idata unsigned char Clock_Set[6] = {2, 3, 5, 9, 5, 5};				  // 时钟设置
unsigned char Clock_Set_Index;
idata unsigned char Alarm[3] = {0, 0, 0};			   // 初始闹钟
idata unsigned char Alarm_Set[6] = {0, 0, 0, 0, 0, 0}; // 闹钟设置
unsigned int Timer_700Ms;							   // 时钟设置闹钟设置计时
bit Seg_Star_Flag2;									   // 闪烁标志位
bit Alarm_Enable_Flag;								   // 闹钟使能标志位
unsigned char Key_Input_Flag;						   // 输入十位个位标志位
idata unsigned char Clock[3];						   // 设置时钟保存
idata unsigned char E2PROM_Alarm[3];				   // 闹钟保存
float AD_Rb2_Output;								   // Rb2输出
unsigned char Term_Led;								   // 周期
unsigned char Led_Lever;							   // 等级
unsigned char Led_Pos;								   // LED扫描
float DA_Output;									   // DA输出
unsigned char Timer_Count;							   // 串口时间计数

// 键盘映射
uchar Key_to_num(uchar Key_Down)
{
	uchar key_num[10] = {4, 8, 12, 16, 9, 13, 17, 10, 14, 18};
	uchar i;
	for (i = 0; i < 10; i++)
		if (Key_Down == key_num[i])
			return i;
	// 当按下的按键是没有用的按键的时候
	if (i == 10 && Key_Down != key_num[9])
		return 100;
}
/* 键盘处理函数 */
void Key_Proc()
{
	static uchar Key_Val, Key_Down, Key_Old, Key_Up; // 按键专用变量
	unsigned char i;
	if (Key_Slow_Down)
		return;
	Key_Slow_Down = 1; // 键盘减速程序

	Key_Val = Key_Read();					  // 实时读取键码值
	Key_Down = Key_Val & (Key_Old ^ Key_Val); // 捕捉按键下降沿
	Key_Up = ~Key_Val & (Key_Old ^ Key_Val);  // 捕捉按键上降沿
	Key_Old = Key_Val;						  // 辅助扫描变量

	/*初始界面按键 密码输入*/
	if (Seg_Disp_Mode == 0 && Pass_Input_Index < 8)
	{
		uchar input_value;
		input_value = Key_to_num(Key_Down);
		// 输入有效
		if (input_value != 100)
		{
			Pass_Input[Pass_Input_Index] = input_value;
			Pass_Input_Index++;
		}
	}
	if (Seg_Disp_Mode == 0)
	{
		// 长按 跳过密码输入
		if (Key_Down == 5)
			Key_Flag = 1;
		if (Timer_3000Ms >= 3000) // 判定为长按
		{
			if (Key_Old == 5)
				Seg_Disp_Mode = 1; // 进入主界面
			if (Key_Up == 5)
			{
				Timer_3000Ms = Key_Flag = 0;
			}
		}
		else
		{
			if (Key_Up == 5)
				Timer_3000Ms = Key_Flag = 0;
		}
		switch (Key_Down)
		{
		case 7:												 // 确认
			if (Pass_Input_Index == 8 && Seg_Disp_Mode == 0) // 密码输完
			{
				i = 0;
				while (Pass_Input[i] == Pass_Word[i]) // 循环判断
				{
					i++;
					if (i == 8)
						break; // 防止循环越界判断
				}
				if (i == 8) // 密码输入正确
				{
					Seg_Disp_Mode = 1; // 进入主界面
					Pass_Input_Index = 0;
					Beep(0);
				}
				else
				{
					Pass_Input_Index = 0;
					for (i = 0; i < 8; i++)
						Pass_Input[i] = 11; // 输入内容清空
					Beep(1);
				}
			}
			break;
		case 6: // 删除
			if (Pass_Input_Index != 0)
			{
				Pass_Input_Index--;
				Seg_Buf[7 - Pass_Input_Index] = 11;
			}
			break;
		}
	}
	/*系统主界面按键*/
	if ((Seg_Disp_Mode == 1 || Seg_Disp_Mode == 2) && (Uart_Enable_Flag == 0)) // 时钟 超声波温度
	{
		// 长按信息显示按键可进入信息显示界面 松手后返 回时钟显示界面
		if (Key_Down == 7)
			Key_Flag1 = 1;
		if (Timer_2000Ms > 2000) // 判定为长按
		{
			if (Key_Old == 7)
				Seg_Disp_Mode = 2;
			if (Key_Up == 7) // 捕捉到上升沿
			{
				Seg_Disp_Mode = 1;
				Timer_2000Ms = Key_Flag1 = 0; // 复位，便于下次
			}
		}
		else
		{
			if (Key_Up == 7)				  // 捕捉到上升沿
				Timer_2000Ms = Key_Flag1 = 0; // 复位，便于下次
		}

		switch (Key_Down)
		{
		case 11: // 串口开
			Uart_Enable_Flag = 1;
			break;
		case 6: // 时钟设置界面
			for (i = 0; i < 3; i++)
			{
				Clock_Set[i * 2] = Clock_Crl[i] / 10;
				Clock_Set[i * 2 + 1] = Clock_Crl[i] % 10;
			}
			Seg_Disp_Mode = 3;
			break;
		case 5: // 闹钟设置
			Seg_Disp_Mode = 4;
			break;
		case 4: // 闹钟使能
			Alarm_Enable_Flag = 1;
			break;
		}
	}

	/*时钟设置闹钟设置按键*/
	if ((Seg_Disp_Mode == 3 || Seg_Disp_Mode == 4) && (Uart_Enable_Flag == 0))
	{
		switch (Key_Down)
		{
		case 11: // 小时设置
			Key_Input_Flag = 0;
			Clock_Set_Index = 0;
			break;
		case 15: // 分钟设置
			Key_Input_Flag = 2;
			Clock_Set_Index = 2;
			break;
		case 19: // 秒钟设置
			Key_Input_Flag = 4;
			Clock_Set_Index = 4;
			break;
		case 7:						// 确认保存 保存到 EEPROM 内
			if (Seg_Disp_Mode == 3) // 时钟
			{
				Clock[0] = (Clock_Set[0] * 10 + Clock_Set[1]);
				Clock[1] = (Clock_Set[2] * 10 + Clock_Set[3]);
				Clock[2] = (Clock_Set[4] * 10 + Clock_Set[5]);
				if (Clock[0] < 23 && Clock[1] < 59 && Clock[2] < 59) // 数据合理
				{
					Set_Rtc(Clock); // 保存
					E2PROM_Read(Clock, 8, 3);
					Seg_Disp_Mode = 1;
				}
			}
			if (Seg_Disp_Mode == 4) // 闹钟
			{
				E2PROM_Alarm[0] = (Alarm_Set[0] * 10 + Alarm_Set[1]);
				E2PROM_Alarm[1] = (Alarm_Set[2] * 10 + Alarm_Set[3]);
				E2PROM_Alarm[2] = (Alarm_Set[4] * 10 + Alarm_Set[5]);
				if (E2PROM_Alarm[0] < 23 && E2PROM_Alarm[1] < 59 && E2PROM_Alarm[2] < 59) // 数据合理
				{
					E2PROM_Read(E2PROM_Alarm, 0, 3);
					Seg_Disp_Mode = 1;
				}
			}
			break;
		case 6: // 取消保存
			if (Seg_Disp_Mode == 3)
			{
				Set_Rtc(Clock_Crl); // 保存原来值
			}
			if (Seg_Disp_Mode == 4)
			{
				E2PROM_Alarm[0] = E2PROM_Alarm[1] = E2PROM_Alarm[2] = 0;
			}
			break;
		case 5: // 串口
			Uart_Enable_Flag = 1;
			break;
		}
		if (Clock_Set_Index < (Key_Input_Flag + 2)) // 输入两个之后不能再输
		{
			switch (Key_Down) // 时间输入
			{
			case 4:
				if (Seg_Disp_Mode == 3)
					Clock_Set[Clock_Set_Index] = 0;
				else if (Seg_Disp_Mode == 4)
					Alarm_Set[Clock_Set_Index] = 0;
				Clock_Set_Index++;
				break;
			case 8:
				if (Seg_Disp_Mode == 3)
					Clock_Set[Clock_Set_Index] = 1;
				else if (Seg_Disp_Mode == 4)
					Alarm_Set[Clock_Set_Index] = 1;
				Clock_Set_Index++;
				break;
			case 12:
				if (Seg_Disp_Mode == 3)
					Clock_Set[Clock_Set_Index] = 2;
				else if (Seg_Disp_Mode == 4)
					Alarm_Set[Clock_Set_Index] = 2;
				Clock_Set_Index++;
				break;
			case 16:
				if (Seg_Disp_Mode == 3)
					Clock_Set[Clock_Set_Index] = 3;
				else if (Seg_Disp_Mode == 4)
					Alarm_Set[Clock_Set_Index] = 3;
				Clock_Set_Index++;
				break;
			case 9:
				if (Seg_Disp_Mode == 3)
					Clock_Set[Clock_Set_Index] = 4;
				else if (Seg_Disp_Mode == 4)
					Alarm_Set[Clock_Set_Index] = 4;
				Clock_Set_Index++;
				break;
			case 13:
				if (Seg_Disp_Mode == 3)
					Clock_Set[Clock_Set_Index] = 5;
				else if (Seg_Disp_Mode == 4)
					Alarm_Set[Clock_Set_Index] = 5;
				Clock_Set_Index++;
				break;
			case 17:
				if (Seg_Disp_Mode == 3)
					Clock_Set[Clock_Set_Index] = 6;
				else if (Seg_Disp_Mode == 4)
					Alarm_Set[Clock_Set_Index] = 6;
				Clock_Set_Index++;
				break;
			case 10:
				if (Seg_Disp_Mode == 3)
					Clock_Set[Clock_Set_Index] = 7;
				else if (Seg_Disp_Mode == 4)
					Alarm_Set[Clock_Set_Index] = 7;
				Clock_Set_Index++;
				break;
			case 14:
				if (Seg_Disp_Mode == 3)
					Clock_Set[Clock_Set_Index] = 8;
				else if (Seg_Disp_Mode == 4)
					Alarm_Set[Clock_Set_Index] = 8;
				Clock_Set_Index++;
				break;
			case 18:
				if (Seg_Disp_Mode == 3)
					Clock_Set[Clock_Set_Index] = 9;
				else if (Seg_Disp_Mode == 4)
					Alarm_Set[Clock_Set_Index] = 9;
				Clock_Set_Index++;
				break;
			}
		}
	}
}

/* 信息处理函数 */
void Seg_Proc()
{
	unsigned char i;
	//	if(Seg_Slow_Down) return;
	//	Seg_Slow_Down = 1;//数码管减速程序

	switch (Seg_Slow_Down)
	{
	case 100: // 时钟读取
		Seg_Slow_Down += 1;
		Read_Rtc(Clock_Crl);
		break;
	case 200: // 超声波
		Seg_Slow_Down += 1;
		ultrasonic = Ut_Wave_Data();
		if ((ultrasonic_Ture - ultrasonic < 10) || (ultrasonic - ultrasonic_Ture < 10))
			ultrasonic_Ture = ultrasonic;
		break;
	case 300: // AD
		Seg_Slow_Down += 1;
		AD_Output = Ad_Read(0x43);			  // 光敏电阻
		AD_Rb2_Output = Ad_Read(0x41) / 51.0; // 电位器输出
		Da_Write(DA_Output);				  // DA输出
		Sun_Flag = (AD_Output > 100);
		break;
	case 400: // 温度
		Seg_Slow_Down += 1;
		Temp = rd_temperature();
		break;
	}

	Seg_Point[5] = (Seg_Disp_Mode == 2);
	switch (Seg_Disp_Mode)
	{
	case 0: // 系统初始界面
		if (Pass_Input_Index != 0)
		{
			for (i = 0; i < Pass_Input_Index; i++)
				Seg_Buf[7 - i] = Pass_Input[Pass_Input_Index - i - 1];
		}
		else
		{
			for (i = 0; i < 8; i++)
				Seg_Buf[i] = 11;
		}
		break;
	case 1: // 主界面
		for (i = 0; i < 3; i++)
		{
			Seg_Buf[i * 2 + i] = Clock_Crl[i] / 10;
			Seg_Buf[i * 2 + i + 1] = Clock_Crl[i] % 10;
		}
		Seg_Buf[2] = Seg_Buf[5] = Seg_Star_Flag ? 10 : 11;
		break;
	case 2: // 超声波 光 温度
		Seg_Buf[0] = ultrasonic_Ture / 10;
		Seg_Buf[1] = ultrasonic_Ture % 10;
		Seg_Buf[2] = (unsigned char)Sun_Flag;
		Seg_Buf[3] = 11;
		Seg_Buf[4] = (unsigned char)Temp / 10;
		Seg_Buf[5] = (unsigned char)Temp % 10;
		Seg_Buf[6] = (unsigned int)(Temp * 10) % 10;
		Seg_Buf[7] = 12;
		break;
	case 3: // 时钟设置
		Seg_Buf[0] = Clock_Set[0];
		Seg_Buf[1] = Clock_Set[1];
		Seg_Buf[3] = Clock_Set[2];
		Seg_Buf[4] = Clock_Set[3];
		Seg_Buf[6] = Clock_Set[4];
		Seg_Buf[7] = Clock_Set[5];

		Seg_Buf[2] = Seg_Buf[5] = Seg_Star_Flag2 ? 10 : 11;
		break;
	case 4: // 闹钟设置
		Seg_Buf[0] = Alarm_Set[0];
		Seg_Buf[1] = Alarm_Set[1];
		Seg_Buf[3] = Alarm_Set[2];
		Seg_Buf[4] = Alarm_Set[3];
		Seg_Buf[6] = Alarm_Set[4];
		Seg_Buf[7] = Alarm_Set[5];

		Seg_Buf[2] = Seg_Buf[5] = Seg_Star_Flag2 ? 10 : 11;
		break;
	}
}

/* 其他显示函数 */
void Led_Proc()
{

	ucLed[0] = (Seg_Disp_Mode == 0);
	ucLed[1] = (Seg_Disp_Mode == 1);
	ucLed[2] = (Seg_Disp_Mode == 3);
	ucLed[3] = (Seg_Disp_Mode == 4);
	ucLed[4] = (Seg_Disp_Mode == 2);

	Led_Lever = ((AD_Rb2_Output) * 2.0);
	// DA
	DA_Output = (Sun_Flag ? 1 : 5);
}

/*串口*/
void Uart_Sent_Proc()
{
	if (Uart_Slow_Down)
		return;
	Uart_Slow_Down = 1;
	// 每到一个整点后单片机向上位机发送指令
	if (Clock_Crl[1] == 0 && Clock_Crl[2] == 0) // 整点
	{
		printf("Time = %d:%d:%d", (unsigned int)Clock_Crl[0], (unsigned int)Clock_Crl[1], (unsigned int)Clock_Crl[2]);
	}

	if (Seg_Disp_Mode == 1) // 时钟显示界面
	{
		if (Timer_Count == 2) // 在时钟显示界面时每间隔三秒单片机向上位机发送指令
			printf("%d:%d-%dCM_%dC", (unsigned int)Clock_Crl[0], (unsigned int)Clock_Crl[1], (unsigned int)ultrasonic_Ture, (unsigned int)Temp);
	}
}

/*串口处理函数*/
void Uart_Proc()
{
	unsigned char a; // For循环
	unsigned char i; // while循环
	if (Uart_Recv_Index == 0)
		return;
	if (Syt_Flag >= 10)
	{
		Syt_Flag = Time_Syt_Flag = 0; // 复位
		if (Uart_Enable_Flag)		  // 串口开始使能
		{
			if ((Seg_Disp_Mode == 1 || Seg_Disp_Mode == 2))
			{
				if (Uart_Recv_Index == 4) // 主界面串口功能
				{
					if (Uart_Recv[0] == 'D' && Uart_Recv[1] == 'I' && Uart_Recv[2] == 'S' && Uart_Recv[3] == 'P') // 信息显示
					{
						Seg_Disp_Mode = 2; // 信息显示界面
					}
					if (Uart_Recv[0] == 'C' && Uart_Recv[1] == 'K' && Uart_Recv[2] == 'G' && Uart_Recv[3] == 'B') // 串口关
					{
						Uart_Enable_Flag = 0; // 关闭串口功能
						Seg_Disp_Mode = 1;
					}
					if (Uart_Recv[0] == 'S' && Uart_Recv[1] == 'Z' && Uart_Recv[2] == 'S' && Uart_Recv[3] == 'Z') // 时钟设置
					{
						Seg_Disp_Mode = 3; // 时钟设置界面
					}
					if (Uart_Recv[0] == 'N' && Uart_Recv[1] == 'Z' && Uart_Recv[2] == 'S' && Uart_Recv[3] == 'Z') // 闹钟设置
					{
						Seg_Disp_Mode = 4; // 闹钟设置界面
					}
					if (Uart_Recv[0] == 'N' && Uart_Recv[1] == 'Z' && Uart_Recv[2] == 'K' && Uart_Recv[3] == 'G') // 闹钟设置
					{
						Alarm_Enable_Flag = 1; // 闹钟使能
					}
				}
			}
			if ((Seg_Disp_Mode == 3 || Seg_Disp_Mode == 4))
			{
				if (Uart_Recv_Index == 6) // 时钟闹钟设置
				{
					// 小时设置：HSetXX
					if (Uart_Recv[0] == 'H' && Uart_Recv[1] == 'S' && Uart_Recv[2] == 'e' && Uart_Recv[3] == 't') // 闹钟设置
					{
						if (((Uart_Recv[4] >= '0') && (Uart_Recv[4] <= '9')) && ((Uart_Recv[5] >= '0') && (Uart_Recv[5] <= '9'))) // 0-9
						{
							if (Seg_Disp_Mode == 3) // 时钟设置
							{
								Clock_Set[0] = Uart_Recv[4] - 48;
								Clock_Set[1] = Uart_Recv[5] - 48;
							}
							else // 闹钟设置
							{
								Alarm_Set[0] = Uart_Recv[4] - 48;
								Alarm_Set[1] = Uart_Recv[5] - 48;
							}
						}
					}
					// 分钟设置：MSetXX
					if (Uart_Recv[0] == 'M' && Uart_Recv[1] == 'S' && Uart_Recv[2] == 'e' && Uart_Recv[3] == 't') // 闹钟设置
					{
						if (((Uart_Recv[4] >= '0') && (Uart_Recv[4] <= '9')) && ((Uart_Recv[5] >= '0') && (Uart_Recv[5] <= '9'))) // 0-9
						{
							if (Seg_Disp_Mode == 3)
							{
								Clock_Set[2] = Uart_Recv[4] - 48;
								Clock_Set[3] = Uart_Recv[5] - 48;
							}
							else
							{
								Alarm_Set[2] = Uart_Recv[4] - 48;
								Alarm_Set[3] = Uart_Recv[5] - 48;
							}
						}
					}
					// 秒钟设置：SSetXX
					if (Uart_Recv[0] == 'S' && Uart_Recv[1] == 'S' && Uart_Recv[2] == 'e' && Uart_Recv[3] == 't') // 闹钟设置
					{
						if (((Uart_Recv[4] >= '0') && (Uart_Recv[4] <= '9')) && ((Uart_Recv[5] >= '0') && (Uart_Recv[5] <= '9'))) // 0-9
						{
							if (Seg_Disp_Mode == 3)
							{
								Clock_Set[4] = Uart_Recv[4] - 48;
								Clock_Set[5] = Uart_Recv[5] - 48;
							}
							else
							{
								Alarm_Set[4] = Uart_Recv[4] - 48;
								Alarm_Set[5] = Uart_Recv[5] - 48;
							}
						}
					}
				}
				if (Uart_Recv_Index == 4) // 关闭串口
				{
					if (Uart_Recv[0] == 'C' && Uart_Recv[1] == 'K' && Uart_Recv[2] == 'G' && Uart_Recv[3] == 'B') // 串口关
						Uart_Enable_Flag = 0;
				}
			}
		}
		if (Seg_Disp_Mode == 1) // 在时钟显示界面时上位机可发送修改密码指令
		{
			if (Uart_Recv_Index == 10)
			{
				if (Uart_Recv[0] == 'X' && Uart_Recv[1] == 'G')
				{
					i = 2;
					while (((Uart_Recv[i] >= '0') && (Uart_Recv[4] <= '9')))
					{
						i++;
						if (i == 8)
							break;
					}
					if (i == 8)
					{
						for (a = 0; a < 8; a++)
						{
							Pass_Word[a] = Uart_Recv[a + 2] - 48;
							Seg_Disp_Mode = 0;
						}
					}
				}
			}
		}
		memset(Uart_Recv, 0, Uart_Recv_Index);
		Uart_Recv_Index = 0;
	}
}
/*定时器1中断初始化函数*/
void Timer1_Init(void) // 125微秒@12.000MHz
{
	AUXR &= 0xBF; // 定时器时钟12T模式
	TMOD &= 0x0F; // 设置定时器模式
	TL1 = 0x83;	  // 设置定时初始值
	TH1 = 0xFF;	  // 设置定时初始值
	TF1 = 0;	  // 清除TF1标志
	TR1 = 1;	  // 定时器1开始计时
	ET1 = 1;	  // 定时器中断1打开
}

/* 定时器0中断初始化函数 */
void Timer0Init(void) // 1毫秒@12.000MHz
{
	AUXR &= 0x7F; // 定时器时钟12T模式
	TMOD &= 0xF0; // 设置定时器模式
	TL0 = 0x18;	  // 设置定时初始值
	TH0 = 0xFC;	  // 设置定时初始值
	TF0 = 0;	  // 清除TF0标志
	TR0 = 1;	  // 定时器0开始计时
	ET0 = 1;	  // 定时器中断0打开
	EA = 1;		  // 总中断打开
}

/* 定时器0中断服务函数 */
void Timer0Server() interrupt 1
{
	if (++Uart_Slow_Down == 1000)
	{
		Uart_Slow_Down = 0;
		if (++Timer_Count == 3)
			Timer_Count = 0;
	}
	if (++Key_Slow_Down == 10)
		Key_Slow_Down = 0; // 键盘减速专用
	if (++Seg_Slow_Down == 500)
		Seg_Slow_Down = 0; // 数码管减速专用
	if (++Seg_Pos == 8)
		Seg_Pos = 0; // 数码管显示专用
	Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], Seg_Point[Seg_Pos]);

	if (Key_Flag == 1)
	{
		if (++Timer_3000Ms == 4000) // 长按
		{
			Timer_3000Ms = 4000;
		}
	}

	if (++Timer_500Ms == 500) // 时间间隔符闪烁
	{
		Timer_500Ms = 0;
		Seg_Star_Flag ^= 1;
	}

	if (++Timer_700Ms == 700) // 时间闹钟设置间隔符
	{
		Timer_700Ms = 0;
		Seg_Star_Flag2 ^= 1;
	}

	if (Key_Flag1)
	{
		if (++Timer_2000Ms == 3000) // 长按
			Timer_2000Ms = 3000;
	}

	// 串口
	if (Time_Syt_Flag)
		Syt_Flag++;

	// led亮度等级
	if (++Term_Led == 10)
		Term_Led = 0;
}

/*定时器1服务函数*/
void Timer1Server() interrupt 3
{
	if (++Led_Pos == 8)
		Led_Pos = 0;

	if (Term_Led <= Led_Lever)
		Led_Disp(Led_Pos, ucLed[Led_Pos]);
	else
		Led_Disp(Led_Pos, 0);
}

/*串口*/
void Uart1Server() interrupt 4
{
	if (RI == 1) // 开始接受数据
	{
		Time_Syt_Flag = 1;
		Syt_Flag = 0;
		Uart_Recv[Uart_Recv_Index] = SBUF;
		Uart_Recv_Index++;
		RI = 0;
	}
	if (Uart_Recv_Index > 10)
		Uart_Recv_Index = 0;
}

/* Main */
void main()
{

	while (rd_temperature() == 85)
		;
	System_Init();
	Timer0Init();
	Set_Rtc(Clock_Crl);
	UartInit();
	E2PROM_Write(E2PROM_Alarm, 0, 3);
	Alarm[0] = E2PROM_Alarm[0];
	Alarm[1] = E2PROM_Alarm[1];
	Alarm[2] = E2PROM_Alarm[2];
	E2PROM_Write(Clock, 8, 3);
	Clock_Crl[0] = Clock[0];
	Clock_Crl[1] = Clock[1];
	Clock_Crl[2] = Clock[2];

	Timer1_Init();
	while (1)
	{
		Uart_Sent_Proc();
		Key_Proc();
		Seg_Proc();
		Led_Proc();
		Uart_Proc();
	}
}