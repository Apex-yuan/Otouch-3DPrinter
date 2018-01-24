/**
  ******************************************************************************
  * @file    lcd12864_menu.c
  * @author  xiaoyuan
  * @version V1.0
  * @date    2016-04-16
  * @brief   This file provides all the lcd12864_menu functions.
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lcd12864_menu.h"
#include "lcd12864.h" 
#include "key.h"
#include "delay.h"
#include "timer.h"
#include "malloc.h"
#include "cardreader.h"
#include "marlin_main.h"
#include "exti.h"
#include "flash.h"

#define NULL 0

//FIL fgcode; //���ڲ��Զ�ȡ�ļ����ݵ��ļ����
//char buf[100];//���ڲ��Զ�ȡ�ļ����ݵĽ�������
//UINT brr; //���ڲ��Զ�ȡ�ļ�������Ҫ�ı���

uint8_t **zzz; //�洢�ļ����б�������malloc������������ռ�ſ���ʹ��
u16 totgconum; 		//Gcode�ļ�����
u16 curindex;		     //Gcode�ļ���ǰ����
uint8_t lcdDisplayUpdate = 1; //��Ļ��ʾ���±�־λ
static uint32_t nextUpdateMillis = 0; //������¼�´θ�����Ļ��ʱ��
char temperature0[30],temperatureBed[30]; 
char percentDone[30];
char consumingTime[30];
char printingFilename[30];
char cardFileList[100][30];

uint16_t temp_feedMultiply = 100; //�����ٶȵı���
uint16_t temp_e0TargetTemp;
uint8_t differenceValue = 5;

static float manual_feedrate[4] = {50*60,50*60,4*60,60};

MenuTypeDef * CurrentMenu; //ָ��ǰ�˵��Ľṹ��ָ��


/**********************һ���˵�*********************/
MenuTypeDef MainMenu = 
{
 0,0,2,3,0,
	"   3D Printer   ",
	{
		"Prepare         ",
		"Control         ",
		"Card Menu       "
	}
};

/**********************�����˵�*********************/
MenuTypeDef PrepareMenu = 
{
	0,0,2,7,0,
	"    Prepare     ",
	{

		"Printer Status��", //0
		"Preheat PLA     ", //1
		"Preheat ABS     ", //2
		"AutoHome        ", //3
		"Disable Stepper ", //4
		"Move Axis     ��", //5
		"Back MainMenu ��", //6
	}
};

MenuTypeDef ControlMenu = 
{
	0,0,2,6,0,
	"    Control     ",
	{
		"Temperature   ��", //0
		"Motion        ��", //1
		"Store Memory    ", //2
		"Load Memory     ", //3
		"Restore Fsafe   ", //4
		"Back MainMenu ��", //5
	}
};

MenuTypeDef CardMenu = 
{
	CARD_MENU_ID,0,2,100,0,
	" Card File List ",

};
/**********************�����˵�*********************/
//MenuTypeDef MoveDistanceMenu = 
//{
//	0,0,3,4,0,
//	"MoveDistanceMenu",
//	{
//		"goMoveAxisMenu",
//		"Move  0.1mm   ",
//		"Move  1  mm   ",
//		"Move 10  mm   ",
//	}
//};

MenuTypeDef TemperatureMenu = 
{
	0,0,2,14,0,
	"  Temperature   ",
	{
		"ControlMenu   ",
		"Nozzle        ",
		"Autotemp      ",
		"Min           ",
		"Max           ",
		"Fact          ",
		"Bed           ",
		"Fan Speed     ",
		"PID-P         ",
		"PID-I         ",
		"PID-D         ",
		"PID-C         ",
		"PreheatPLA Set",
		"PreheatABS Set"
	}
};

MenuTypeDef MotionMenu = 
{
	0,0,2,17,0,
	"MotionMenu",
	{
		"ControlMenu   ",
		"Acc:          ",
		"VXY-Jerk      ",
		"Vmax-X        ",
		"Vmax-Y        ",
		"Vmax-Z        ",
		"Vmax-E        ",
		"Vtrav-min     ",
		"Amax-X        ",
		"Amax-Y        ",
		"Amax-Z        ",
		"Amax-E        ",
		"A-retract     ",
		"Xsteps/mm     ",
		"Ysteps/mm     ",
		"Zsteps/mm     ",
		"Esteps/mm     "
	}
};
/**********************�ļ��˵�*********************/
MenuTypeDef MoveAxisMenu = 
{
	0,0,2,5,0,
	"   Move Axis   ",
	{
		"��uint�� :  10mm",
		" X- ��Back�� X+ ",
		" Y- ��Back�� Y+ ",
		" Z- ��Back�� Z+ ",
		"E0- ��Back�� E0+",
	}
};

//�ڴ�ӡʱ�Ĳ˵�
/**********************״̬�˵�*********************/
MenuTypeDef StatusMenu = 
{
	STATUS_MENU_ID,0,3,4,0,
	"StatusMenu",
	{
    "              ",
	 	"              ",
	  "              ",
	  "              ",
	}
};
/**********************��ӡѡ��˵�*********************/
MenuTypeDef PrintingOptionsMenu = 
{
	0,0,2,7,0,
	"Printing Options",
	{
		"Back Last Menu",       
		"Pause         ",
		"Stop Printing ",
		"Adjust Param  ",
		"ChangeFilament",
		"Change Speed  ",
		"Change Temp   ",
		
		
	}
};

MenuTypeDef ChangeParameterMenu = 
{
	0,0,3,4,0,
	" ",
	{
		"                ",
		"                ",
		"                ",
		"                ",
	}
};

MenuTypeDef AdjustParameterMenu = 
{
	0,0,2,4,0,
	" AdjustParameter",
	{
		"              ",
		"              ",
		"              ",
		"              ",
	}
};

MenuTypeDef PrintingFinishedMenu = 
{
	2,0,3,4,0,
	" ",
	{
	  "                ",
		"***Print Done***",
		"                ",
	  "any key back... ",
	}
};
//ȷ����ȡ���˵�������������ѡ����ʽ������ʹ�øò˵���
//Ŀǰʹ�øò˵��Ĳ��֣���ӡֹͣ��ѡ�񣬶ϵ������ѡ��
MenuTypeDef YesOrNoMenu = 
{
	0,0,3,4,2,
	"",
	{
	  "YES OR NO ?   ",
	  "              ",
	  "YES         ",
	  "NO          ",
	}
};

//��ʾ��ӭ����
void welcome_screen(void)
{
	LCD12864_Clear();
	LCD12864_ShowString(0,0,"***************");
	LCD12864_ShowString(1,0," WECOME TO USE  ");
	LCD12864_ShowString(2,0,"STM32 3D Printer");
	LCD12864_ShowString(3,0,"***************");
	//BEEP_Ring(2500); //��ʾ5s
	delay_ms(5000); //��ʾ5s
//	LCD12864_Clear(); //����
}

//��ʾ��Ʒ��Ϣ
void lcd_productInfo(void)
{
	LCD12864_Clear();
	LCD12864_ShowString(0,0,"OTouch->TopWill ");
	LCD12864_ShowString(1,0," WECOME TO USE  ");
	LCD12864_ShowString(2,0,"STM32 3D Printer");
	LCD12864_ShowString(3,0,"author: wangyuan");
	while(!keyPressed); //�ȴ���������
}

//lcd��Ļ���º���
void lcd_update(void)
{
	lcd_cardInsertOrNot(); //���SD���Ĳ������
	CurrentMenu->displayUpdate_f(); //�����������µ���Ļ����
	
	if(nextUpdateMillis < millis()) //��Ļ��ʱ����
	{
//		FLASH_WRITE_VAR(FLASH_SET_STORE_OFFSET+114,card.sdpos);
//		FLASH_WRITE_VAR(FLASH_SET_STORE_OFFSET+118,current_position[Z_AXIS]);
//		FLASH_WRITE_VAR(FLASH_SET_STORE_OFFSET+122,printingFilename);
		if(STATUS_MENU_ID == CurrentMenu->ID || CARD_MENU_ID == CurrentMenu->ID) //״̬��Ҫ����ʱ��ˢ�µĲ˵�
		{
		  lcdDisplayUpdate = 1;
			CurrentMenu->displayUpdate_f();
		}
		//card_readFileListInfo();//��������ϸú���ֻ�ڳ�ʼ����ִ��һ�Σ�SD���е��ļ����漸������ʾ���룬ԭ���в���������ڸú�������Ҳ�����������Ӵ����ˣ�������Ƿ���ϸ΢�쳣��
	  nextUpdateMillis = millis() + 800; //ˢ�����ڣ�ÿ100msˢ��һ�Σ�
	}
}

void lcd_backMainMenu(void)
{
	//�˳��˵�ʱ������ǰ�˵���ѡ��������,������ʾ��Χ��������ʼ��״̬
	CurrentMenu->selected = 0; 
	CurrentMenu->range_from = 0; 
	CurrentMenu->range_to = 2;
	CurrentMenu = &MainMenu;
	lcdDisplayUpdate = 1;
	CurrentMenu->displayUpdate_f();
}
 
void lcd_autoHome(void)
{
	enquecommand("G28");
}
void lcd_disableStepper(void)
{
	enquecommand("M84");
}

void lcd_configResetDefault(void) //�ָ���������
{
	Config_ResetDefault();
}


//2017/3/10 �޸���ÿ�μ�ⶼ�����¶�ȡ����Ϣ��ʹ���Ӳ��ϴ��ڵ�bug
//���SD���ǲ��뻹�ǰγ�״̬
void lcd_cardInsertOrNot()
{
	uint8_t rereadsdfileinfoflag; //�ض�SD���ļ���Ϣ��־λ
	//���SD�����Ը���Card Menu �˵���Ŀ������
	if(SD_CD) //δ����SD��
	{
		rereadsdfileinfoflag = 1; //�ض�SD���ļ���Ϣ��־λ��λ
		MainMenu.subMenus[2] = NULL;	
		MainMenu.func[2] = &lcd_noCard;
    	//lcdDisplayUpdate = 1;
		if(CurrentMenu == &CardMenu) //��CardMenu�˵��°γ�SD���������˵�
		{
			CurrentMenu = &MainMenu;
			lcdDisplayUpdate = 1;
		}
	}
	if(rereadsdfileinfoflag == 1) //���޿�״̬�£�����SD��
	{
		if(!SD_CD)
		{
            //exfuns_init();
			//f_mount(fs[0],"0:",1); 	//����SD��     		
			MainMenu.subMenus[2] = &CardMenu;
			MainMenu.func[2] = NULL;
		  card_readFileListInfo();
		}
		rereadsdfileinfoflag = 0; //����
	}
}

