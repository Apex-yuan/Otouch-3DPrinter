/**
  *******************  ***********************************************************
  * @file    lcd12864.c
  * @author  xiaoyuan
  * @version V1.0
  * @date    2016-04-16
  * @brief   This file provides all the lcd12864 functions.
  ******************************************************************************
  * @attention
  * �ô����Ǵ�xs128��������ֲ�����ġ�
	* ����ģʽ
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lcd12864.h" 
#include "delay.h"

unsigned char adress_table[]=                 //����Һ�����������
{ 
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,      //��һ�к���λ�� 
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,      //�ڶ��к���λ�� 
0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,      //�����к���λ�� 
0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F       //�����к���λ�� 
};


/*************************************************************/
/*                      ��ʼ��Һ���ӿ�                       */
/*************************************************************/
void LCD12864_Init(void) 
{
  GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);  //ʹ��PAʱ��
	
	GPIO_InitStructure.GPIO_Pin = PIN_CS|PIN_SID|PIN_CLK|PIN_PSB|PIN_RST;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //�������
	GPIO_Init(GPIOE,&GPIO_InitStructure);             //�������趨��ʼ��GPIOA�˿�
	RESET_PSB;	   //PSB���Ͷ���Ϊ��������ģʽ

}

/*************************************************************/
/*                   дһ���ֽڵ�����                     */
/*************************************************************/
void LCD12864_WriteByte(unsigned char byte) 
{ 
unsigned char j; 
for(j=0;j<8;j++)        
{ 
	if((byte<<j)&0x80)
		SET_SID; 
	else 
		RESET_SID; 
	SET_SCLK; 
	RESET_SCLK; 
} 
} 

/*************************************************************/
/*                     ��Һ����������                        */
/*************************************************************/
void LCD12864_WriteData(uint8_t data) 
{
  SET_CS; 
  RESET_SCLK; 
  LCD12864_WriteByte(LCD12864_MODE_DTAE); 
  LCD12864_WriteByte(data&0xF0);          //д����λ���� 
  LCD12864_WriteByte(0xf0&(data<<4));     //д����λ���� 
  RESET_CS; 
}

/*************************************************************/
/*                      ��Һ����������                       */
/*************************************************************/
void LCD12864_WriteCmd(uint8_t cmd) 
{
  SET_CS; 
  RESET_SCLK; 
  LCD12864_WriteByte(LCD12864_MODE_CMD); 
  LCD12864_WriteByte(cmd&0xF0);        //д����λ���� 
  LCD12864_WriteByte(0xf0&(cmd<<4));        //д����λ���� 
  RESET_CS;   
}

/***************************************************************************/
/*                            �����ӳ���                                   */
/***************************************************************************/
void LCD12864_Clear(void)
{
    LCD12864_WriteCmd(0x30);//0011,0000 �������ã�һ����8λ���ݣ�����ָ� 
    delay_us(80);       //��ʱ80us
    LCD12864_WriteCmd(0x03);//AC��0,���ı�DDRAM���� 
    delay_ms(5);        //��ʱ5ms
    LCD12864_WriteCmd(0x01);//0000,0001 ��DDRAM 
    delay_ms(5);        //��ʱ5ms
    LCD12864_WriteCmd(0x06);//д��ʱ,�α����ƶ� 
    delay_us(80);       //��ʱ80us
    LCD12864_WriteCmd(0x0C);//0000,1100  ������ʾ���α�off���α�λ��off
    delay_us(80);       //��ʱ80us
}

