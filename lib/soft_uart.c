#include "soft_uart.h"

sfr16 DPTR = 0x82;
#define YES 1
#define NO 0

//����ʹ���ĸ���ʱ��, ֻ�ɶ���һ��
#define TIMER_0
// #define TIMER_1

//���崮���ա����͹ܽš�
sbit rs_TXD = P2 ^ 1;
sbit rs_RXD = P2 ^ 0;

//���ݶ�ʱ��ȷ������
#ifdef TIMER_0
#define TMOD_AND_WORD 0xF0;
#define TMOD_TIME_MODE 0x01;
#define TMOD_COUNT_MODE 0x05; //���ü���ģʽλ
sbit TCON_ENABLE_TIMER = TCON ^ 4;
sbit TCON_TFx = TCON ^ 5; //�жϱ�־λ
sbit IE_ETx = IE ^ 1;	 //�ж�����λΪ ET0
sbit IP_PTx = IP ^ 1;	 //�ж����ȼ�

sfr rs_timerL = 0x8A; //TL0
sfr rs_timerH = 0x8C; //TH0
#endif

#ifdef TIMER_1
#define TMOD_AND_WORD 0x0F;
#define TMOD_TIME_MODE 0x10;
#define TMOD_COUNT_MODE 0x50;	  //���ü���ģʽλ
sbit TCON_ENABLE_TIMER = TCON ^ 6; //
sbit TCON_TFx = TCON ^ 7;		   //�жϱ�־λ
sbit IE_ETx = IE ^ 3;			   //�ж�����λΪ ET1
sbit IP_PTx = IP ^ 4;			   //�ж����ȼ�

sfr rs_timerL = 0x8B; //TL1
sfr rs_timerH = 0x8D; //TH1
#endif

unsigned char bdata rs_BUF;	//�����ա���ʱ�õ���λ�ݴ�����
sbit rs_BUF_bit7 = rs_BUF ^ 7; //��λ�ݴ��������λ��
unsigned char rs_shift_count;  //��λ��������

unsigned char bdata rsFlags;
sbit rs_f_TI = rsFlags ^ 0;		   //0:���ڷ���; 1: һ���ַ����
sbit rs_f_RI_enable = rsFlags ^ 1; //0:��ֹ����; 1:�������
sbit rs_f_TI_enable = rsFlags ^ 2; //0:��ֹ����; 1:������

//ѡ������һ������Ƶ��
//#define Fosc 6000000                 //6MHz
#define Fosc 11059200 //11.059MHz
//#define Fosc 12000000
//#define Fosc 18432000
//#define Fosc 20000000
//#define Fosc 24000000
//#define Fosc 30000000
//#define Fosc 40000000

//ѡ������һ��������:
//#efine Baud 300                      //11.059MHzʱ��baud ���Ϊ 300
//#define Baud 1200
//#define Baud 2400
//#define Baud 4800
#define Baud 9600
//#define Baud 14400
//#define Baud 19200
//#define Baud 28800
//#define Baud 38400
//#define Baud 57600

//�ա���һλ���趨ʱ������
#define rs_FULL_BIT0 ((Fosc / 12) / Baud)
#define rs_FULL_BIT (65536 - rs_FULL_BIT0)
#define rs_FULL_BIT_H rs_FULL_BIT >> 8		 //�ա���һλ���趨ʱ��������λ
#define rs_FULL_BIT_L (rs_FULL_BIT & 0x00FF) //�ա���һλ���趨ʱ��������λ

//�����ʼλ��ʱ�������趨ʱ������
#define rs_TEST0 rs_FULL_BIT0 / 4 //�����ʽϵ�ʱ���Գ��� 3 ����� 2
#define rs_TEST ((~rs_TEST0))
#define rs_TEST_H rs_TEST >> 8	 //��λ
#define rs_TEST_L rs_TEST & 0x00FF //��λ