//SD��δ����
void lcd_noCard(void)
{
	LCD12864_Clear();
  LCD12864_ShowString(0,1,"  No SD Card");
	while(SD_CD)
	{
		if(keyPressed) //�а�������
		{
			CurrentMenu = &MainMenu; //�������˵�
			return; //������ǰ����
		}
	}
	//���޿�״̬�²�����SD�����ָ���ʾSD���ļ��б�
	card_readFileListInfo();  
	CurrentMenu = &CardMenu; //������SD��
}

//����SD���ļ���ӡ����
void lcd_cardPrinting()
{
	uint8_t *fnn;
//	fnn = (u8 *)(CardMenu.menuItems[CardMenu.selected]); //��ȡ��ǰ��ӡ�ļ����ļ���
	fnn = (u8 *)(cardFileList[CardMenu.selected]); //��ȡ��ǰ��ӡ�ļ����ļ���
	strcpy((char *) printingFilename,(const char*)fnn);  //�Ƚ��ļ����洢���ַ��������У�����������״̬������ʾ�ļ���ʱ�����bug���洢��StatusMenu.menuItems[0]�е��ļ����ڴ��ڼ���״̬ʱ���ɺ����һ���ļ�֮����ļ����ļ��������в����ԭ��
	StatusMenu.menuItems[0] = (uint8_t *)printingFilename; //���ļ����洢��״̬�˵���Ŀ�У�������ʾ��ǰ��ӡ�ļ���	
	//ִ�д�ӡ���д��ڽ�һ�������Ľ�
				card_initsd();
				card_openFile((char *)fnn,true);
				card_startFileprint();
				starttime=millis();
	
//  CurrentMenu = &PrintingOptionsMenu;	
	CurrentMenu = &StatusMenu;
	lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
	CurrentMenu->displayUpdate_f();
}

//��ͣ��ӡ���̻��Ǽ�����ӡ����
void lcd_printingPauseOrContinue()
{
	if(card.sdprinting == true)
	{
		PrintingOptionsMenu.menuItems[1] = "Continue      ", 
		card.sdprinting = false;
	}
	else
	{
		PrintingOptionsMenu.menuItems[1] = "Pause         ",
		card.sdprinting = true;
		//TIM_ITConfig(TIM3,TIM_IT_Update, ENABLE); //����TIM3�ж�
	} 
	CurrentMenu = &PrintingOptionsMenu;
	lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
	if(card_eof()) //��ȡ�����ļ�ĩβ
	{
		card_printingHasFinished();
		CurrentMenu = &MainMenu;
		lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
  }
}

//�Ƿ���ֹ��ӡ����
void lcd_stopPrintingOrNot(void)
{
	YesOrNoMenu.menuItems[0] = "Stop Printing?";
	YesOrNoMenu.menuItems[1] = "              ";
	YesOrNoMenu.menuItems[2] = "YES           ";
	YesOrNoMenu.menuItems[3] = "NO            ";
	YesOrNoMenu.func[2] = &lcd_stopPrinting;
	YesOrNoMenu.subMenus[3] = &PrintingOptionsMenu; //2017.3.14ѡ��NOֱ�ӽ��Ӳ˵�ָ�򵽸��˵�����֪�����ɲ����У����ϲ���һ�£�������оͿ���ʡ�Ե�ѡ��NO�Ĺ��ܺ����� 
    CurrentMenu = &YesOrNoMenu;
	lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
	CurrentMenu->displayUpdate_f();
}

//��ֹ��ӡ����
void lcd_stopPrinting()
{
	quickStop(); 
	card_closefile();
	starttime=0;
	card.sdprinting = false;

	autotempShutdown();
	setTargetHotend(0,active_extruder);
	//heater_0_temp = 0;
	//bed_temp = 0;

	disable_x(); 
	disable_y(); 
	disable_z(); 
	disable_e0(); 
	disable_e1(); 
	enquecommand("M84") ;
	
	CurrentMenu = &MainMenu;
	lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
	CurrentMenu->displayUpdate_f();
}

//�ı��ӡ�ٶ�
void lcd_changePrintingSpeed(void)
{
	char tempStr[16];
	ChangeParameterMenu.menuItems[0] = "  Change Speed  ";
	sprintf(tempStr,"Speed:%d%%",temp_feedMultiply );
	ChangeParameterMenu.menuItems[1] = (uint8_t *)tempStr;
	ChangeParameterMenu.menuItems[2] = "                ";
	ChangeParameterMenu.menuItems[3] = "by up/down key  ";
	ChangeParameterMenu.displayUpdate_f = &lcd_displayUpdate_changeParameterMenu_speed;
	CurrentMenu = &ChangeParameterMenu;
	lcdDisplayUpdate = 1;
	CurrentMenu->displayUpdate_f();
}
//�ı��ӡ�¶�
void lcd_changePrintingTemp(void)
{
	char tempStr[16];
	ChangeParameterMenu.menuItems[0] = "  Change Temp  ";
	sprintf(tempStr,"Temp : %ddeg",temp_e0TargetTemp );
	ChangeParameterMenu.menuItems[1] = (uint8_t *)tempStr;
	ChangeParameterMenu.menuItems[2] = "                ";
	ChangeParameterMenu.menuItems[3] = "by up/down key  ";
	ChangeParameterMenu.displayUpdate_f = &lcd_displayUpdate_changeParameterMenu_temp;
	CurrentMenu = &ChangeParameterMenu;
	lcdDisplayUpdate = 1;
	CurrentMenu->displayUpdate_f();
}
////��������
//void lcd_changeParam(void)
//{
//	char temp_diff[16];
//	char temp_speed[16];
//	char temp_temp[16];
//	char temp_fanSpeed[16];
//	sprintf(temp_diff,"Diff  :  %d%%",feedMultiply );
//	ChangeParamMenu.menuItems[0] = (uint8_t *)temp_diff;
//	sprintf(temp_speed,"Speed  :  %d%%",feedMultiply );
//	ChangeParamMenu.menuItems[1] = (uint8_t *)temp_speed;
//	sprintf(temp_temp,"Temp_E0  :  %d%%",target_temperature[0] );
//	ChangeParamMenu.menuItems[2] = (uint8_t *)temp_temp;
//	sprintf(temp_fanSpeed,"FanSpeed  :  %d%%",fanSpeed );
//	ChangeParamMenu.menuItems[3] = (uint8_t *)temp_fanSpeed;
//}

//������ӡ�Ĳ�
void lcd_changePrintingFilament(void)
{	
	enquecommand("M600"); //�����Ĳĵ�G���� //ֻ�ǽ�M600�����˻���������û������ִ��

}



//�ϵ������ϵ���Ƿ�ָ���ǰ�Ĵ�ӡ����
void lcd_poweroff_recoverPrintingOrNot(void)
{
//	//��ȡ�洢�Ĳ���
	FLASH_READ_VAR(FLASH_SET_STORE_OFFSET+114,poweroff_sdpos); 
	FLASH_READ_VAR(FLASH_SET_STORE_OFFSET+118,poweroff_position_z);
	FLASH_READ_VAR(FLASH_SET_STORE_OFFSET+122,printingFilename);
//	AT24CXX_Read(60,(u8 *)&poweroff_sdpos,8);
//		AT24CXX_Read(68,(u8 *)&poweroff_position_z,8);
//		AT24CXX_Read(76,(u8 *)&printingFilename,8);
	if(poweroff_sdpos != 0 && poweroff_position_z != 0)
	{
		LCD12864_Clear();
		YesOrNoMenu.menuItems[0] = "Recover Print?  ";
		YesOrNoMenu.menuItems[1] = "                ";
		YesOrNoMenu.menuItems[2] = "YES           ";
		YesOrNoMenu.menuItems[3] = "NO            ";
		YesOrNoMenu.func[2] = &lcd_poweroff_recoverPrinting;
		YesOrNoMenu.func[3] = &lcd_poweroff_stopPrinting; 

		CurrentMenu = &YesOrNoMenu;
		lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
		CurrentMenu->displayUpdate_f();
	}
}
//�ָ��ϵ�֮ǰ�Ĵ�ӡ����
void lcd_poweroff_recoverPrinting(void) //�ϵ�����
{
	lcd_contiune_print_after_poweroff = 1;
	card_initsd();
	card_openFile((char *)printingFilename,true);
	card_startFileprint();
	StatusMenu.menuItems[0] = (uint8_t *)printingFilename; //���ļ����洢��״̬�˵���Ŀ�У�������ʾ��ǰ��ӡ�ļ���	
	CurrentMenu = &StatusMenu;
	lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
}
//��ֹ�ϵ�ǰ�Ĵ�ӡ����
void lcd_poweroff_stopPrinting(void) //��������
{
	uint8_t i;
	//���洢�Ķϵ�����������㣨���ҽ�����������д�룬���ҵ����õķ������滻��
	poweroff_sdpos = 0;
	poweroff_position_z = 0;
	for(i=0;i<30;i++)
	{
		poweroff_printing_filename[i] = 0; 
	}
	FLASH_WRITE_VAR(FLASH_SET_STORE_OFFSET+114,poweroff_sdpos); 
	FLASH_WRITE_VAR(FLASH_SET_STORE_OFFSET+118,poweroff_position_z);
	FLASH_WRITE_VAR(FLASH_SET_STORE_OFFSET+122,poweroff_printing_filename);
	CurrentMenu = &MainMenu;
	lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
}

