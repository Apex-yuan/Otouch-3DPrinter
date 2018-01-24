#include "usart.h"


//��ʼ��IO ����1 
//bound:������
void uart1_init(u32 bound)
{
    //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	#if EN_USART1_RX		  //���ʹ���˽���  
	NVIC_InitTypeDef NVIC_InitStructure;
	#endif
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);		//ʹ��USART1��GPIOAʱ��
 	USART_DeInit(USART1);  //��λ����1
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10

   //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������

#if EN_USART1_RX		  //���ʹ���˽���  
   //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2 ;//��ռ���ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��NVIC�Ĵ���
   
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
#endif
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 
}

ring_buffer rx_buffer  =  { { 0 }, 0, 0 };

void store_char(unsigned char c)  //�����յ������ݴ��뻺����
{
  int i = (unsigned int)(rx_buffer.head + 1) % RX_BUFFER_SIZE;
  // if we should be storing the received character into the location  //�������Ӧ�ô洢�Ľ��յ����ַ���λ�øպ�
  // just before the tail (meaning that the head would advance to the  //��β�˵�ǰ�棨��ζ��ͷ����Ҫ����β�˵�
  // current location of the tail), we're about to overflow the buffer //��ǰλ�ã�����������������������������
  // and so we don't write the character or advance the head.          //���ô�������ַ���ʹ���ͷǰ��
  if (i != rx_buffer.tail)  //������û�д��� 
	{
    rx_buffer.buffer[rx_buffer.head] = c;
    rx_buffer.head = i;
  }
}

void checkRx(void)  //�����յ�������
{ 
	unsigned char c;
	unsigned int i;
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) 
	{ 
		c = USART_ReceiveData(USART1);
		i = (unsigned int)(rx_buffer.head + 1) % RX_BUFFER_SIZE;
		// if we should be storing the received character into the location
		// just before the tail (meaning that the head would advance to the
		// current location of the tail), we're about to overflow the buffer
		// and so we don't write the character or advance the head.
		if (i != rx_buffer.tail) 
		{
			rx_buffer.buffer[rx_buffer.head] = c;
			rx_buffer.head = i;
		}
	}
}

unsigned int MYSERIAL_available(void)  //���ش��ڻ����������ݵĸ���
{
   return (unsigned int)(RX_BUFFER_SIZE + rx_buffer.head - rx_buffer.tail) % RX_BUFFER_SIZE;
}

uint8_t MYSERIAL_read(void)  //������˳�������ȡ������������
{ 
	uint8_t c;
  // if the head isn't ahead of the tail, we don't have any characters //���ͷ������β��ǰ�棬���ղ����κ��ַ�
  if (rx_buffer.head == rx_buffer.tail) 
	{
    return 0;
  } 
	else 
	{
    c = rx_buffer.buffer[rx_buffer.tail];
    rx_buffer.tail = (unsigned int)(rx_buffer.tail + 1) % RX_BUFFER_SIZE;
    return c;
  }
}
void MYSERIAL_flush(void)  //��մ�������
{
  // RX
	// don't reverse this or there may be problems if the RX interrupt  //��Ҫ�ߵ����������ܻ���һЩ���⣬��������ж�
  // occurs after reading the value of rx_buffer_head but before writing  //�����ڶ�ȡrx_buffer_head֮����д��rx_buffer_tail֮ǰ
  // the value to rx_buffer_tail; the previous value of rx_buffer_head  //֮ǰ��rx_buffer_headֵ���ܱ�д��rx_buffer_tail
  // may be written to rx_buffer_tail, making it appear as if the buffer  //ʹ�����ֻ����������Ķ��ǿյ�״̬
  // were full, not empty.
  rx_buffer.head = rx_buffer.tail;
}		

//ѡ��ͬ�ķ�ʽ֧��printf������Ҫ����stdio.hͷ�ļ�

#if 1   //����Ҫѡ��ʹ��MicroLIB	���Ϳ�֧��printf����

#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
    USART_SendData(USART1,(uint8_t)ch);   
	return ch;
}

#elif 0   //ʹ��microLib�ķ���֧��printf����
  
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}

#endif  

#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
//u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
void USART1_IRQHandler(void)                	//����1�жϷ������
{
	uint8_t rec;
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d //0x0a��β)
	{
		rec = USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
		store_char(rec);  		//����ȡ�������ݴ��뻺���� 
  } 
} 

#endif

