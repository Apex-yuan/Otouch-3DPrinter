#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//�ⲿ�ж� ��������			   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/12/01  
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved	  
////////////////////////////////////////////////////////////////////////////////// 	  

uint8_t pressedkey;
extern bool poweroff_flag;
 
//�ⲿ�жϳ�ʼ������
void EXTIX_Init(void)
{
 
 	  EXTI_InitTypeDef EXTI_InitStructure;
 	  NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;
	
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//�ⲿ�жϣ���Ҫʹ��AFIOʱ��

//	  KEY_Init();//��ʼ��������Ӧioģʽ
	 
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //ʹ��PC,PD�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 //KEY�˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //���ó���������
 GPIO_Init(GPIOD, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOC.0

    //GPIOC.5 �ж����Լ��жϳ�ʼ������
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource0);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line0;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½��ش���
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

//    //GPIOA.15	  �ж����Լ��жϳ�ʼ������
//  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource8);

//  	EXTI_InitStructure.EXTI_Line=EXTI_Line15;
//  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  	EXTI_Init(&EXTI_InitStructure);	  	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

//    //GPIOA.0	  �ж����Լ��жϳ�ʼ������
//  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource9);

//   	EXTI_InitStructure.EXTI_Line=EXTI_Line9;
//  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  	EXTI_Init(&EXTI_InitStructure);		//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
//		
//		//GPIOA.0	  �ж����Լ��жϳ�ʼ������
//  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource10);

//   	EXTI_InitStructure.EXTI_Line=EXTI_Line10;
//  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  	EXTI_Init(&EXTI_InitStructure);		//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
//		
//		//GPIOA.0	  �ж����Լ��жϳ�ʼ������
//  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource11);

//   	EXTI_InitStructure.EXTI_Line=EXTI_Line11;
//  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//  	EXTI_Init(&EXTI_InitStructure);		//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���


 
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//�����ȼ�1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure);  	  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
		
//		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
//  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
//  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//�����ȼ�1
//  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
//  	NVIC_Init(&NVIC_InitStructure); 
// 
// 
//   	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
//  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
//  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//�����ȼ�1
//  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
//  	NVIC_Init(&NVIC_InitStructure); 
 
}

 
void EXTI0_IRQHandler(void)
{
  delay_ms(10);    //����
	if(PDin(0)==0)
	{	  
		poweroff_flag = 1;
//		LED0=!LED0;
//		LED1=!LED1;	
	}
	EXTI_ClearITPendingBit(EXTI_Line0);  //���EXTI0��·����λ
}
// void EXTI9_5_IRQHandler(void)
//{			
//	delay_ms(20);
//	if(KEY_D == 0)
//	{
//		pressedkey = 2;
//	}
//	else if(KEY_R == 0)
//  {
//	  pressedkey = 4;
//  }	
//	else if(KEY_M == 0)
//	{
//		pressedkey = 5;
//	}
//	else
//	{
//		pressedkey =0;
//	}
//	//pressedkey = KEY_Scan(0);
////	delay_ms(10);   //����			 
////	if(KEY0==0)	{
////		LED0=!LED0;
////		//system_task_return = 1;
////		//mui_init();
////	}
//	//else system_task_return = 0;
// 	 EXTI_ClearITPendingBit(EXTI_Line7);    //���LINE5�ϵ��жϱ�־λ
//   EXTI_ClearITPendingBit(EXTI_Line8);    //���LINE5�ϵ��жϱ�־λ 
//   EXTI_ClearITPendingBit(EXTI_Line9);    //���LINE5�ϵ��жϱ�־λ   
//}


//void EXTI15_10_IRQHandler(void)
//{
//	delay_ms(20);
//	if(KEY_U==0)
//	{
//		pressedkey = 1;
//	}
//	else if(KEY_L == 0)
//	{
//		pressedkey = 3;
//	}
//	else
//	{
//		pressedkey = 0;
//	}
//		
//	
//	//pressedkey = KEY_Scan(0);
////  delay_ms(10);    //����			 
////  if(KEY1==0)	{
////		LED1=!LED1;
////		
////	}
//	 EXTI_ClearITPendingBit(EXTI_Line10);  //���LINE15��·����λ
//	 EXTI_ClearITPendingBit(EXTI_Line11);    //���LINE5�ϵ��жϱ�־λ 
//}