//��Ļ�˵���ʼ��
void lcd_menuInit()
{
	uint8_t i;
	
	welcome_screen(); //��ʼ����ӭ����
	
	/*��ȡSD���ļ���Ŀ���ļ����б�*/
	card_readFileListInfo(); //������ִ�з���CardMenu�˵���ʼ����������
	/**********************һ���˵�*********************/
	MainMenu.subMenus = mymalloc(sizeof(&MainMenu)*3);
	MainMenu.subMenus[0] = &PrepareMenu;
	MainMenu.subMenus[1] = &ControlMenu;
	MainMenu.subMenus[2] = &CardMenu;
	MainMenu.parent = NULL;
	MainMenu.func = mymalloc(sizeof(NULL)*3);
	MainMenu.func[0] = NULL;//&lcd_productInfo;
	MainMenu.func[1] = NULL;
	MainMenu.func[2] = NULL;
	MainMenu.displayUpdate_f = &lcd_displayUpdate_general;
	/**********************�����˵�*********************/
	PrepareMenu.subMenus = mymalloc(sizeof(&PrepareMenu)*7);
	PrepareMenu.subMenus[0] = 
	PrepareMenu.subMenus[1] =
	PrepareMenu.subMenus[2] =
	PrepareMenu.subMenus[3] =
	PrepareMenu.subMenus[4] = NULL;
	PrepareMenu.subMenus[5] = &MoveAxisMenu;
	PrepareMenu.subMenus[6] = NULL;//&MainMenu; //���ڷ������˵�
	PrepareMenu.parent = &MainMenu;
	PrepareMenu.func = mymalloc(sizeof(&PrepareMenu)*7);
	PrepareMenu.func[0] = 
	PrepareMenu.func[1] = 
	PrepareMenu.func[2] = NULL;
	PrepareMenu.func[3] = &lcd_autoHome;
	PrepareMenu.func[4] = &lcd_disableStepper;
	PrepareMenu.func[5] = NULL;
	PrepareMenu.func[6] = &lcd_backMainMenu;
	//PrepareMenu.displayUpdate_f = mymalloc(sizeof(&PrepareMenu));
	PrepareMenu.displayUpdate_f = &lcd_displayUpdate_general;
	
	ControlMenu.subMenus = mymalloc(sizeof(&ControlMenu)*6);
	ControlMenu.subMenus[0] = NULL;
	ControlMenu.subMenus[1] = &TemperatureMenu;
	ControlMenu.subMenus[2] = &MotionMenu;
	ControlMenu.subMenus[3] =
	ControlMenu.subMenus[4] =
	ControlMenu.subMenus[5] = NULL;
	ControlMenu.parent = &MainMenu;
	ControlMenu.func = mymalloc(sizeof(&ControlMenu)*6);
	ControlMenu.func[0] = 
	ControlMenu.func[1] =
	ControlMenu.func[2] =
	ControlMenu.func[3] = NULL;
	ControlMenu.func[4] = &lcd_configResetDefault;
	ControlMenu.func[5] = &lcd_backMainMenu;//
	//ControlMenu.displayUpdate_f = mymalloc(sizeof(&ControlMenu));
	ControlMenu.displayUpdate_f = &lcd_displayUpdate_general;
	
	CardMenu.subMenus = mymalloc(sizeof(&CardMenu)*totgconum);
	for(i=0;i<totgconum;i++)
	{
		CardMenu.subMenus[i] = NULL;
	}
	CardMenu.parent = &MainMenu;
	CardMenu.func = mymalloc(sizeof(&CardMenu)*totgconum);
	for(i=0;i<totgconum;i++)
	{
		CardMenu.func[i] = &lcd_cardPrinting;
	}
	//CardMenu.displayUpdate_f = (void (*)())mymalloc(sizeof(&CardMenu));
	CardMenu.displayUpdate_f = &lcd_displayUpdate_cardMenu;
	/**********************�����˵�*********************/
//	MoveDistanceMenu.subMenus = mymalloc(sizeof(&MoveDistanceMenu)*4);
//	MoveDistanceMenu.subMenus[0] = NULL;
//	MoveDistanceMenu.subMenus[1] = &MoveAxisMenu;
//	MoveDistanceMenu.subMenus[2] = &MoveAxisMenu;
//	MoveDistanceMenu.subMenus[3] = &MoveAxisMenu;//NULL;
//	MoveDistanceMenu.parent = &PrepareMenu;
//	MoveDistanceMenu.func = mymalloc(sizeof(NULL)*4);
//	MoveDistanceMenu.func[0] = NULL;
//	MoveDistanceMenu.func[1] = NULL;
//	MoveDistanceMenu.func[2] = NULL;
//	MoveDistanceMenu.func[3] = NULL;
//	MoveDistanceMenu.displayUpdate_f = &lcd_displayUpdate_general;
	
	TemperatureMenu.subMenus = mymalloc(sizeof(&TemperatureMenu)*9);
	TemperatureMenu.subMenus[0] = 
	TemperatureMenu.subMenus[1] =
	TemperatureMenu.subMenus[2] =
	TemperatureMenu.subMenus[3] =
	TemperatureMenu.subMenus[4] =
	TemperatureMenu.subMenus[5] =
	TemperatureMenu.subMenus[6] =
	TemperatureMenu.subMenus[7] =
	TemperatureMenu.subMenus[8] = 
	TemperatureMenu.subMenus[9] =
	TemperatureMenu.subMenus[10] =
	TemperatureMenu.subMenus[11] =
	TemperatureMenu.subMenus[12] =
	TemperatureMenu.subMenus[13] = NULL;
	TemperatureMenu.parent = &ControlMenu;
	TemperatureMenu.func = mymalloc(sizeof(&TemperatureMenu)*9);
	TemperatureMenu.func[0] = 
	TemperatureMenu.func[1] =
	TemperatureMenu.func[2] =
	TemperatureMenu.func[3] =
	TemperatureMenu.func[4] =
	TemperatureMenu.func[5] =
	TemperatureMenu.func[6] =
	TemperatureMenu.func[7] =
	TemperatureMenu.func[8] = 
	TemperatureMenu.func[9] =
	TemperatureMenu.func[10] =
	TemperatureMenu.func[11] =
	TemperatureMenu.func[12] =
	TemperatureMenu.func[13] = NULL;
	TemperatureMenu.displayUpdate_f = &lcd_displayUpdate_general;
	
	MotionMenu.subMenus = mymalloc(sizeof(&MotionMenu)*9);
	MotionMenu.subMenus[0] = 
	MotionMenu.subMenus[1] =
	MotionMenu.subMenus[2] =
	MotionMenu.subMenus[3] =
	MotionMenu.subMenus[4] =
	MotionMenu.subMenus[5] =
	MotionMenu.subMenus[6] =
	MotionMenu.subMenus[7] =
	MotionMenu.subMenus[8] = 
	MotionMenu.subMenus[9] =
	MotionMenu.subMenus[10] =
	MotionMenu.subMenus[11] =
	MotionMenu.subMenus[12] =
	MotionMenu.subMenus[13] = 
	MotionMenu.subMenus[14] =
	MotionMenu.subMenus[15] =
	MotionMenu.subMenus[16] = NULL;
	MotionMenu.parent = &ControlMenu;
	MotionMenu.func = mymalloc(sizeof(&MotionMenu)*9);
	MotionMenu.func[0] = 
	MotionMenu.func[1] =
	MotionMenu.func[2] =
	MotionMenu.func[3] =
	MotionMenu.func[4] =
	MotionMenu.func[5] =
	MotionMenu.func[6] =
	MotionMenu.func[7] =
	MotionMenu.func[8] = 
	MotionMenu.func[9] =
	MotionMenu.func[10] =
	MotionMenu.func[11] =
	MotionMenu.func[12] =
	MotionMenu.func[13] = 
	MotionMenu.func[14] =
	MotionMenu.func[15] =
	MotionMenu.func[16] = NULL;
	MotionMenu.displayUpdate_f = &lcd_displayUpdate_general;
	/**********************�ļ��˵�*********************/
	MoveAxisMenu.subMenus = mymalloc(sizeof(&MoveAxisMenu)*6);
	MoveAxisMenu.subMenus[0] = 
	MoveAxisMenu.subMenus[1] = 
	MoveAxisMenu.subMenus[2] = 
	MoveAxisMenu.subMenus[3] =
	MoveAxisMenu.subMenus[4] = 
	MoveAxisMenu.subMenus[5] = NULL;
	MoveAxisMenu.parent = &PrepareMenu;//&MoveDistanceMenu;
	MoveAxisMenu.func = mymalloc(sizeof(&MoveAxisMenu)*6);
	MoveAxisMenu.func[0] = 
	MoveAxisMenu.func[1] =
	MoveAxisMenu.func[2] =
	MoveAxisMenu.func[3] =
	MoveAxisMenu.func[4] =
	MoveAxisMenu.func[5] = NULL;
	//MoveAxisMenu.displayUpdate_f = mymalloc(sizeof(&MoveAxisMenu));
	MoveAxisMenu.displayUpdate_f = &lcd_displayUpdate_MoveAxisMenu;

//��ӡ�е���Ļ�˵�
/**********************һ���˵�*********************/
	StatusMenu.subMenus = mymalloc(sizeof(&StatusMenu)*4);
	StatusMenu.subMenus[0] = &PrintingOptionsMenu;
	StatusMenu.subMenus[1] = &PrintingOptionsMenu;
	StatusMenu.subMenus[2] = &PrintingOptionsMenu;
	StatusMenu.subMenus[3] = &PrintingOptionsMenu;
	StatusMenu.parent = NULL;
	StatusMenu.func = mymalloc(sizeof(NULL)*4);
	StatusMenu.func[0] = NULL;
	StatusMenu.func[1] = NULL;
	StatusMenu.func[2] = NULL;
	StatusMenu.func[3] = NULL;//&Card_Menu;
	StatusMenu.displayUpdate_f = &lcd_displayUpdate_statusMenu;
	
	YesOrNoMenu.subMenus = mymalloc(sizeof(&YesOrNoMenu)*4);
	YesOrNoMenu.subMenus[0] = NULL;
	YesOrNoMenu.subMenus[1] = NULL;
	YesOrNoMenu.subMenus[2] = NULL;
	YesOrNoMenu.subMenus[3] = NULL;
	YesOrNoMenu.parent = &StatusMenu;
	YesOrNoMenu.func = mymalloc(sizeof(NULL)*4);
	YesOrNoMenu.func[0] = NULL;
	YesOrNoMenu.func[1] = NULL;
	YesOrNoMenu.func[2] = NULL;//lcd_stopPrinting;
	YesOrNoMenu.func[3] = NULL;//&Card_Menu;
	YesOrNoMenu.displayUpdate_f = &lcd_displayUpdate_YesOrNotMenu;
	
	PrintingOptionsMenu.subMenus = mymalloc(sizeof(&PrintingOptionsMenu)*4);
	PrintingOptionsMenu.subMenus[0] = &StatusMenu;
	PrintingOptionsMenu.subMenus[1] = NULL;
	PrintingOptionsMenu.subMenus[2] = NULL;
	PrintingOptionsMenu.subMenus[3] = &AdjustParameterMenu;
	PrintingOptionsMenu.subMenus[4] = NULL;
	PrintingOptionsMenu.subMenus[5] = NULL;
	PrintingOptionsMenu.subMenus[6] = NULL;
	PrintingOptionsMenu.parent = &StatusMenu;
	PrintingOptionsMenu.func = mymalloc(sizeof(NULL)*4);
	PrintingOptionsMenu.func[0] = NULL;
	PrintingOptionsMenu.func[1] = &lcd_printingPauseOrContinue;
	PrintingOptionsMenu.func[2] = &lcd_stopPrintingOrNot;//lcd_stopPrinting;
	PrintingOptionsMenu.func[3] = NULL;//&Card_Menu;
	PrintingOptionsMenu.func[4] = &lcd_changePrintingFilament;
	PrintingOptionsMenu.func[5] = &lcd_changePrintingSpeed;
	PrintingOptionsMenu.func[6] = &lcd_changePrintingTemp;
	PrintingOptionsMenu.displayUpdate_f = &lcd_displayUpdate_general;
	
	ChangeParameterMenu.subMenus = mymalloc(sizeof(&ChangeParameterMenu)*4);
	ChangeParameterMenu.subMenus[0] = NULL;
	ChangeParameterMenu.subMenus[1] = NULL;
	ChangeParameterMenu.subMenus[2] = NULL;
	ChangeParameterMenu.subMenus[3] = NULL;
	ChangeParameterMenu.parent = &PrintingOptionsMenu;
	ChangeParameterMenu.func = mymalloc(sizeof(NULL)*4);
	ChangeParameterMenu.func[0] = NULL;
	ChangeParameterMenu.func[1] = NULL;
	ChangeParameterMenu.func[2] = NULL;
	ChangeParameterMenu.func[3] = NULL;
	ChangeParameterMenu.displayUpdate_f = NULL;//&lcd_displayUpdate_changePrintingSpeedMenu;	
	
	AdjustParameterMenu.subMenus = mymalloc(sizeof(&AdjustParameterMenu)*4);
	AdjustParameterMenu.subMenus[0] = NULL;
	AdjustParameterMenu.subMenus[1] = NULL;
	AdjustParameterMenu.subMenus[2] = NULL;
	AdjustParameterMenu.subMenus[3] = NULL;
	AdjustParameterMenu.parent = &PrintingOptionsMenu;
	AdjustParameterMenu.func = mymalloc(sizeof(NULL)*4);
	AdjustParameterMenu.func[0] = NULL;
	AdjustParameterMenu.func[1] = NULL;
	AdjustParameterMenu.func[2] = NULL;
	AdjustParameterMenu.func[3] = NULL;
	AdjustParameterMenu.displayUpdate_f = &lcd_displayUpdate_adjustParameterMenu;	
	
	PrintingFinishedMenu.subMenus = mymalloc(sizeof(&PrintingFinishedMenu)*4);
	PrintingFinishedMenu.subMenus[0] = NULL;
	PrintingFinishedMenu.subMenus[1] = NULL;
	PrintingFinishedMenu.subMenus[2] = NULL;
	PrintingFinishedMenu.subMenus[3] = NULL;
	PrintingFinishedMenu.parent = NULL;
	PrintingFinishedMenu.func = mymalloc(sizeof(NULL)*4);
	PrintingFinishedMenu.func[0] = NULL;
	PrintingFinishedMenu.func[1] = NULL;
	PrintingFinishedMenu.func[2] = NULL;
	PrintingFinishedMenu.func[3] = NULL;
	PrintingFinishedMenu.displayUpdate_f = &lcd_displayUpdate_printingFinishedMenu;	
	
	lcd_poweroff_recoverPrintingOrNot(); //����Ƿ���δ��ɴ�ӡ��ģ��
	//welcome_screen(); //��ʾ5s�Ļ�ӭ����
	CurrentMenu = &MainMenu; //����ǰ�˵�ָ�����˵�
	lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
  CurrentMenu->displayUpdate_f();
}

