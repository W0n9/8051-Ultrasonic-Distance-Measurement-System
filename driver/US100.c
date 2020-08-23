#include "US100.H"

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef unsigned long uint32_t;

typedef signed char int8_t;
typedef signed int int16_t;
typedef signed long int32_t;

#define EXT0_VECTOR 0  /* 0x03 external interrupt 0 */
#define TIM0_VECTOR 1  /* 0x0b timer 0 */
#define EXT1_VECTOR 2  /* 0x13 external interrupt 1 */
#define TIM1_VECTOR 3  /* 0x1b timer 1 */
#define UART0_VECTOR 4 /* 0x23 serial port 0 */

void UART_Init();
uint16_t distance;

uint16_t Ultra_Sonic_Distance()
{
    UART_Init();
    SBUF = 0x55;
    while (1)
    {
        distance = UART();
        return distance;
    }
}

void UART_init()
{
    TMOD = 0x20;
    TH1 = 0xfd;
    TL1 = 0xfd;
    TR1 = 1;
    SM0 = 0;
    SM1 = 1;
    REN = 1;
    EA = 1;
    ES = 1;
}

uint16_t UART() interrupt UART0_VECTOR
{
    static uint8_t count = 2;
    if (RI == 1)
    {
        distance = SBUF;
        RI = 0;
        count = count - 1;
        if (count == 0)
        {
            SBUF = 0x55;
        }
    }
    if (TI == 1)
    {
        TI = 0;
    }
    return distance;
}