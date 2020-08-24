/**
 * *********************************************
 * 
 * 8051 Ultrasonic Distance Measurement System
 * 
 * Encoding = GBK
 * 
 * Author:W0n9, SailorYan
 * 
 * *********************************************
*/

#include "REG52.H"
#include "intrins.h"
// #include "lcd1602.h"

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef unsigned long uint32_t;

typedef signed char int8_t;
typedef signed int int16_t;
typedef signed long int32_t;

#define XTAL11M
// #define XTAL12M

#define EXT0_VECTOR 0  /* 0x03 external interrupt 0 */
#define TIM0_VECTOR 1  /* 0x0b timer 0 */
#define EXT1_VECTOR 2  /* 0x13 external interrupt 1 */
#define TIM1_VECTOR 3  /* 0x1b timer 1 */
#define UART0_VECTOR 4 /* 0x23 serial port 0 */

// sbit RCK_STMM = P2 ^ 7;
// sbit DIO_DS = P2 ^ 6;
// sbit SCK_SHBit = P2 ^ 5;

sbit Trig_Pin = P2 ^ 1;
sbit Echo_Pin = P2 ^ 0;
sbit Mode_1 = P1 ^ 1;
sbit Mode_2 = P1 ^ 2;
sbit Mode_3 = P1 ^ 3;

sbit lcden = P3 ^ 4; //使能端:en=1,读取；en=下降沿，执行指令
sbit lcdrs = P3 ^ 5; //数据/指令选择端：rs=1，为数据；rs=0，为指令。
sbit lcdrw = P3 ^ 6; //读写控制端：rw=1,读；rw=0,写
// int W1,W2,W3,W4,W5,W6,W7;

/***********延时函数******************/
void delayms(uint16_t time)
{
    uint16_t i;
    for (; time > 0; time--)
    {
        for (i = 0; i < 124; i++)
        {
            ;
        }
    }
}

/*******************lcd写入指令子函数********************/
void lcd_wcom(uint8_t com)
{
    lcdrs = 0;  // rs=0,指令
    lcdrw = 0;  // rw=0,写入
    P0 = com;   // 把指令送入P0口
    delayms(5); //延时一小会 ，让1602准备接收数据--------代替忙检测函数
    lcden = 1;  //使能端 下降沿，把指令送入1602的8为数据口
    delayms(2);
    lcden = 0;
}

/************************lcd写入数据函数************************/
void lcd_wdata(uint8_t dat)
{
    lcdrs = 1;  // RS=1,数据
    lcdrw = 0;  // rw=0,写入
    P0 = dat;   // 把要显示的数据送入P0口
    delayms(5); //延时一小会 ，让1602准备接收数据--------代替忙检测函数
    lcden = 1;  //使能端 下降沿，把指令送入1602的8为数据口
    delayms(2);
    lcden = 0;
}

/******************LCD初始化子函数****************************/
void lcd_init() //初始化，写入指令
{
    lcdrw = 0; //rw=0,写
    lcden = 1;
    lcden = 0;      //下降沿，执行指令
    lcd_wcom(0x38); //写指令,8位数据，双列
    lcd_wcom(0x0c); //开启显示屏，关光标，光标不闪烁
    lcd_wcom(0x06); //显示地址递增，即写一个数据后，显示位置右移一位
    lcd_wcom(0x01); //清屏
}

/**********************LCD显示子函数****************************/
//i设置字符位置，j是显示的数字
void lcd_display(uint16_t i, uint16_t j)
{

    lcd_wcom(0x80 + i - 1);
    switch (j)
    {
    case 0:
        lcd_wdata('0');
        delayms(2);
        break;
    case 1:
        lcd_wdata('1');
        delayms(2);
        break;
    case 2:
        lcd_wdata('2');
        delayms(2);
        break;
    case 3:
        lcd_wdata('3');
        delayms(2);
        break;
    case 4:
        lcd_wdata('4');
        delayms(2);
        break;
    case 5:
        lcd_wdata('5');
        delayms(2);
        break;
    case 6:
        lcd_wdata('6');
        delayms(2);
        break;
    case 7:
        lcd_wdata('7');
        delayms(2);
        break;
    case 8:
        lcd_wdata('8');
        delayms(2);
        break;
    case 9:
        lcd_wdata('9');
        delayms(2);
        break;
    default:
        lcd_wdata('.');
        delayms(2);
        break;
    }
}
/*********************最终显示函数*************************************/
//abc分别为百十个位，使用前需lcd_init()初始化
void fin_display(uint16_t a, b, c)
{
    lcd_wcom(0x80 + 7 - 1);
    lcd_wdata('M');
    delayms(2);

    lcd_wcom(0x80 + 6 - 1);
    lcd_wdata('C');
    delayms(2);

    lcd_display(5, 5);
    lcd_display(4, '.');
    lcd_display(3, c);
    lcd_display(2, b);
    lcd_display(1, a);
}
/**************************主函数*******************************/
// void main()
// {
// 	lcd_init();//初始化
// 	while(1)
//     {
// 	//lcd_display(10,W1);
// 	//lcd_display(9,':');
// 	// lcd_display(8,1);
// 	// lcd_display(7,W3);
//  	// lcd_display(6,':');
//  	// lcd_display(5,W4);
//   	// lcd_display(4,W5);
//  	lcd_display(10,3);
// 	lcd_display(2,2);
//     lcd_display(1,1);
// 	}
// }