//lcd_displayUpdate_xxx �������ڸ�����ʾ�������ݰ����������º͸���ʱ����£����������ĸ��·���
//��case 1��2��3 �������ȴ�����ʱ�������Ҫ����default������ʱ���ж�if(nextUpdateMillis < millis())��
//�����Ҫ���¾ͽ���Ļ���±�־λ��λ����lcdDisplayUpdate = 1�����������ʾ���²�������


//SD���˵��µ���ʾ���£�����������������������ʱ����£�
void lcd_displayUpdate_cardMenu(void)
{
	uint8_t i;
	uint8_t m;
	static uint8_t n = 0;
	uint8_t lineSeclected; //��ǰѡ�е���
	char seclected_filename[32];
	char temp_string[32];
	//char temp_string2[32];
	//��������Ϣ
	switch(keyPressed)  //keyPressed�����µļ�ֵ�� �ɶ�ʱ�ж�ɨ����
	{
		case 1://����
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			if(CurrentMenu->selected == 0) //���Ƶ��������Ϸ�ҳ������βҳ��
			{
				CurrentMenu->selected = CurrentMenu->itemCount-1;
				CurrentMenu->range_to = CurrentMenu->selected;
				CurrentMenu->range_from = CurrentMenu->range_to-2;
				lcdDisplayUpdate = 1; //��Ļ��ʾ���±����λ
				break;
			}
			else //�����Ϸ�
			{
				CurrentMenu->selected--;
				if(CurrentMenu->selected < CurrentMenu->range_from)
				{
					CurrentMenu->range_from = CurrentMenu->selected;
					CurrentMenu->range_to = CurrentMenu->range_from+2;
				}
				lcdDisplayUpdate = 1; //��Ļ��ʾ���±����λ
				break;
			}
			case 2: //����
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			if(CurrentMenu->selected == CurrentMenu->itemCount-1) //���Ƶ������·�ҳ��������ҳ��
			{	
			  CurrentMenu->selected = 0;
				CurrentMenu->range_from = CurrentMenu->selected;
				CurrentMenu->range_to = CurrentMenu->range_from+2;
				lcdDisplayUpdate = 1; //��Ļ��ʾ���±����λ
				break;
			}
			else
			{
				CurrentMenu->selected++;
				if(CurrentMenu->selected > CurrentMenu->range_to)
				{
					CurrentMenu->range_to = CurrentMenu->selected;
					CurrentMenu->range_from = CurrentMenu->range_to-2;
				}
				lcdDisplayUpdate = 1; //��Ļ��ʾ���±����λ
				break;
			}
			case 3://���ؼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				if(CurrentMenu->parent!=NULL)//���˵���Ϊ�գ�����ʾ���˵�
			  {
				  //�˳��˵�ʱ������ǰ�˵���ѡ��������,������ʾ��Χ��������ʼ��״̬
			      CurrentMenu->selected = 0; 
				  CurrentMenu->range_from = 0; 
				  CurrentMenu->range_to = 2;
				  
				  CurrentMenu = CurrentMenu->parent;
			      lcdDisplayUpdate = 1; //��Ļ��ʾ���±����λ
				  return;
			  }
			 break;
			}
			case 4: //������һ���˵�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
//				if(CurrentMenu->subMenus[CurrentMenu->selected] != NULL)
//				{
//					CurrentMenu = CurrentMenu->subMenus[CurrentMenu->selected];
//					lcdDisplayUpdate = 1; //��Ļ��ʾ���±����λ
//				}
				break;
			}
			case 5: //ȷ�ϼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				if(CurrentMenu->subMenus[CurrentMenu->selected] != NULL)
				{
					CurrentMenu = CurrentMenu->subMenus[CurrentMenu->selected];
					lcdDisplayUpdate = 1; //��Ļ��ʾ���±����λ
					CurrentMenu->displayUpdate_f();
				}
				else
				{
					if(CurrentMenu->func[CurrentMenu->selected] != NULL)
					{
						CurrentMenu->func[CurrentMenu->selected]();//ִ����Ӧ�ĺ���
//						lcdDisplayUpdate = 1; //��Ļ��ʾ���±����λ  //��ȥ�����Ե�ǰ��ʽ������һ���˵�����ʹ��ʾbug
					}
				}
				break;
			}
			
			default:
			  break;
	}
	//����LCD��Ļ����ʾ����Ŀ���ݼ�ѡ���еı��
	if(1 == lcdDisplayUpdate) //�������±��
	{
		lcdDisplayUpdate = 0; //��Ļ��ʾ���±������
		
		LCD12864_Clear();
		LCD12864_ShowString(0,0,CurrentMenu->title);
		for(i=1;i<4;i++)
		{
			if(strlen((const char *)CurrentMenu->menuItems[i-1+CurrentMenu->range_from])>14) //������ʾ��Χ��ʾ��������
			{
				strncpy((char *)temp_string,(const char *)CurrentMenu->menuItems[i-1+CurrentMenu->range_from],14);
			  LCD12864_ShowString(i,1,(uint8_t *)temp_string);
			}
			else //δ������ʾ��Χȫ����ʾ
			{
			  LCD12864_ShowString(i,1,CurrentMenu->menuItems[i-1+CurrentMenu->range_from]);
			}
		}
		lineSeclected = CurrentMenu->selected; //��ȡ��ǰ�˵���ѡ�е���Ŀ��
		lineSeclected = 3-(CurrentMenu->range_to - lineSeclected);	//ת��λ�ʺ���Ļ��ʾ�ĵ�ǰѡ����
        LCD12864_ShowString(lineSeclected,0,"��");	//Ϊ��ǰѡ�е��л���ѡ�б��
		//LCD12864_HightlightShow(0,0,16,1);
		//ѡ�����ļ�����ʾ���¹�����ʾ
		if(strlen((const char *)CurrentMenu->menuItems[CurrentMenu->selected])>14)
		{
			strcpy(seclected_filename,(const char *)CurrentMenu->menuItems[CurrentMenu->selected]);
			for(m=0;m<14;m++)
			{
				temp_string[m] = seclected_filename[m+n];
			}
			n++;
			if(n>(strlen(seclected_filename)-14))
			{
				n = 0;
			}
			LCD12864_ShowString(lineSeclected,1,(uint8_t *)temp_string);
            			
		}	
	}
}

