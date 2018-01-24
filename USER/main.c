/*******************************************************************************************************
* File Name:              main.c
* Copyright:              OTouch->TopWill��All Rights Reserved.
* Author/Corporation:     ��Ԫ
* Create date:            2016-09-01
* Version:                V1.5
* Abstract Description:   ������   
*------------------------Revision History----------------------------------------------------------------
*  NO    Version      Date       Revised By    Description
*  1      V1.5      2016/9/6      xiaoyuan     
*********************************************************************************************************/

#include "marlin_main.h"
#include "include_conf.h"


/********************************************************************************************************
* Function name:           main
* Author/Corporation:      ��Ԫ
* Create date:             2016/12/08
* input parameters:        none                      
* output parameters:       none                
* parameters Description:  none                           
* Return value:            none   
* Abstract Description:    ������
*------------------------Revision History----------------------------------------------------------------
*  NO    Version      Date       Revised By    Description
*  1      V1.5      2016/12/8      xiaoyuan     �淶������
*********************************************************************************************************/
int main(void)
{	
     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	 //*****************************��ʼ���ں�ϵͳ����************************//
     delay_init();			     //��ʱ��ʼ��
 	 uart1_init(BAUDRATE); 	 //����1��ʼ��  
     TIM3_Int_Init(9,7199);  //���ڷ���ϵͳ����ʱ�䣨1ms��ʱ�жϣ�
	 TIM2_Int_Init(99,7199); //���ڰ����Ķ�ʱ��⣨10ms��ʱ�жϣ�
     EXTIX_Init();           //���ڰ�����⣬����û�õ�
//	 Adc_Init();			  	   //ADC��ʼ��,�ڲ��¶ȴ����� 
	 //****************************��ʼ���ײ�Ӳ������**************************//
 	 LED_Init();	    	//LED��ʼ��
	BEEP_Init();
 	 KEY_Init();				//������ʼ�� 
	 AT24CXX_Init();    //AT24CXX��ʼ�� 
	 SPI_Flash_Init();  //W25Qxx��ʼ��
	 LCD12864_Init();   //ST7920 lcd12864�ײ������ĳ�ʼ��
	 //*****************************��ʼ�����������	****************************// 										  
   mem_init();				      //�ڴ�س�ʼ��
   exfuns_init();		        //Ϊfatfs��ر��������ڴ�
   f_mount(fs[0],"0:",1); 	//����SD��  
//   f_mount(fs[1],"1:",1); 	//���ع���FLASH.	
	 lcd_menuInit();          //��Ļ�˵���ʼ��
   //***************************���ϵͳ��ʼ��*********************************//
 	setup(); //ִ�д�ӡ��������ĳ�ʼ������
	
	while(1) //�Ǵ�ӡ״̬�µ���ѭ��
	{		
		loop(); //��ӡ����ѭ����
	}
}