void UART_init() //非同步傳輸
{
    SCON = 0x52; //串列控制暫存器01010010 TI=1 REN=1 01 8位元UART傳輸(8-11)
    TMOD = 0x20; //計時計數模式暫存器 00100000    設定定時器1為工作方式2
    TCON = 0x69; //計時計數控制暫存器 01101001
    TH1 = 0xFD;  //計時計數暫存器  <230> 鮑率1200
    TR1 = 1;     //計時計數控制暫存器裡面的TR1  //設置波特率 9600bps
    ES = 1;      //串列中斷致能
    EA = 1;      //全部中斷源致能
}

void UARTSnd(uint8_t dat)
{

    TI = 0;     // clear transmit interrupt flag
    SBUF = dat; // start sending one byte
    while (!TI)
        ; // wait until sent
}

// void Delay10us() //@11.0592MHz
// {
//     uint8_t tempi;

//     _nop_();
//     tempi = 25;
//     while (--tempi)
//         ;
// }

// void Timer0Init(void) //10毫秒@11.0592MHz
// {
//     // AUXR &= 0x7F; //定时器时钟12T模式
//     TMOD &= 0xF0; //设置定时器0模式
//     TL0 = 0x00;   //设置定时初值
//     TH0 = 0xDC;   //设置定时初值
//     TF0 = 0;      //清除TF0标志
//     TR0 = 1;      //定时器0开始计时
// }

#ifdef XTAL11M
uint32_t MeterByTrig()
{
    // uint8_t tempa;
    uint8_t tempi;
    // uint8_t IsOverFlow;
    uint32_t PresentTime;
    // IsOverFlow = 0;

    //Delay1Ms();

    // P1M1 &= 0X7F;
    // P1M2 &= 0X7F;

    // P1M1 |= 0X40;
    // P1M2 &= 0XBF;

    //Timer Init
    TMOD &= 0xF1;
    // TAMOD = 0X00;
    // TCON = 0X00;
    TH0 = 0;
    TL0 = 0;

    // Module Start
    Trig_Pin = 0;
    Trig_Pin = 1;
    _nop_();
    tempi = 25;
    while (--tempi)
        ;
    Trig_Pin = 0;

    // while (1)
    // {
    //     if (Echo_Pin == 1)
    //     {            //Start Timer;
    //         TR0 = 1; //start timer
    //         break;
    //     }
    //     if (TF0 == 1)
    //     {
    //         IsOverFlow = 1;
    //         break; //overflow;
    //     }
    // }

    // while (1)
    // {
    //     if (Echo_Pin == 0)
    //     { //Stop Timer;
    //         TR0 = 0;
    //         break;
    //     }
    //     if (TF0 == 1)
    //     {
    //         IsOverFlow = 1;
    //         break; //overflow;
    //     }
    // }

    while (!Echo_Pin)
        ;
    TR0 = 1;
    while (Echo_Pin)
        ;
    TR0 = 0;

    //	uint32_t PresentTime, tempb;
    PresentTime = TH0;
    PresentTime <<= 8; // Equal TH0 * 256
    PresentTime += TL0;
    //921600 Cycles @11.0592MHz
    // PresentTime /= 921.6;
    // PresentTime /= 922;
    // PresentTime *= 17;
    //  TODO
    // PresentTime = (uint32_t)(PresentTime * 0.0184);
    PresentTime = (uint32_t)(PresentTime * 0.0023);

    // tempb = PresentTime / 137;
    // PresentTime /= 7; //*3

    // PresentTime -= tempb; //time, us

    // tempb = PresentTime / 150;
    // PresentTime /= 3;
    // PresentTime += tempb;
    //Delay1Ms();
    return PresentTime;
}
#endif