//״̬�˵��µ���ʾ���£����������������ºͳ�ʱ���£�
void lcd_displayUpdate_statusMenu(void)
{
	uint8_t i;
	uint8_t m;
	static uint8_t n = 0;
	char temp_string[32];
//	uint8_t lineSeclected; //��ǰѡ�е���
	//��������Ϣ
	switch(keyPressed)  //keyPressed�����µļ�ֵ�� ���ⲿ�жϻ��
	{
		case 1://����
		case 2: //����
		case 3://���ؼ�
		case 4: //������һ���˵�
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			break;
		case 5: //ȷ�ϼ�
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
		    CurrentMenu = &PrintingOptionsMenu;
		    lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
		    CurrentMenu->displayUpdate_f();
			break;			
		default:
			if(strlen(printingFilename) > 16) //ʵ�ֵ�����ʾ�ַ�����16��������ʾ
			{
				for(m=0;m<16;m++)
				{
					temp_string[m] = printingFilename[m+n];
				}
				n++;
				if(n>(strlen(printingFilename)-16))
				{
					n = 0;
				}
				StatusMenu.menuItems[0] = (uint8_t *)temp_string;
			}
			sprintf(percentDone,"percentDone:%d%%",card_percentDone());
			//sprintf(percentDone," %d ",f_tell(&card.fgcode)); //���ԣ��ļ���ȡ����λ�á�
			StatusMenu.menuItems[1] = (uint8_t *)percentDone;
			sprintf(temperature0," T0   :  %d/%d",(int)degHotend(0),(int)degTargetHotend(0));
			StatusMenu.menuItems[2] = (uint8_t *)temperature0;
			sprintf(temperatureBed," BED  :  %d/%d",(int)degBed(),(int)degTargetBed());
			StatusMenu.menuItems[3] = (uint8_t *)temperatureBed;
			break;
	}
	//����LCD��Ļ����ʾ����Ŀ���ݼ�ѡ���еı��
	if(1 == lcdDisplayUpdate) //�������±��
	{
		lcdDisplayUpdate = 0; //��Ļ��ʾ���±������
		LCD12864_Clear();
		for(i=0;i<4;i++)
		{
			LCD12864_ShowString(i,0,CurrentMenu->menuItems[i+CurrentMenu->range_from]);
		}
	}
}
//��ӡ��ɺ�˵�����ʾ���£�����������������������ʱ����£�
void lcd_displayUpdate_printingFinishedMenu(void)
{
	uint8_t i;
	//��������Ϣ
	switch(keyPressed)  //keyPressed�����µļ�ֵ�� ���ⲿ�жϻ��
	{
		case 1://����
		case 2: //����
		case 3://���ؼ�
		case 4: //������һ���˵�
		case 5: //ȷ�ϼ�
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
		    CurrentMenu = &MainMenu;
		    lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
		    CurrentMenu->displayUpdate_f();
			break;	
		default:
      //Ϊ��ӡ��ɵĲ˵���Ŀ�ĸ�ֵ����get_commond()��������ɵġ�
			break;
  }
	//����LCD��Ļ����ʾ����Ŀ���ݼ�ѡ���еı��
	if(1 == lcdDisplayUpdate) //�������±��
	{
		lcdDisplayUpdate = 0; //��Ļ��ʾ���±������
		LCD12864_Clear();
		for(i=0;i<4;i++)
		{
			LCD12864_ShowString(i,0,CurrentMenu->menuItems[i+CurrentMenu->range_from]);
		}
	}
}

//ȷ�����˵�����ʾ���º���������������������������ʱ����£�
void lcd_displayUpdate_YesOrNotMenu(void)
{
	uint8_t i;
	uint8_t lineSeclected; //��ǰѡ�е���
	//��������Ϣ
	switch(keyPressed)  //keyPressed�����µļ�ֵ�� ���ⲿ�жϻ��
	{
		case 1://����
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			if(CurrentMenu->selected <= 2)
				break;
			else
			{
				CurrentMenu->selected--;
//				if(CurrentMenu->selected < CurrentMenu->range_from)
//				{
//					CurrentMenu->range_from = CurrentMenu->selected;
//					CurrentMenu->range_to = CurrentMenu->range_from+3;
//				}
				lcdDisplayUpdate = 1;
				break;
			}
			case 2: //����
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			if(CurrentMenu->selected == CurrentMenu->itemCount-1)
				break;
			else
			{
				CurrentMenu->selected++;
//				if(CurrentMenu->selected > CurrentMenu->range_to)
//				{
//					CurrentMenu->range_to = CurrentMenu->selected;
//					CurrentMenu->range_from = CurrentMenu->range_to-3;
//				}
				lcdDisplayUpdate = 1;
				break;
			}
			case 3://���ؼ�
			{
//				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
//				if(CurrentMenu->parent!=NULL)//���˵���Ϊ�գ�����ʾ���˵�
//			  {
//			    CurrentMenu = CurrentMenu->parent;
//			    lcdDisplayUpdate = 1;
//			  }
//				lcdDisplayUpdate = 1;
			 break;
			}
			case 4: //������һ���˵�
			{
//				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
//				if(CurrentMenu->subMenus[CurrentMenu->selected] != NULL)
//				{
//					CurrentMenu = CurrentMenu->subMenus[CurrentMenu->selected];
//					lcdDisplayUpdate = 1;
//				}

				break;
			}
			case 5: //ȷ�ϼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				//CurrentMenu->selected = 0; //�˳��˵�ʱ������ǰ�˵���ѡ��������
				if(CurrentMenu->subMenus[CurrentMenu->selected] != NULL)
				{
					//ѡ��Noʱ������ǰ�˵���ѡ��������,������ʾ��Χ��������ʼ��״̬
//			        CurrentMenu->selected = 2; 
//				    CurrentMenu->range_from = 0; 
//				    CurrentMenu->range_to = 3;
					
					CurrentMenu = CurrentMenu->subMenus[CurrentMenu->selected];
					lcdDisplayUpdate = 1;
					CurrentMenu->displayUpdate_f();
				}
				else
				{
					if(CurrentMenu->func[CurrentMenu->selected] != NULL)
					{
						CurrentMenu->func[CurrentMenu->selected]();//ִ����Ӧ�ĺ���
						lcdDisplayUpdate = 1;
						CurrentMenu->displayUpdate_f();
					}
				}
				break;
			}
			
			default:
			break;
	}
	//����LCD��Ļ����ʾ����Ŀ���ݼ�ѡ���еı��
	if(1 == lcdDisplayUpdate) //�������±��
	{
		lcdDisplayUpdate = 0; //��Ļ��ʾ���±������
		LCD12864_Clear();
		for(i=0;i<4;i++)
		{
			if(i<2) //����ʾ�ַ������У����п�ʼ��ʾ
			{
			  LCD12864_ShowString(i,0,CurrentMenu->menuItems[i+CurrentMenu->range_from]);
			}
			else //���а�ť�������У�����ѡ��ͼ���λ���ٿ�ʼ��ʾ
			{
				LCD12864_ShowString(i,0,CurrentMenu->menuItems[i+CurrentMenu->range_from]);
			}
		}
		lineSeclected = CurrentMenu->selected; //��ȡ��ǰ�˵���ѡ�е���Ŀ��
		lineSeclected = 3-(CurrentMenu->range_to - lineSeclected);	//ת��λ�ʺ���Ļ��ʾ�ĵ�ǰѡ����
        //LCD12864_ShowString(lineSeclected,0,"->");	//Ϊ��ǰѡ�е��л���ѡ�б��
        LCD12864_HightlightShow(lineSeclected,0,16,1);		
	}
}

