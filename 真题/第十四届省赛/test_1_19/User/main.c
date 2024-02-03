/* ͷ�ļ������� */
#include <STC15F2K60S2.H> //��Ƭ���Ĵ���ר��ͷ�ļ�
#include <Init.h>         //��ʼ���ײ�����ר��ͷ�ļ�
#include <Led.h>          //Led�ײ�����ר��ͷ�ļ�
#include <Key.h>          //�����ײ�����ר��ͷ�ļ�
#include <Seg.h>          //����ܵײ�����ר��ͷ�ļ�
#include <stdio.h>        //��׼��ײ�����ר��ͷ�ļ�
#include <iic.h>          //��ģת���ײ�����
#include "onewire.h"      //�¶ȴ���������
#include "ds1302.h"       //ʱ�ӵײ�����

/* ���������� */
idata unsigned char Key_Val, Key_Down, Key_Old, Key_Up;              // ����ר�ñ���
idata unsigned char Key_Slow_Down;                                   // ��������ר�ñ���
idata unsigned char Seg_Buf[8]   = {10, 10, 10, 10, 10, 10, 10, 10}; // �������ʾ���ݴ������
idata unsigned char Seg_Point[8] = {0, 0, 0, 0, 0, 0, 0, 0};         // �����С�������ݴ������
idata unsigned char Seg_Pos;                                         // �����ɨ��ר�ñ���
idata unsigned char Seg_Slow_Down;                                   // ����ܼ���ר�ñ���
idata unsigned char ucLed[8] = {0, 0, 0, 0, 0, 0, 0, 0};             // Led��ʾ���ݴ������

idata unsigned short time_3s      = 0;                  // 3s��ʱ
idata unsigned short time_2s      = 0;                  // 2s��ʱ
idata unsigned short time_1s      = 0;                  // 1s��ʱ
idata unsigned char ucRtc[3]      = {0x23, 0x11, 0x59}; // ����ʱ�ӵ��ϵ��ʼ״̬
idata unsigned char Seg_show_mode = 0;                  // �������ʾ��ģʽ
// 0��ʾʱ���룬1��ʾC�¶ȣ�2��ʾHʪ�ȣ�3��ʾF��������+�ɼ���ʱ�֣�4��ʾP�¶Ȳ���
idata unsigned char show_flag      = 0; // ʱ��->����->����----->��ʪ�Ȳɼ���ʾ
idata unsigned char re_show_flag   = 0; // �¶�->ʪ��->ʱ��
idata unsigned char light_flag     = 0; // 1�Ǵӹ��ձ�Ϊ�ڰ�
idata unsigned char temp_show_flag = 0; // ��¼������ʪ�Ȳɼ�����һ�̵���ʾ

/*�¶Ƚ���*/
idata float max_temperature           = 0; // ����¶�
 idata float sum_temperature           = 0; // ���¶�
 idata unsigned char index_temperature = 0; // �¶ȼ����
   idata float temp_temperature;
  
  idata  float aver_temperature;
   
/*ʪ�Ƚ���*/
idata unsigned short count_f       = 0; // Ƶ�ʼ���
idata unsigned short dat_f         = 0; // 1s��Ƶ��
idata float max_humidity           = 0; // ���ʪ��
idata float sum_humidity           = 0; // ��ʪ��
idata unsigned char index_humidity = 0; // ʪ�ȼ����
 idata float temp_humidity;
 idata float aver_humidity;
/*��������*/
idata unsigned char temperature_ref = 30; // �ο��¶�
                                          /*��������+ʱ����ʾ*/
idata unsigned char triggers_number = 0;  // ��������

/* ���̴����� */
void Key_Proc() // 10msɨ��һ��
{
    if (Key_Slow_Down) return;
    Key_Slow_Down = 1;                              // ���̼��ٳ���
    Key_Val       = Key_Read();                     // ʵʱ��ȡ����ֵ
    Key_Down      = Key_Val & (Key_Old ^ Key_Val);  // ��׽�����½���
    Key_Up        = ~Key_Val & (Key_Old ^ Key_Val); // ��׽�����Ͻ���
    Key_Old       = Key_Val;                        // ����ɨ�����
    // ������9���Ҵ���ʱ�����
    if (Key_Val == 9 && re_show_flag == 2) {
        time_2s++;
        triggers_number = 0;
    } else {
        time_2s = 0;
    }
    // ����2s
    if (time_2s >= 100) {
        triggers_number = 0;
    }
    // ���������һ���¶Ȳɼ�
    if (temp_show_flag != 0) {
        show_flag      = temp_show_flag;
        temp_show_flag = 0;
    }
    // ���½��水ť
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
        time_3s        = 0;   // �����ڿ�ʼ��¼3s
        show_flag      = 100; // ���ʱ�򴥷��ɼ������ǽ�����ʪ�Ƚ���
        light_flag     = 0;
    }
}
/* ��Ϣ������ */
void Seg_Proc()
{
    unsigned char int_temp_temperature; // ���������¶�
    unsigned char int_temp_humidity;    // ��������ʪ��

    if (Seg_Slow_Down) return;
    Seg_Slow_Down = 1; // ����ܼ��ٳ���

    /*����ܶ�ȡʱ��*/
    Read_Rtc(ucRtc); // ��ȡDS1302�����ʱ��

    /*����ܶ�ȡ�¶Ȳ�����*/
    temp_temperature = rd_temperature();
    index_temperature++;
    // ��������Ϊ�˷�ֹоƬ��ȡ������Ǹ�85��
    if (temp_temperature == 85) {
        temp_temperature = 0;
        index_temperature--;
    }
    sum_temperature += temp_temperature;
    aver_temperature = sum_temperature / index_temperature; // ����ƽ���¶�
    if (max_temperature < temp_temperature) {
        max_temperature = temp_temperature;
    }
    int_temp_temperature = temp_temperature * 10;

    /*����ܶ�ȡʪ�Ȳ�����*/
    if (dat_f < 200 || dat_f > 2000) {
        temp_humidity = 0; // ��Ч����
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
    /*����ܶ�ȡ�ⰵ��ֵ������*/
    // triggers_number++;
    if (triggers_number >= 100) {
        triggers_number = 0;
    }
    /*�������ʾ*/
    switch (Seg_show_mode) // �������ʾģʽ
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
                Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = Seg_Buf[6] = Seg_Buf[7] = 10; // ��
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
            Seg_Buf[1] = Seg_Buf[2] = Seg_Buf[3] = Seg_Buf[4] = Seg_Buf[5] = 10; // ��
            break;
        case 5:
            Seg_Buf[0] = 17; // E
            Seg_Buf[3] = (int)temp_temperature / 10;
            Seg_Buf[4] = (int)temp_temperature % 10;
            Seg_Buf[5] = 12;              //-
            Seg_Buf[1] = Seg_Buf[2] = 10; // ��
            break;
        default:
            break;
    }
}

/* ������ʾ���� */
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

/* ��ʱ��0�жϳ�ʼ������ */
void Timer0Init(void) // 1����@12.000MHz
{
    AUXR &= 0x7F; // ��ʱ��ʱ��12Tģʽ
    TMOD &= 0xF0; // ���ö�ʱ��ģʽ
    TL0 = 0x18;   // ���ö�ʱ��ʼֵ
    TH0 = 0xFC;   // ���ö�ʱ��ʼֵ
    TF0 = 0;      // ���TF0��־
    TR0 = 1;      // ��ʱ��0��ʼ��ʱ
    ET0 = 1;      // ��ʱ���ж�0��
    EA  = 1;      // ���жϴ�
}
/*������1�жϳ�ʼ������*/
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
/* ��ʱ��0�жϷ����� */
void Timer0Server() interrupt 1
{
    if (++Key_Slow_Down == 10) Key_Slow_Down = 0;  // ���̼���ר��
    if (++Seg_Slow_Down == 200) Seg_Slow_Down = 0; // ����ܼ���ר��
    if (++Seg_Pos == 8) Seg_Pos = 0;               // �������ʾר��
    if (++time_3s >= 3000)                         // ����ʱ3sʱ
    {
        time_3s    = 3001; // ��ֹ���
        light_flag = 0;
    }
    if (++time_1s >= 1000) {
        // ����1s,������ͳ��һ��Ƶ��
        time_1s = 0;
        dat_f   = count_f; // ȡ����1s��Ƶ��
        count_f = 0;       // ����ֵ����
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
    Set_Rtc(ucRtc); // �ϵ��ʼ��ʱ��
    while (1) {
        Key_Proc();
        Seg_Proc();
        Led_Proc();
        deal_mode();
    }
}