//������ʼλ���趨ʱ���ܼ���
#define rs_START_BIT 0xFFFF - (Fosc / 12 / Baud) + 0x28
#define rs_START_BIT_H rs_START_BIT >> 8	 //������ʼλ���趨ʱ��������λ
#define rs_START_BIT_L rs_START_BIT & 0x00FF //������ʼλ���趨ʱ��������λ

#define rs_RECEIVE_MAX 128						 //�����ճ���
unsigned char idata rs232buffer[rs_RECEIVE_MAX]; //�ա���������
unsigned int ReceivePoint;						 //�������ݴ洢ָ��

#ifdef TIMER_0
void timer0(void) interrupt 1 using 3
{
	if (rs_RXD == 0 | rs_shift_count > 0)
	{
		soft_rs232_interrupt();
	}
	else
	{
		rs_timerH = rs_TEST_H;
		rs_timerL = rs_TEST_L;
	}
}
#endif

#ifdef TIMER_1
void timer1(void) interrupt 3 using 3
{
	if (rs_RXD == 0 | rs_shift_count > 0)
	{
		soft_rs232_interrupt();
	}
	else
	{
		rs_timerH = rs_TEST_H;
		rs_timerL = rs_TEST_L;
	}
}
#endif
/***************************************/

void soft_rs232_init(void) //���ڳ�ʼ��
{
	TCON_ENABLE_TIMER = 0; //ֹͣ��ʱ��
	TMOD &= TMOD_AND_WORD;
	TMOD |= TMOD_TIME_MODE;
	rs_RXD = 1; //���ս��óɸߵ�ƽ
	rs_TXD = 1; //������óɸߵ�ƽ
	IP_PTx = 1; //���ж����ȼ�Ϊ��
	IE_ETx = 1; //����ʱ���ж�
}

void soft_receive_init() //�����ʼλ
{
	TCON_ENABLE_TIMER = 0; //ֹͣ��ʱ��
	rs_timerH = rs_TEST_H;
	rs_timerL = rs_TEST_L;
	rs_shift_count = 0;
	TCON_ENABLE_TIMER = 1; //������ʱ��
}

void soft_receive_enable() //�������
{
	rs_f_RI_enable = 1;  //�������
	rs_f_TI_enable = 0;  //��ֹ����
	soft_receive_init(); //�����ʼλ, RXD �½��ش��������ֽڹ���.
}

void soft_send_enable(void) //������
{
	TCON_ENABLE_TIMER = 0; //ֹͣ��ʱ��
	rs_f_TI_enable = 1;	//������
	rs_f_RI_enable = 0;	//��ֹ����

	rs_shift_count = 0;	//����λ������
	rs_f_TI = 1;		   //����һ���ַ���ϱ�־
	TCON_ENABLE_TIMER = 1; //������ʱ��
}

