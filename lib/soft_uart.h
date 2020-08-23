#ifndef __SOFT_UART_H
#define __SOFT_UART_H

#include <STC89C5xRC.H>
// #include "REG52.H"
#include <string.h>

void soft_rs232_init(void);
void soft_send_enable(void);
void soft_rs232_interrupt(void);
void PrintCom(unsigned char *DAT, unsigned char len);
void SYN_FrameInfo(unsigned char Music, unsigned char *HZdata);

#endif
