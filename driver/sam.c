#include "reg51.h"
#include "stdio.h"
sbit F1 = P1 ^ 0; //F1就是代表P1^0這隻腳位
sbit F2 = P1 ^ 1; //F2就是代表P1^1這隻腳位
sbit F3 = P1 ^ 2; //F3就是代表P1^2這隻腳位
sbit F4 = P1 ^ 3; //F4就是代表P1^3這隻腳位

void UART();
void delay(int);

void UARTOFF(); //關閉9600RF傳值
unsigned char sbuf, y;
char a, b, c;
bit x1 = 0;

main()
{

    while (1)
    {

        UART(); //開啟9600RF傳值
        putchar('Q');
        delay(10);
    }
    UARTOFF();
}

void UART() //非同步傳輸
{
    SCON = 0x52; //串列控制暫存器01010010 TI=1 REN=1 01 8位元UART傳輸(8-11)
    TMOD = 0x20; //計時計數模式暫存器 00100000    設定定時器1為工作方式2
    TCON = 0x69; //計時計數控制暫存器 01101001
    TH1 = 0xFD;  //計時計數暫存器  <230> 鮑率1200
    TR1 = 1;     //計時計數控制暫存器裡面的TR1  //設置波特率 9600bps
    ES = 1;      //串列中斷致能
    EA = 1;      //全部中斷源致能
}

void SCON_int(void) interrupt 4 //串列副中斷函數(SCON串列控制暫存器，串列傳輸中斷 第四號中斷編號)
{
    if (RI == 1) //RI是中斷旗標他的功用是接收，１為接收，在4號中斷編號有RI跟TI，
                 // ＲＩ的用法是當ＭＣＵ完成一筆資料的接收後，硬體會自動將旗標設為１，
    //配合ＥＡ＝１的設定，便會執行相對應的串列副程式
    {
        RI = 0;      // 把RI變成0  在中斷中要手動清RI位址，RI置位表示接收完畢，允許下次接收
        sbuf = SBUF; //讀取資料，把暫存器資料放到定義的 sbuf裡面  (SBUF是串列暫存器)
    }
}

void delay(int count)
{
    int i, j;

    for (i = 0; i < count; i++)
        for (j = 0; j < 1000; j++)
            ;
}

void UARTOFF() //9600 RF關閉用
{
    IE = 0x82;
    SCON = 0x52;
    TMOD = 0x20;
    TCON = 0x69;
    TH1 = 0xFD;
    TR1 = 1;
    ES = 0; //這可以設定開與關,所以倘若把ES與EA設定成0,9600RF便會關起來
    EA = 0;
}
//=========================================//