//һ���Ե���ʾ���º���������������������������ʱ����£�
void lcd_displayUpdate_general(void)
{
	uint8_t i;
	uint8_t lineSeclected; //��ǰѡ�е���
	//��������Ϣ
	switch(keyPressed)  //keyPressed�����µļ�ֵ�� ���ⲿ�жϻ��
	{
		case 1://����
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			if(CurrentMenu->selected <= 0)
				break;
			else
			{
				CurrentMenu->selected--;
				if(CurrentMenu->selected < CurrentMenu->range_from)
				{
					CurrentMenu->range_from = CurrentMenu->selected;
					CurrentMenu->range_to = CurrentMenu->range_from+2;
				}
				lcdDisplayUpdate = 1;
				break;
			}
			case 2: //����
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			if(CurrentMenu->selected == CurrentMenu->itemCount-1)
				break;
			else
			{
				CurrentMenu->selected++;
				if(CurrentMenu->selected > CurrentMenu->range_to)
				{
					CurrentMenu->range_to = CurrentMenu->selected;
					CurrentMenu->range_from = CurrentMenu->range_to-2;
				}
				lcdDisplayUpdate = 1;
				break;
			}
			case 3://���ؼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				//�˳��˵�ʱ������ǰ�˵���ѡ��������,������ʾ��Χ��������ʼ��״̬
			    CurrentMenu->selected = 0; 
				CurrentMenu->range_from = 0; 
				CurrentMenu->range_to = 2;
				
				if(CurrentMenu->parent!=NULL)//���˵���Ϊ�գ�����ʾ���˵�
			  {
			    CurrentMenu = CurrentMenu->parent;
			    lcdDisplayUpdate = 1;
			    CurrentMenu->displayUpdate_f();
			  }
				lcdDisplayUpdate = 1;
			 break;
			}
			case 4: //������һ���˵�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				if(CurrentMenu->subMenus[CurrentMenu->selected] != NULL)
				{
					CurrentMenu = CurrentMenu->subMenus[CurrentMenu->selected];
					lcdDisplayUpdate = 1;
					CurrentMenu->displayUpdate_f();
				}

				break;
			}
			case 5: //ȷ�ϼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				if(CurrentMenu->subMenus[CurrentMenu->selected] != NULL)
				{
					CurrentMenu = CurrentMenu->subMenus[CurrentMenu->selected];
					lcdDisplayUpdate = 1;
					CurrentMenu->displayUpdate_f();
				}
				else
				{
					if(CurrentMenu->func[CurrentMenu->selected] != NULL)
					{
						CurrentMenu->func[CurrentMenu->selected]();//ִ����Ӧ�ĺ���
						//lcdDisplayUpdate = 1;
					}
				}
				break;
			}
			
			default:
			break;
	}
	//����LCD��Ļ����ʾ����Ŀ���ݼ�ѡ���еı��
	if(1 == lcdDisplayUpdate) //�������±��
	{
		lcdDisplayUpdate = 0; //��Ļ��ʾ���±������
		LCD12864_Clear();
		LCD12864_ShowString(0,0,CurrentMenu->title);
		for(i=1;i<4;i++)
		{
			LCD12864_ShowString(i,0,CurrentMenu->menuItems[i-1+CurrentMenu->range_from]);
		}
		lineSeclected = CurrentMenu->selected; //��ȡ��ǰ�˵���ѡ�е���Ŀ��
		lineSeclected = 3-(CurrentMenu->range_to - lineSeclected);	//ת��λ�ʺ���Ļ��ʾ�ĵ�ǰѡ����
    //LCD12864_ShowString(lineSeclected,0,"->");	//Ϊ��ǰѡ�е��л���ѡ�б��	
		LCD12864_HightlightShow(lineSeclected,0,16,1);
	}
}

void lcd_displayUpdate_MoveAxisMenu(void)
{
	uint8_t i;
	uint8_t lineSeclected; //��ǰѡ�е���
	static float move_menu_scale = 10;
//	if(CurrentMenu->parent->selected == 1) //�������˵�ѡ������ƶ��������ӵĹ���
//	{
//		move_menu_scale = 0.1;
//	}
//	else if(CurrentMenu->parent->selected == 2)
//	{
//		move_menu_scale = 1;
//	}
//	else if(CurrentMenu->parent->selected == 3)
//	{
//		move_menu_scale = 10;
//	}
	//��������Ϣ
	switch(keyPressed)  //keyPressed�����µļ�ֵ�� ���ⲿ�жϻ��
	{
		case 1://����
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			if(CurrentMenu->selected == 0)
				break;
			else
			{
				CurrentMenu->selected--;
				if(CurrentMenu->selected < CurrentMenu->range_from)
				{
					CurrentMenu->range_from = CurrentMenu->selected;
					CurrentMenu->range_to = CurrentMenu->range_from+2;
				}
				lcdDisplayUpdate = 1;
				break;
			}
			case 2: //����
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			if(CurrentMenu->selected == CurrentMenu->itemCount-1)
				break;
			else
			{
				CurrentMenu->selected++;
				if(CurrentMenu->selected > CurrentMenu->range_to)
				{
					CurrentMenu->range_to = CurrentMenu->selected;
					CurrentMenu->range_from = CurrentMenu->range_to-2;
				}
				lcdDisplayUpdate = 1;
				break;
			}
			case 3://���ؼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				/*���ݵ�ǰѡ��������ƶ��������������*/
				if(CurrentMenu->selected == 0) //��ǰѡ����Ϊ������λ����˵�
				{
					if(move_menu_scale == 100)
					{
						move_menu_scale = 10;
						CurrentMenu->menuItems[0] = "��uint�� :  10mm";
					}
					else if(move_menu_scale == 10)
					{
						move_menu_scale = 1;
						CurrentMenu->menuItems[0] = "��uint�� :   1mm";
					}
					lcdDisplayUpdate = 1;
					break;
				}
				current_position[CurrentMenu->selected - 1] -= 1 * move_menu_scale; //�����˵����(CurrentMenu->selected)����ţ�X_AXIS���Ĺ���
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], manual_feedrate[Y_AXIS]/60, active_extruder);
//				if(CurrentMenu->parent!=NULL)//���˵���Ϊ�գ�����ʾ���˵�
//			  {
//			    CurrentMenu = CurrentMenu->parent;
//			    lcdDisplayUpdate = 1;
//					CurrentMenu->displayUpdate_f();
//			  }
//				lcdDisplayUpdate = 1;
			 break;
			}
			case 4: //������һ���˵�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				/*���ݵ�ǰѡ��������ƶ��������������*/
				if(CurrentMenu->selected == 0) //��ǰѡ����Ϊ������λ����˵�
				{
				    if(move_menu_scale == 1)
					{
						move_menu_scale = 10;
						CurrentMenu->menuItems[0] = "��uint�� :  10mm";
					}
					else if(move_menu_scale == 10)
					{
						move_menu_scale = 100;
						CurrentMenu->menuItems[0] = "��uint�� : 100mm";
					}
					lcdDisplayUpdate = 1;
					break;
				}
				/*������ǰ�˵����(CurrentMenu->selected)����ţ�X_AXIS���Ĺ���*/
				current_position[CurrentMenu->selected - 1] += 1 * move_menu_scale; 
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], manual_feedrate[Y_AXIS]/60, active_extruder);

//				if(CurrentMenu->subMenus[CurrentMenu->selected] != NULL)
//				{
//					CurrentMenu = CurrentMenu->subMenus[CurrentMenu->selected];
//					lcdDisplayUpdate = 1;
//				}

				break;
			}
			case 5: //ȷ�ϼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				if(CurrentMenu->parent!=NULL)//���˵���Ϊ�գ�����ʾ���˵�
			  {
			    CurrentMenu = CurrentMenu->parent;
			    lcdDisplayUpdate = 1;
					CurrentMenu->displayUpdate_f();
				  return;
			  }
				lcdDisplayUpdate = 1;
//				if(CurrentMenu->subMenus[CurrentMenu->selected] != NULL)
//				{
//					CurrentMenu = CurrentMenu->subMenus[CurrentMenu->selected];
//					lcdDisplayUpdate = 1;
//				}
//				else
//				{
//					if(CurrentMenu->func[CurrentMenu->selected] != NULL)
//					{
//						CurrentMenu->func[CurrentMenu->selected]();//ִ����Ӧ�ĺ���
//						lcdDisplayUpdate = 1;
//					}
//				}
				break;
			}
			
			default:
			break;
	}
	//����LCD��Ļ����ʾ����Ŀ���ݼ�ѡ���еı��
	if(1 == lcdDisplayUpdate) //�������±��
	{
		lcdDisplayUpdate = 0; //��Ļ��ʾ���±������
		LCD12864_Clear();
		LCD12864_ShowString(0,0,CurrentMenu->title);
		for(i=1;i<4;i++)
		{
			LCD12864_ShowString(i,0,CurrentMenu->menuItems[i-1+CurrentMenu->range_from]);
		}
		lineSeclected = CurrentMenu->selected; //��ȡ��ǰ�˵���ѡ�е���Ŀ��
		lineSeclected = 3-(CurrentMenu->range_to - lineSeclected);	//ת��λ�ʺ���Ļ��ʾ�ĵ�ǰѡ����
//        LCD12864_ShowString(lineSeclected,0,"->");	//Ϊ��ǰѡ�е��л���ѡ�б��
        LCD12864_HightlightShow(lineSeclected,0,16,1);		
	}
}

