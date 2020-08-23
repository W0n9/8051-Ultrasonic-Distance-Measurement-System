#include "reg51.h"
#include "stdio.h"
sbit F1 = P1 ^ 0; //F1���Ǵ���P1^0�@�b�_λ
sbit F2 = P1 ^ 1; //F2���Ǵ���P1^1�@�b�_λ
sbit F3 = P1 ^ 2; //F3���Ǵ���P1^2�@�b�_λ
sbit F4 = P1 ^ 3; //F4���Ǵ���P1^3�@�b�_λ

void UART();
void delay(int);

void UARTOFF(); //�P�]9600RF��ֵ
unsigned char sbuf, y;
char a, b, c;
bit x1 = 0;

main()
{

    while (1)
    {

        UART(); //�_��9600RF��ֵ
        putchar('Q');
        delay(10);
    }
    UARTOFF();
}

void UART() //��ͬ����ݔ
{
    SCON = 0x52; //���п��ƕ�����01010010 TI=1 REN=1 01 8λԪUART��ݔ(8-11)
    TMOD = 0x20; //Ӌ�rӋ��ģʽ������ 00100000    �O�����r��1�鹤����ʽ2
    TCON = 0x69; //Ӌ�rӋ�����ƕ����� 01101001
    TH1 = 0xFD;  //Ӌ�rӋ��������  <230> �U��1200
    TR1 = 1;     //Ӌ�rӋ�����ƕ������e���TR1  //�O�ò����� 9600bps
    ES = 1;      //�����Д�����
    EA = 1;      //ȫ���Д�Դ����
}

void SCON_int(void) interrupt 4 //���и��Дຯ��(SCON���п��ƕ����������Ђ�ݔ�Д� ����̖�Дྎ̖)
{
    if (RI == 1) //RI���Д�������Ĺ����ǽ��գ�������գ���4̖�Дྎ̖��RI��TI��
                 // �ңɵ��÷��Ǯ��ͣã����һ�P�Y�ϵĽ����ᣬӲ�w���Ԅӌ�����O�飱��
    //��ϣţ��������O������������������Ĵ��и���ʽ
    {
        RI = 0;      // ��RI׃��0  ���Д���Ҫ�ք���RIλַ��RI��λ��ʾ�����ꮅ�����S�´ν���
        sbuf = SBUF; //�xȡ�Y�ϣ��ѕ������Y�Ϸŵ����x�� sbuf�e��  (SBUF�Ǵ��Е�����)
    }
}

void delay(int count)
{
    int i, j;

    for (i = 0; i < count; i++)
        for (j = 0; j < 1000; j++)
            ;
}

void UARTOFF() //9600 RF�P�]��
{
    IE = 0x82;
    SCON = 0x52;
    TMOD = 0x20;
    TCON = 0x69;
    TH1 = 0xFD;
    TR1 = 1;
    ES = 0; //�@�����O���_�c�P,����������ES�cEA�O����0,9600RF����P����
    EA = 0;
}
//=========================================//