#ifdef XTAL12M
uint32_t MeterByTrig()
{
    uint8_t tempa;
    uint8_t IsOverFlow;
    uint32_t PresentTime;
    IsOverFlow = 0;

    //Delay1Ms();

    // P1M1 &= 0X7F;
    // P1M2 &= 0X7F;

    // P1M1 |= 0X40;
    // P1M2 &= 0XBF;

    //Timer Init
    TMOD &= 0xF5;
    // TAMOD = 0X00;
    TCON = 0X00;
    TH1 = 0;
    TL1 = 0;

    Trig_Pin = 0;
    for (tempa = 0; tempa < 10; tempa++) //up for 10us
    {
        Trig_Pin = 1;
    }
    Trig_Pin = 0;

    while (1)
    {
        if (Echo_Pin == 1)
        {            //Start Timer;
            TR0 = 1; //start timer
            break;
        }
        if (TF0 == 1)
        {
            IsOverFlow = 1;
            break; //overflow;
        }
    }

    while (1)
    {
        if (Echo_Pin == 0)
        { //Stop Timer;
            TR0 = 0;
            break;
        }
        if (TF0 == 1)
        {
            IsOverFlow = 1;
            break; //overflow;
        }
    }

    //	uint32_t PresentTime, tempb;
    PresentTime = TH1;
    PresentTime <<= 0X08;
    PresentTime += TL1;
    //1000000 Cycles @12MHz
    PresentTime /= 1000;
    PresentTime *= 17;

    // tempb = PresentTime / 137;
    // PresentTime /= 7; //*3

    // PresentTime -= tempb; //time, us

    // tempb = PresentTime / 150;
    // PresentTime /= 3;
    // PresentTime += tempb;
    //Delay1Ms();
    return PresentTime;
}
#endif

void DisplayLength(uint32_t number)
{
    uint8_t Hundred, Decade, Unit;
    if (number > 999)
        number = 0;
    // Thousand = number / 1000;

    // number = number % 1000;
    Hundred = number / 100;

    number = number % 100;
    Decade = number / 10;

    Unit = number % 10;

    fin_display(Hundred, Decade, Unit);
    // DisplayLED(1, Thousand, 0);
    // DisplayLED(2, Hundred, 0);
    // DisplayLED(3, Decade, 0);
    // DisplayLED(4, Unit, 0);
}

// void DisplayLED(uint8_t LEDid, uint8_t Value, uint8_t Isdot)
// {
//     uint8_t tempa, tempb;
//     uint8_t tempc, tempd;
//     const uint8_t NumValue[11] = {0X40, 0X79, 0X24, 0X30, 0X19, 0X12, 0X02, 0X78, 0X00, 0X10, 0X3F}; //0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -.

//     tempa = 1;
//     tempa <<= (LEDid - 1);
//     if (Value >= 11) //value should little than 10
//         Value = 0;
//     tempb = NumValue[Value];
//     if (Isdot == 0)
//         tempb += 0X80;

//     //begin output
//     tempd = 0X80;
//     for (tempc = 0; tempc < 8; tempc++)
//     {
//         SCK_SHBit = 0;
//         SCK_SHBit = 0;
//         DIO_DS = (tempa & tempd);
//         SCK_SHBit = 1;
//         SCK_SHBit = 1;
//         tempd >>= 1;
//     }

//     tempd = 0X80;
//     for (tempc = 0; tempc < 8; tempc++)
//     {
//         SCK_SHBit = 0;
//         SCK_SHBit = 0;
//         DIO_DS = (tempb & tempd);
//         SCK_SHBit = 1;
//         SCK_SHBit = 1;
//         tempd >>= 1;
//     }

//     RCK_STMM = 0;
//     RCK_STMM = 0;
//     RCK_STMM = 0;
//     RCK_STMM = 0;
//     RCK_STMM = 1;
//     RCK_STMM = 1;
//     RCK_STMM = 1;
//     RCK_STMM = 1;
//     //Delay1Ms();
// }

void Delay20ms() //@11.0592MHz
{
    uint8_t i, j;

    i = 36;
    j = 217;
    do
    {
        while (--j)
            ;
    } while (--i);
}

void Delay5s() //@11.0592MHz
{
    uint8_t i, j, k;

    _nop_();
    i = 36;
    j = 5;
    k = 211;
    do
    {
        do
        {
            while (--k)
                ;
        } while (--j);
    } while (--i);
}