void lcd_displayUpdate_adjustParameterMenu(void)
{
	uint8_t i;
	uint8_t lineSeclected; //��ǰѡ�е���
//	uint8_t tempDiff;
	char temp_diff[16];
	char temp_speed[16];
	char temp_temp[16];
	char temp_fanSpeed[16];
	
	//��������Ϣ
	switch(keyPressed)  //keyPressed�����µļ�ֵ�� ���ⲿ�жϻ��
	{
		case 1://����
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			if(CurrentMenu->selected == 0)
				break;
			else
			{
				CurrentMenu->selected--;
				if(CurrentMenu->selected < CurrentMenu->range_from)
				{
					CurrentMenu->range_from = CurrentMenu->selected;
					CurrentMenu->range_to = CurrentMenu->range_from+2;
				}
				lcdDisplayUpdate = 1;
				break;
			}
			case 2: //����
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			if(CurrentMenu->selected == CurrentMenu->itemCount-1)
				break;
			else
			{
				CurrentMenu->selected++;
				if(CurrentMenu->selected > CurrentMenu->range_to)
				{
					CurrentMenu->range_to = CurrentMenu->selected;
					CurrentMenu->range_from = CurrentMenu->range_to-2;
				}
				lcdDisplayUpdate = 1;
				break;
			}
			//������ǰ�˵���ѡ��������Ҫ�ı�����Ĺ���
			case 3:
				if(CurrentMenu->selected == 0)  //������ֵ
				{
					keyPressed = 0;
					differenceValue -= 1;
					if(differenceValue <= 1) //��Сֵ�޷�
					{
						differenceValue = 1;
					}
//					sprintf(temp_diff,"DiffValue : %d",differenceValue);
//					AdjustParameterMenu.menuItems[0] = (uint8_t *)temp_diff;
					lcdDisplayUpdate = 1;
//					CurrentMenu->displayUpdate_f();
					break;
				}
				else if(CurrentMenu->selected == 1)
				{
					keyPressed = 0;
					feedmultiply -= differenceValue;
					if(feedmultiply <= 5) //��Сֵ�޷�
					{
						feedmultiply = 5;
					}
//					sprintf(temp_speed,"Speed : %d%%",feedmultiply );
//					AdjustParameterMenu.menuItems[1] = (uint8_t *)temp_speed;
					lcdDisplayUpdate = 1;
//					CurrentMenu->displayUpdate_f();
					break;
				}
				else if(CurrentMenu->selected == 2)
				{
					keyPressed = 0;
					target_temperature[0] -= differenceValue;
					if(target_temperature[0] <= 0) //��Сֵ�޷�
					{
						target_temperature[0] = 0;
					}
//					sprintf(temp_temp,"Temp_E0:%ddeg",target_temperature[0] );
//					AdjustParameterMenu.menuItems[2] = (uint8_t *)temp_temp;
					lcdDisplayUpdate = 1;
//					CurrentMenu->displayUpdate_f();
					break;
				}
				else if(CurrentMenu->selected == 3)
				{
					keyPressed = 0;
					fanSpeed -= differenceValue;
					if(fanSpeed <= 0) //��Сֵ�޷�
					{
						fanSpeed = 0;
					}
//					sprintf(temp_fanSpeed,"FanSpeed : %d%%",fanSpeed );
//					AdjustParameterMenu.menuItems[3] = (uint8_t *)temp_fanSpeed;
					lcdDisplayUpdate = 1;
//					CurrentMenu->displayUpdate_f();
					break;
				}
				//break;
			case 4:
				if(CurrentMenu->selected == 0)
				{
					keyPressed = 0;
					differenceValue += 1;
					if(differenceValue >= 10) //���ֵ�޷�
					{
						differenceValue = 10;
					}
//					sprintf(temp_diff,"DiffValue : %d",differenceValue);
//					AdjustParameterMenu.menuItems[0] = (uint8_t *)temp_diff;
					lcdDisplayUpdate = 1;
//					CurrentMenu->displayUpdate_f();
					break;
				}
				else if(CurrentMenu->selected == 1)
				{
					keyPressed = 0;
					feedmultiply += differenceValue;
					if(feedmultiply >= 500) //���ֵ�޷�
					{
						feedmultiply = 500;
					}
//					sprintf(temp_speed,"Speed : %d%%",feedmultiply );
//					AdjustParameterMenu.menuItems[1] = (uint8_t *)temp_speed;
					lcdDisplayUpdate = 1;
//					CurrentMenu->displayUpdate_f();
					break;
				}
				else if(CurrentMenu->selected == 2)
				{
					keyPressed = 0;
					target_temperature[0] += differenceValue;
					if(target_temperature[0] >= HEATER_0_MAXTEMP - 15) //��Сֵ�޷�
					{
						target_temperature[0] = HEATER_0_MAXTEMP - 15;
					}
//					sprintf(temp_temp,"Temp_E0  :  %d%%",target_temperature[0] );
//					AdjustParameterMenu.menuItems[2] = (uint8_t *)temp_temp;
					lcdDisplayUpdate = 1;
//					CurrentMenu->displayUpdate_f();
					break;
				}
				else if(CurrentMenu->selected == 3)
				{
					keyPressed = 0;
					fanSpeed += differenceValue;
					if(fanSpeed >= 255) //��Сֵ�޷�
					{
						fanSpeed = 255;
					}
//					sprintf(temp_fanSpeed,"FanSpeed  :  %d%%",fanSpeed );
//					AdjustParameterMenu.menuItems[3] = (uint8_t *)temp_fanSpeed;
					lcdDisplayUpdate = 1;
//					CurrentMenu->displayUpdate_f();
					break;
				}
			case 5: //ȷ�ϼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				//�˳��˵�ʱ������ǰ�˵���ѡ��������,������ʾ��Χ��������ʼ��״̬
			    CurrentMenu->selected = 0; 
				CurrentMenu->range_from = 0; 
				CurrentMenu->range_to = 3;
				
				if(CurrentMenu->parent!=NULL)//���˵���Ϊ�գ�����ʾ���˵�
				{
					CurrentMenu = CurrentMenu->parent;
					lcdDisplayUpdate = 1;
					CurrentMenu->displayUpdate_f();
					return;
		        }
				lcdDisplayUpdate = 1;
			    break;
//				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
//				if(CurrentMenu->parent!=NULL)//���˵���Ϊ�գ�����ʾ���˵�
//			  {
//			    CurrentMenu = CurrentMenu->parent;
//			    lcdDisplayUpdate = 1;
//					CurrentMenu->displayUpdate_f();
//			  }
//				lcdDisplayUpdate = 1;
//				if(CurrentMenu->subMenus[CurrentMenu->selected] != NULL)
//				{
//					CurrentMenu = CurrentMenu->subMenus[CurrentMenu->selected];
//					lcdDisplayUpdate = 1;
//				}
//				else
//				{
//					if(CurrentMenu->func[CurrentMenu->selected] != NULL)
//					{
//						CurrentMenu->func[CurrentMenu->selected]();//ִ����Ӧ�ĺ���
//						lcdDisplayUpdate = 1;
//					}
//				}
//				break;
			}
			
			default:
			break;
	}
	//����LCD��Ļ����ʾ����Ŀ���ݼ�ѡ���еı��
	if(1 == lcdDisplayUpdate) //�������±��
	{
		lcdDisplayUpdate = 0; //��Ļ��ʾ���±������
        sprintf(temp_diff,"��-Diff+��  %d",differenceValue);
		AdjustParameterMenu.menuItems[0] = (uint8_t *)temp_diff;
		sprintf(temp_speed,"��Speed �� %d%%",feedmultiply );
		AdjustParameterMenu.menuItems[1] = (uint8_t *)temp_speed;
		sprintf(temp_temp,"��Temp_0�� %d ",target_temperature[0] );
		AdjustParameterMenu.menuItems[2] = (uint8_t *)temp_temp;
		sprintf(temp_fanSpeed,"��FanSpeed��%d",fanSpeed );
		AdjustParameterMenu.menuItems[3] = (uint8_t *)temp_fanSpeed;
		LCD12864_Clear();
		LCD12864_ShowString(0,0,CurrentMenu->title);
		for(i=1;i<4;i++)
		{
			LCD12864_ShowString(i,0,CurrentMenu->menuItems[i-1+CurrentMenu->range_from]);
		}
		lineSeclected = CurrentMenu->selected; //��ȡ��ǰ�˵���ѡ�е���Ŀ��
		lineSeclected = 3-(CurrentMenu->range_to - lineSeclected);	//ת��λ�ʺ���Ļ��ʾ�ĵ�ǰѡ����
        //LCD12864_ShowString(lineSeclected,0,"->");	//Ϊ��ǰѡ�е��л���ѡ�б��
        LCD12864_HightlightShow(lineSeclected,0,16,1);		
	}
}

void lcd_displayUpdate_changeParameterMenu_speed(void)
{
	uint8_t i;
	static uint8_t diffValue = 5; //ÿ�����ӻ��С�Ĳ�ֵ
	char tempStr1[16],tempStr2[16];
	//��������Ϣ
	switch(keyPressed)  //keyPressed�����µļ�ֵ�� ���ⲿ�жϻ��
	{
		case 1://����
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
		  temp_feedMultiply += diffValue;
		  if(temp_feedMultiply >= 500) //���ֵ�޷�
			{
				temp_feedMultiply = 500;
			}
		  sprintf(tempStr1,"Speed:%d%%",temp_feedMultiply );
		  ChangeParameterMenu.menuItems[2] = (uint8_t *)tempStr1;
		  lcdDisplayUpdate = 1;
		  CurrentMenu->displayUpdate_f();
			break;
		case 2: //����
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			temp_feedMultiply -= diffValue;
			if(temp_feedMultiply <= 5)
			{
				temp_feedMultiply = 5;
			}
			sprintf(tempStr1,"Speed:%d%%",temp_feedMultiply );
			ChangeParameterMenu.menuItems[2] = (uint8_t *)tempStr1;
			lcdDisplayUpdate = 1;
			CurrentMenu->displayUpdate_f();
			break;
		case 3://���ؼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
//			  diffValue--;
//				if(diffValue <= 1) //��ֵ��Сֵ�޷�
//				{
//					diffValue = 1;
//				}
//				sprintf(tempStr2,"DiffValue:%d",diffValue );
//				ChangeParameterMenu.menuItems[3] = (uint8_t *)tempStr2;
//				lcdDisplayUpdate = 1;
//				CurrentMenu->displayUpdate_f();
				break;
			}
			case 4: //������һ���˵�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
//			  diffValue++;
//				if(diffValue >= 10) //��ֵ��Сֵ�޷�
//				{
//					diffValue = 10;
//				}
//				sprintf(tempStr2,"DiffValue:%d",diffValue );
//				ChangeParameterMenu.menuItems[3] = (uint8_t *)tempStr2;
//				lcdDisplayUpdate = 1;
//				CurrentMenu->displayUpdate_f();
				break;
			}
			case 5: //ȷ�ϼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				feedmultiply = temp_feedMultiply;
				if(CurrentMenu->parent!=NULL)//���˵���Ϊ�գ�����ʾ���˵�
			  {
			    CurrentMenu = CurrentMenu->parent;
			    lcdDisplayUpdate = 1;
				CurrentMenu->displayUpdate_f();
			  }
				break;
			}
			
			default:
			break;
	}
  
	//����LCD��Ļ����ʾ����Ŀ���ݼ�ѡ���еı��
	if(1 == lcdDisplayUpdate) //�������±��
	{
		lcdDisplayUpdate = 0; //��Ļ��ʾ���±������
		LCD12864_Clear();
		for(i=0;i<4;i++)
		{
			LCD12864_ShowString(i,0,CurrentMenu->menuItems[i+CurrentMenu->range_from]);
		}
	}
}

