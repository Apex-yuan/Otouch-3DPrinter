/**
  ******************************************************************************
  * @file    lcd12864_menu.h
  * @author  xiaoyuan
  * @version V1.0
  * @date    2016-04-12
  * @brief   
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD12864_MENU_H
#define __LCD12864_MENU_H	

#include "stm32f10x.h"

#include "exfuns.h"
#include "string.h"	
#include "led.h"

#define MENU_SIZE 6

//void (*Operation_Function)(void);

//MenuTypeDef fun_index;

//�궨��˵���ID��
#define STATUS_MENU_ID 10
#define CARD_MENU_ID 11


typedef struct menu//����һ���˵�
{
	uint8_t ID;
	u8 range_from,range_to; //��ǰ��ʾ���ʼ���������
	u8 itemCount;//��Ŀ����
	u8 selected;//��ǰѡ����
	u8 title[16]; //�˵�����
	u8 *menuItems[30];//�˵���Ŀ //ȡ���в˵���Ŀ��Ŀ�����ֵ
	struct menu **subMenus;//�Ӳ˵�
	struct menu *parent;//�ϼ��˵� ,����Ƕ�����Ϊnull
	void (**func)();//ѡ����Ӧ�ȷ������ִ�еĺ���
  void (*displayUpdate_f)(); //���ڸ��²˵���ʾ���������������ĸ��º��Զ��ĸ��£�	
}MenuTypeDef;

extern MenuTypeDef *CurrentMenu;
extern MenuTypeDef StatusMenu;
extern MenuTypeDef PrintingFinishedMenu;
extern MenuTypeDef YesOrNoMenu;

extern u16 totgconum; 		//Gcode�ļ�����
extern uint8_t **zzz;
extern char consumingTime[30];
extern char printingFilename[30];
extern uint8_t lcdDisplayUpdate;

void welcome_screen(void);
void lcd_productInfo(void);

void lcd_cardPrinting(void);
void lcd_printingPauseOrContinue(void);
void lcd_stopPrinting(void);
void lcd_cardInsertOrNot(void);
void lcd_noCard(void);

void lcd_menuInit(void);
void lcd_update(void);

void lcd_poweroff_recoverPrintingOrNot(void);
void lcd_poweroff_stopPrinting(void);
void lcd_poweroff_recoverPrinting(void);

void lcd_displayUpdate_mainMenu(void);
void lcd_displayUpdate_cardMenu(void);
void lcd_displayUpdate_statusMenu(void);
void lcd_displayUpdate_printingFinishedMenu(void);
void lcd_displayUpdate_YesOrNotMenu(void);
void lcd_displayUpdate_general(void);
void lcd_displayUpdate_MoveAxisMenu(void);
void lcd_displayUpdate_adjustParameterMenu(void);
void lcd_changePrintingSpeed(void);
void lcd_changePrintingTemp(void);	
void lcd_changePrintingFilament(void);
void lcd_displayUpdate_changeParameterMenu_speed(void);
void lcd_displayUpdate_changeParameterMenu_temp(void);

uint16_t card_getFileNum(uint8_t *path);
void card_readFileListInfo(void);

#endif