/***************************************************************************/
/*                           �����ͼ�洢��                               */
/***************************************************************************/
void LCD12864_ClearGraphMemory(void)//�ڷ���֮ǰ�����ͼ�洢��,����ͼ�洢���Ĳ���ȫ��Ϊ������0x00.
{
	unsigned char i,j;
	LCD12864_WriteCmd(0x36);//lcm_w_test(0,0x36); //ͼ�η�ʽ
	delay_us(80);
	for(i=0;i<32;i++)
	{
		LCD12864_WriteCmd(0x80+i);//lcm_w_test(0,0x80+i);
		delay_us(20);
		LCD12864_WriteCmd(0x80);//lcm_w_test(0,0x80);
		delay_us(20);
		for(j=0;j<16;j++) 
		{
		    LCD12864_WriteData(0x00);//lcm_w_test(1,0x00);
			delay_us(20);
		}
	}
	for(i=0;i<32;i++)
	{
		LCD12864_WriteCmd(0x80+i);//lcm_w_test(0,0x80+i);
		delay_us(20);
		LCD12864_WriteCmd(0x88);//lcm_w_test(0,0x88);
		delay_us(20);
		for(j=0;j<16;j++) 
		{
			LCD12864_WriteData(0x00);//lcm_w_test(1,0x00);
			delay_us(20);
		}
	}
}
/***************************************************************************/
/*                           ������ʾ                               */
/***************************************************************************/
void LCD12864_HightlightShow (unsigned char CX, unsigned char CY, unsigned char width,unsigned char f) //CX��0-3��������ʾ���У�CY��0-7����ʼ������ʾ���У�width��0-16��������ʾ�Ŀ��
{
	unsigned char halfLineCnt, basicBlock,lcdPosX,lcdPosY;
	/*������ʾ֮ǰ�����ͼ�δ洢����*/
	if(f==1)
	LCD12864_ClearGraphMemory(); //���ͼ��
	lcdPosY = 0x80;
	if (CX == 0)
	{
		CX = 0x80;
		halfLineCnt = 16;
	}
	else if (CX == 1)
	{
		CX = 0x80;
		halfLineCnt = 32;
	}
	else if (CX == 2)
	{
		CX = 0x88;
		halfLineCnt = 16;
	}
	else if (CX == 3)
	{
		CX = 0x88;
		halfLineCnt = 32;
	}
	lcdPosX = CX + CY;

	for (; halfLineCnt != 0; halfLineCnt--)
	{
		basicBlock = width;
		LCD12864_WriteCmd(0x34);//write_com(0x34);
		delay_us(20);
		LCD12864_WriteCmd(lcdPosY);
		delay_us(20);
		LCD12864_WriteCmd(lcdPosX);
		delay_us(20);
		LCD12864_WriteCmd(0x30);
		delay_us(20);

		for (;basicBlock != 0; basicBlock--)
		{
			if (halfLineCnt > 16)
			{
				LCD12864_WriteData(0x00);
				delay_us(20);
			}
			else
			{
				LCD12864_WriteData(0xff);
//				if(YN==1) 
//					LCD12864_WriteData(0xff); //����
//				else 
//					LCD12864_WriteData(0x00); //�������
			}
			delay_us(20);
		}
		lcdPosY++;
	}
	LCD12864_WriteCmd(0x36);
	delay_us(80);
	LCD12864_WriteCmd(0x30);
	delay_us(80);
}


/***************************************************************************/
/*                           ��LCD12864�����ַ���                               */
/***************************************************************************/
void LCD12864_ShowString(uint8_t row,uint8_t col,uint8_t *data1)   //rowΪд���������ڵ�����,colΪд���������ڵ�������*data1Ϊд�������
{
  for(;row<4&&(*data1)!=0;row++)
  { 
      for(;col<8&&(*data1)!=0;col++)
      { 
          LCD12864_WriteCmd(adress_table[row*8+col]);
          delay_us(80);       //��ʱ80us
           
          LCD12864_WriteData(*data1++); 
          delay_us(80);       //��ʱ80us
          LCD12864_WriteData(*data1++); 
          delay_us(80);       //��ʱ80us
      } 
      col=0; 
  }
}
/***************************************************************************/
/*                           ��LCD12864��������                               */
/***************************************************************************/
void LCD12864_ShowNum(uint8_t row,uint8_t col,uint16_t num)  //��ʾ���ͱ����ĺ���,�����ʾ16λ��������ֻ����ʾ������
{
	uint8_t temp[17];
	uint8_t str[17];
	int i=0,j=0;
	while(num != 0)	  //���ﲻ����num%10 != 0�����num��10����������
	                  //���磬100�������ͻ���������Ͳ��ܽ���ѭ���塣
	{
		temp[i] = (num%10)+0x30;
		num/=10;
		i++;
	}
	i--;           //��Ϊi���˳�ѭ��֮ǰ���Լ���һ�Σ���ʱ��
	                //ָ�����һ���洢����ֵ��Ԫ�صĺ�һ��λ�á�
	while(i != -1)	 //��Ϊi=0ʱ��temp[0]��������ֵ��
	{
		str[j] = temp[i];
		j++;
		i--;	
	}
	str[j]='\0';  //��Ϊi���˳�ѭ��֮ǰ���Լ���һ�Σ���ʱ��
	              //ָ�����һ���洢����ֵ��Ԫ�صĺ�һ��λ�á�
	LCD12864_ShowString(row,col,str);	
}
/***************************************************************************/
/*                           ��LCD12864����С��                              */
/***************************************************************************/
void LCD12864_ShowFloat(uint8_t row,uint8_t col,float fnum) //��ʾ��4λС���ĸ���������λ��������16λ��
{
	long int num = fnum*10000;
	u8 temp[17];
	u8 str[17];
	int i=0,j=0;
	while(num != 0)	                  
	{
		temp[i] = (num%10)+0x30;
		num/=10;
		i++;
		if(i == 4)	 //4λС��������󣬼���С���㡣
		{
			temp[i] = '.';
			i++;
		}
	}
	i--;   
	while(i != -1)	
	{
		str[j] = temp[i];
		j++;
		i--;	
	}
	str[j]='\0';  
	LCD12864_ShowString(row,col,str);				
}