void lcd_displayUpdate_changeParameterMenu_temp(void)
{
	uint8_t i;
	static uint8_t diffValue = 1; //ÿ�����ӻ��С�Ĳ�ֵ
//  uint16_t temp_e0TargetTemp ; //
	char tempStr1[16],tempStr2[16];
	temp_e0TargetTemp = degTargetHotend(0);
	//��������Ϣ
	switch(keyPressed)  //keyPressed�����µļ�ֵ�� ���ⲿ�жϻ��
	{
		case 1://����
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
		  temp_e0TargetTemp +=diffValue;
		  if(temp_e0TargetTemp >= HEATER_0_MAXTEMP - 15) //���ֵ�޷�
			{
				temp_e0TargetTemp = HEATER_0_MAXTEMP - 15;
			}
		  sprintf(tempStr1,"Temp0:%ddeg",temp_e0TargetTemp );
		  ChangeParameterMenu.menuItems[2] = (uint8_t *)tempStr1;
		  lcdDisplayUpdate = 1;
		  CurrentMenu->displayUpdate_f();
		case 2: //����
			keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			temp_e0TargetTemp -=diffValue;
			if(temp_e0TargetTemp <= 5)
			{
				temp_e0TargetTemp = 5;
			}
			sprintf(tempStr1,"Temp0:%ddeg",temp_e0TargetTemp );
			ChangeParameterMenu.menuItems[2] = (uint8_t *)tempStr1;
			lcdDisplayUpdate = 1;
			CurrentMenu->displayUpdate_f();
			break;
		case 3://���ؼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			  diffValue--;
				if(diffValue <= 1) //��ֵ��Сֵ�޷�
				{
					diffValue = 1;
				}
				sprintf(tempStr2,"DiffValue:%d",diffValue );
				ChangeParameterMenu.menuItems[3] = (uint8_t *)tempStr2;
				lcdDisplayUpdate = 1;
				CurrentMenu->displayUpdate_f();
				break;
			}
			case 4: //������һ���˵�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
			  diffValue++;
				if(diffValue >= 10) //��ֵ��Сֵ�޷�
				{
					diffValue = 10;
				}
				sprintf(tempStr2,"DiffValue:%d",diffValue );
				ChangeParameterMenu.menuItems[3] = (uint8_t *)tempStr2;
				lcdDisplayUpdate = 1;
				CurrentMenu->displayUpdate_f();
				break;
			}
			case 5: //ȷ�ϼ�
			{
				keyPressed = 0; //���µļ�ֵ���㣬�����һֱִ�и�״̬
				setTargetHotend(temp_e0TargetTemp,0);
				if(CurrentMenu->parent!=NULL)//���˵���Ϊ�գ�����ʾ���˵�
			  {
			    CurrentMenu = CurrentMenu->parent;
			    lcdDisplayUpdate = 1;
					CurrentMenu->displayUpdate_f();
			  }
				break;
			}
			
			default:
			break;
	}
  
	//����LCD��Ļ����ʾ����Ŀ���ݼ�ѡ���еı��
	if(1 == lcdDisplayUpdate) //�������±��
	{
		lcdDisplayUpdate = 0; //��Ļ��ʾ���±������
		LCD12864_Clear();
		for(i=0;i<4;i++)
		{
			LCD12864_ShowString(i,0,CurrentMenu->menuItems[i+CurrentMenu->range_from]);
		}
	}
}


//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���  Example��card_getFileNum("0:/GCODE");
uint16_t card_getFileNum(uint8_t *path)
{	  
	u8 res;   //�����ڲ����ú����ķ���ֵ�����ڲ��Ժ����Ƿ�����ִ��
	u16 rval=0;  //��������ֵ
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO tfileinfo;	//��ʱ�ļ���Ϣ	
	u8 *fn;
	
  res=f_opendir(&tdir,(const TCHAR*)path); 	//��Ŀ¼	
//	while(res)//��GCODE�ļ���
// 	{	    
//		LCD12864_ShowString(1,0,"SD���޴˸�Ŀ¼��");
//		delay_ms(200);				  
//		LCD12864_Clear();//�����ʾ	     
//		delay_ms(200);				  
//	} 
  tfileinfo.lfsize=_MAX_LFN*2+1;				//���ļ�����󳤶�
	tfileinfo.lfname=mymalloc(tfileinfo.lfsize);//Ϊ���ļ������������ڴ�
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	    res=f_readdir(&tdir,&tfileinfo);       		//��ȡĿ¼�µ�һ���ļ�
	    if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//������/��ĩβ��,�˳�		  
      fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	 //��ȡ�ļ�����
			if((res&0XF0)==0X70)//ȡ����λ,�����ǲ���Gcode�ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	} 
	myfree(tfileinfo.lfname);
	return rval;
}
//���ڶ�ȡSD��GCODEĿ¼�µ���Ч�ļ�����Ŀ����¼����ֵ�������ļ����б��ȡ����
void card_readFileListInfo(void)
{ 
	u8 res;              //��������ֵ
 	DIR gcodir;	 		     //Gcode�ļ�Ŀ¼
	FILINFO gcofileinfo; //�ļ���Ϣ
	u8 *fn;   			     //���ļ���
	u8 temp_fn[100][30];  //���ڴ洢sd���ļ�������ʱ����
	u8 *pname;			     //��·�����ļ���
	//u16 totgconum; 		   //Gcode�ļ�����
//	u16 curindex;		     //Gcode�ļ���ǰ����
	u16 temp;
	u16 *gcoindextbl;	   //Gcode�ļ������� 	
	 
 	//LCD12864_Clear(); //����ý������������Ϊ�½������ʾ��׼��
	 
		//���SD����Ŀ¼�Ƿ���GCODE�ļ���	
	//	while(f_opendir(&gcodir,"0:/GCODE"))//��GCODE�ļ���
	// 	{	    
	//		LCD12864_ShowString(1,0,"SD������GCODE �ļ���...");
	//		delay_ms(200);				  
	//		LCD12864_Clear();//�����ʾ	     
	//		delay_ms(200);				  
	//	} 
		
		totgconum = card_getFileNum("0:/GCODE"); //�õ�����Ч�ļ���
		//LCD12864_ShowNum(2,0,totgconum); //��ʾ��Ч�ļ������������ڲ���
		//�����Ч�ļ�����Ŀ�Ƿ�Ϊ0
	//	while(totgconum==NULL)//�ļ���Ч�ļ���ĿΪ0		
	// 	{	    
	//		LCD12864_ShowString(1,0,"û�пɴ�ӡ�ļ�!");
	//		delay_ms(200);				  
	//		LCD12864_Clear();//�����ʾ	     
	//		delay_ms(200);				  
	//	}
		gcofileinfo.lfsize=_MAX_LFN*2+1;					     	  //���ļ�����󳤶�
		gcofileinfo.lfname=mymalloc(gcofileinfo.lfsize);	//Ϊ���ļ������������ڴ�
		pname=mymalloc(gcofileinfo.lfsize);				        //Ϊ��·�����ļ��������ڴ�
		gcoindextbl=mymalloc(2*totgconum);				//����2*totgconum���ֽڵ��ڴ�,���ڴ���ļ�����
		//zzz = mymalloc(10*totgconum);    //Ϊ�ļ����洢�������ռ䣬��֪���ķ�����ռ䣬�����������ġ�
		//����ڴ�����Ƿ����
	// 	while(gcofileinfo.lfname==NULL||pname==NULL||gcoindextbl==NULL)//�ڴ�������
	// 	{	    
	//		LCD12864_ShowString(1,0,"�ڴ����ʧ��!");
	//		delay_ms(200);				  
	//		LCD12864_Clear();//�����ʾ	     
	//		delay_ms(200);				  
	//	}
		//��¼����
		CardMenu.itemCount = totgconum; //��ʼ���ļ��б����ļ���Ŀ
		res = f_opendir(&gcodir,"0:/GCODE"); //��Ŀ¼
		if(res == FR_OK)
		{
			curindex=0;//��ǰ����Ϊ0
			while(1)//ȫ����ѯһ�飬��¼����ֵ�����ļ���ǰĿ¼Gcode�ļ����б�洢��zzz������
			{
				temp=gcodir.index;								//��¼��ǰindex
				res=f_readdir(&gcodir,&gcofileinfo);       		//��ȡĿ¼�µ�һ���ļ������´�ִ��ʱ���Զ���ȡ��һ���ļ�
				if(res!=FR_OK||gcofileinfo.fname[0]==0) break;	//������/��ĩβ��,�˳�		  
				fn=(u8*)(*gcofileinfo.lfname?gcofileinfo.lfname:gcofileinfo.fname);	//��ȡ�ļ���				
				strcpy((char*)pname,"0:/GCODE/");				    //����·��(Ŀ¼)
				strcat((char*)pname,(const char*)fn);  			//���ļ������ں��棬�γɴ�·�����ļ��� 
				res=f_typetell(fn);	 //��ȡ�ļ�����
				if((res&0XF0)==0X70)//ȡ����λ,�����ǲ���Gcode�ļ�	 
				{
					gcoindextbl[curindex]=temp;//��¼����
					//strcpy((char *)temp_fn[curindex],"  "); //
	//				zzz[curindex] = temp_fn[curindex]; //�����ã�����ά�����е��ַ�����ȡ��zzz������
					strcpy((char *)cardFileList[curindex],(const char*)fn);
//					strncpy((char *)temp_fn[curindex],(const char*)fn,14); //���ļ���ǰ14���ַ��洢��1����ά������
					strcpy((char *)temp_fn[curindex],(const char*)fn);
					CardMenu.menuItems[curindex] = temp_fn[curindex]; //����ά�����е��ַ�����ȡ��CardMenu.menuItems�����У�����ʼ���ļ��б�����
					curindex++;
				}	    
			} 
		}   
			//���ڲ���CardMenu.menuItems[]�Ƿ�洢��SD���ļ��б�
	//	for(curindex=0;curindex<totgconum;curindex++) 
	//	{
	//	  LCD12864_ShowString(3,0,"��ʾ���⡣��");
	//		LCD12864_ShowString(curindex,0,CardMenu.menuItems[curindex]);
	//	}   	 
	//while(1);
		 	
//  if(SD_CD) //��������ʾsd���ļ��б�ҳ��ʱ���γ�sd��������ҳ��
//	{
//		CurrentMenu = &MainMenu;
//		lcdDisplayUpdate = 1; //�л��˵���Ҫ����ʾ���±�־λ��λ������ʹ��ǰ�˵����µ��л��Ĳ˵�
//	}		
		myfree(gcofileinfo.lfname);	//�ͷ��ڴ�			    
		myfree(pname);				//�ͷ��ڴ�			    
		myfree(gcoindextbl);		//�ͷ��ڴ�
		//myfree(zzz);	//������
}












