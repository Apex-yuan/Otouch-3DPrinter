#include "menucontrol.h"
#include "menu_mainui.h"
#include "menu_filebrowse.h"
#include "menu_setting.h"
#include "menu_picview.h"

static uint32_t tp_scan_next_update_millis=0;
static uint32_t display_next_update_millis=0;

menuFunc_t currentMenu = main_interface; //printer_interface, setting_interface, filebrowse_interface
menuFunc_t lastMenu;
bool redraw_screen = true;
u8 test_data_2=12;
int test_data_1=11;
bool windows_flag = false;


void interface_update(void)
{ 
	if (tp_scan_next_update_millis < millis())
	{
	  tp_dev.scan(0);
	  in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);
	  tp_scan_next_update_millis = millis() + 10;
	  (*currentMenu)();
	}
}

void main_interface(void)
{
	u8 selx=0;
	if(redraw_screen == true)
	{
		redraw_screen = false;
	  mui_init(1);
	}		
	//while(1)
	{
		selx = mui_touch_chk();
		switch(selx)
		{
			case 0:	//����ͼ��
				currentMenu = filebrowse_interface;
				return;//break; //���������return�����˳���ǰ��������breakֻ������switch������������whileѭ��
			case 1:	//ͼƬ���		
				currentMenu = picview_interface; 
				return;//break;
			case 2:	//ͼƬ���		
				 sysset_play(); 
				break;
		}	
  }
}
void filebrowse_interface(void)
{
  GcodeFile_Browse(); //�����ļ�·����ֻ����ʾgcode�ļ�,��˫�����ļ���������Ӧ����
}
void picview_interface(void)
{
  GcoPicFile_Browse(); 
}
void print_interface(void)
{
			display_next_update_millis=0;
      gui_phy.back_color=GRAYBLUE;	
			gui_fill_rectangle(0,0,lcddev.width,lcddev.height,GRAYBLUE );
			gui_fill_rectangle(0,0,lcddev.width,40,LIGHTBLUE);  
			gui_draw_hline(0,40,lcddev.width,GRAY);
			
       gui_show_strmid(0,0,lcddev.width,40,WHITE,16,(u8 *)buf);
	     gui_show_strmid(0,0,lcddev.width,lcddev.height,WHITE,16,(u8 *)(&buf[49]));
	//LCD_ShowString(10,210,230,24,24,(u8 *)buf);
	while(1)
	{
	//KEY_Scan(0); 	//���ذ���ɨ��Ľ��
	  if(KEY_Scan(0)==KEY0_PRESS) //�������˵�
		{
			//res_key = 0;
			currentMenu = main_interface;
			break;
		}
	}
}
