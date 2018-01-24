#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include "stdio.h"  //Ҫ�õ�printf���������Ҫ����C���Եı�׼��������ļ�



#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
#define RX_BUFFER_SIZE 128  //������ջ������Ĵ�С

typedef struct ring_buffer
{
  unsigned char buffer[RX_BUFFER_SIZE];
  int head;
  int tail;
}ring_buffer;

extern  ring_buffer rx_buffer;	 
extern uint16_t USART_RX_STA;         		//����״̬���	


void uart1_init(u32 bound);
void checkRx(void);
unsigned int MYSERIAL_available(void);
uint8_t MYSERIAL_read(void);
void MYSERIAL_flush(void);


#endif //__USART_H