void soft_rs232_interrupt(void)
{
	/************************ ���� ****************************/
	if (rs_f_RI_enable == 1)
	{
		if (rs_shift_count == 0) //��λ������==0, ��ʾ��⵽��ʼλ�����
		{
			if (rs_RXD == 1)
			{
				soft_receive_enable(); //��ʼλ��, ���¿�ʼ
			}
			else
			{
				//�´��ж�������λ��ֹͣλ�е�ĳʱ�̷���
				rs_timerL += rs_FULL_BIT_L + 0x10;
				rs_timerH = rs_FULL_BIT_H;
				rs_shift_count++;
				rs_BUF = 0; //����λ�������
			}
		}
		else
		{
			rs_timerL += rs_FULL_BIT_L; //�´��ж�������λ��ֹͣλ�з���
			rs_timerH = rs_FULL_BIT_H;

			rs_shift_count++; //2--9:����λ 10:ֹͣλ

			if (rs_shift_count == 9)
			{
				rs_BUF = rs_BUF >> 1; //���յ�8λ
				rs_BUF_bit7 = rs_RXD;
				if (ReceivePoint < rs_RECEIVE_MAX)
				{ //�����յ����ֽ�
					rs232buffer[ReceivePoint++] = rs_BUF;
				}
				else
				{
					rs_f_RI_enable = 0; //��������, ��ֹ����
				}
			}
			else
			{
				if (rs_shift_count < 9) //�յ���������λ 1 -- 7
				{
					rs_BUF = rs_BUF >> 1;
					rs_BUF_bit7 = rs_RXD;
				}
				else
				{ //�յ�ֹͣλ��������� PC ����������һ����ʼλ
					soft_receive_init();
				}
			}
		}
		TCON_TFx = 0; //�嶨ʱ���жϱ�־
	}
	else
	{
		/************************ ���� ****************************/
		if (rs_f_TI_enable == 1)
		{
			rs_timerL += rs_FULL_BIT_L; //�´��ж�������λ��ĩβʱ��
			rs_timerH = rs_FULL_BIT_H;

			rs_shift_count--; //0:ֹͣλĩβʱ�̵�
			//1:����ֹͣλ
			//2--9:��������λ
			if (rs_shift_count > 9) //����״̬
			{
				rs_shift_count = 9;
				rs_BUF = 0xFF;
			}

			if (rs_shift_count > 1) //2--9:��������λ
			{
				ACC = rs_BUF;
				ACC = ACC >> 1;
				rs_TXD = CY;
				rs_BUF = ACC;
			}
			else
			{
				if (rs_shift_count == 0) //0:ֹͣλĩβʱ�̵�
				{
					rs_TXD = 1;
					rs_f_TI = 1; //�ѷ������һ���ֽ�
				}
				else
				{
					rs_TXD = 1; //1:����ֹͣλ
				}
			}
		}
	}
}

//����ת����ʱ��Ҫ�ȵ��� soft_send_enable ()
void rs_send_byte(unsigned char SendByte) //����һ���ֽ�
{
	while (rs_f_TI == 0)
		; //�ȴ��������ǰһ���ֽ�
	rs_TXD = 1;
	rs_timerL = rs_START_BIT_L; //�´��ж�����ʼλ��ĩβʱ��
	rs_timerH = rs_START_BIT_H;
	rs_BUF = SendByte;
	rs_shift_count = 10;
	rs_TXD = 0;  //������ʼλ
	rs_f_TI = 0; //���ѷ������һ���ֽڵı�־
}

void PrintCom(unsigned char *DAT, unsigned char len)
{
	unsigned char i;
	for (i = 0; i < len; i++)
	{
		rs_send_byte(*DAT++);
	}
}

void SYN_FrameInfo(unsigned char Music, unsigned char *HZdata)
{
	/****************��Ҫ���͵��ı�**********************************/
	unsigned char Frame_Info[50];
	unsigned char HZ_Length;
	unsigned char ecc = 0; //����У���ֽ�
	unsigned int i = 0;
	HZ_Length = strlen(HZdata); //��Ҫ�����ı��ĳ���

	/*****************֡�̶�������Ϣ**************************************/
	Frame_Info[0] = 0xFD;			   //����֡ͷFD
	Frame_Info[1] = 0x00;			   //�������������ȵĸ��ֽ�
	Frame_Info[2] = HZ_Length + 3;	 //�������������ȵĵ��ֽ�
	Frame_Info[3] = 0x01;			   //���������֣��ϳɲ�������
	Frame_Info[4] = 0x01 | Music << 4; //����������������������趨

	/*******************У�������***************************************/
	for (i = 0; i < 5; i++) //���η��͹���õ�5��֡ͷ�ֽ�
	{
		ecc = ecc ^ (Frame_Info[i]); //�Է��͵��ֽڽ������У��
	}

	for (i = 0; i < HZ_Length; i++) //���η��ʹ��ϳɵ��ı�����
	{
		ecc = ecc ^ (HZdata[i]); //�Է��͵��ֽڽ������У��
	}
	/*******************����֡��Ϣ***************************************/
	memcpy(&Frame_Info[5], HZdata, HZ_Length);
	Frame_Info[5 + HZ_Length] = ecc;
	PrintCom(Frame_Info, 5 + HZ_Length + 1);
}
