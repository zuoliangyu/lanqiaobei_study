#include <Led.h>
//�����ַ0-7���Ƿ�ʹ��
void Led_Disp(unsigned char addr,enable)
{
    //��֤������������ֵ������Ϊ�����������ı�
    static unsigned char temp = 0x00;
    static unsigned char temp_old = 0xff;
    //���ĵ�ǰ״̬
    if(enable)
        temp |= 0x01 << addr;
    else
        temp &= ~(0x01 << addr);
    //��ǰ״̬��֮ǰ��״̬��ͬ����в�������led
    if(temp != temp_old)
    {
        P0 = ~temp;
        P2 = P2 & 0x1f | 0x80;
        P2 &= 0x1f;
        temp_old = temp;
    }
}