void Delay1s() //@11.0592MHz
{
    uint8_t i, j, k;

    _nop_();
    i = 11;
    j = 130;
    k = 111;
    do
    {
        do
        {
            while (--k)
                ;
        } while (--j);
    } while (--i);
}

// void greater_than_2m()
// {
//     uint8_t tempj;
//     uint8_t greater_than_2m[] = {0xFD, 0x00, 0x11, 0x01, 0x01, 0x73, 0x6F, 0x75, 0x6E, 0x64, 0x6B, 0xB4, 0xF3, 0xD3, 0xDA, 0xC1, 0xBD, 0xC3, 0xD7, 0xC2};
//     Delay20ms();
//     for (tempj = 0; tempj < 20; tempj++)
//     {
//         UARTSnd(greater_than_2m[tempj]);
//     }
// }

void greater_than_1m()
{
    uint8_t tempj;
    uint8_t greater_than_1m[] = {0xFD, 0x00, 0x11, 0x01, 0x01, 0x73, 0x6F, 0x75, 0x6E, 0x64, 0x6B, 0xB4, 0xF3, 0xD3, 0xDA, 0xD2, 0xBB, 0xC3, 0xD7, 0xD7};
    Delay20ms();
    for (tempj = 0; tempj < 20; tempj++)
    {
        UARTSnd(greater_than_1m[tempj]);
    }
    Delay5s();
}

void less_than_1m()
{
    uint8_t tempj;
    uint8_t less_than_1m[] = {0xFD, 0x00, 0x0B, 0x01, 0x01, 0xD0, 0xA1, 0xD3, 0xDA, 0xD2, 0xBB, 0xC3, 0xD7, 0xF3};
    Delay20ms();
    for (tempj = 0; tempj < 14; tempj++)
    {
        UARTSnd(less_than_1m[tempj]);
    }
    Delay1s();
}

void main()
{
    // uint8_t tempa;
    uint32_t PreLength; //  Unit is cm
    uint8_t i;
    uint8_t tempj;
    uint8_t greater_than_2m[] = {0xFD, 0x00, 0x11, 0x01, 0x01, 0x73, 0x6F, 0x75, 0x6E, 0x64, 0x6B, 0xB4, 0xF3, 0xD3, 0xDA, 0xC1, 0xBD, 0xC3, 0xD7, 0xC2};
    // uint8_t debug;
    lcd_init();
    UART_init();

    // for (i = 0; i < 1; i++)
    // { // greater_than_2m();

    //     Delay20ms();
    //     for (tempj = 0; tempj < 20; tempj++)
    //     {
    //         UARTSnd(greater_than_2m[tempj]);
    //     }
    // }

    // // greater_than_2m();
    // Delay5s();
    // for (i = 0; i < 3; i++)
    // {
    //     greater_than_1m();
    // }
    // // Delay5s();
    // for (i = 0; i < 3; i++)
    // {
    //     less_than_1m();
    // }

    while (1)
    {
        PreLength = MeterByTrig();
        DisplayLength(PreLength);
        // if (PreLength > 200)
        // {
        //     for (i = 0; i < 1; i++)
        //     { // greater_than_2m();
        //         Delay20ms();
        //         for (tempj = 0; tempj < 20; tempj++)
        //         {
        //             UARTSnd(greater_than_2m[tempj]);
        //         }
        //     }
        // }
        // else if (PreLength <= 200 && PreLength >= 100)
        // {
        //     for (i = 0; i < 3; i++)
        //     {
        //         greater_than_1m();
        //     }
        // }
        // else if (PreLength <= 100)
        // {
        //     for (i = 0; i < 3; i++)
        //     {
        //         less_than_1m();
        //     }
        // }
        if (!Mode_1)
        {
            if (PreLength > 200)
            {
                for (i = 0; i < 1; i++)
                { // greater_than_2m();
                    Delay20ms();
                    for (tempj = 0; tempj < 20; tempj++)
                    {
                        UARTSnd(greater_than_2m[tempj]);
                    }
                }
                Delay5s();
            }
        }
        else if (!Mode_2)
        {
            if (PreLength <= 200 && PreLength >= 100)
            {
                greater_than_1m();
            }
        }
        else if (!Mode_3)
        {
            if (PreLength <= 100)
            {
                less_than_1m();
            }
        }

        // debug = PreLength / 1000;
        // UARTSnd(PreLength);
        // Delay5s();

        // for (tempa = 0; tempa < 200; tempa++)   //  refresh for 200 cycles
        // {
        //     DisplayLength(PreLength);
        // }
    }
}
