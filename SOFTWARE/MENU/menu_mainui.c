//#include "includes.h" 
//#include "dma.h"
#include "gui_conf.h"
#include "picdecode.h"
#include "touch.h"
#include "menu_mainui.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//��������� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/2/22
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

//������ ������
m_mui_dev muidev;


//icos��·����
u8*const mui_icos_path_tbl[6]=
{ 
	"1:/SYSTEM/SYSICO/disk.bmp",
	"1:/SYSTEM/SYSICO/piano.bmp",	    
	"1:/SYSTEM/SYSICO/calculator.bmp",	    
	"1:/SYSTEM/SYSICO/console.bmp",	    
	"1:/SYSTEM/SYSICO/movie.bmp",	    
	"1:/SYSTEM/SYSICO/binoculars.bmp",	    
	//"1:/SYSTEM/SYSICO/paint.bmp",	    
	//"1:/SYSTEM/SYSICO/wireless.bmp",	    
	//"1:/SYSTEM/SYSICO/notebook.bmp", 	    
};  

//����ICOS�Ķ�Ӧ��������
u8*const icos_name_tbl[GUI_LANGUAGE_NUM][6]=
{ 
	{
		"����","����","������",
		"����","��Ӱ","��Զ��",
		//"����","���ߴ���","���±�",	   
	}, 
	{
		"��ӈD��","���a���","USB�B��",
		"��������","�r�","ϵ�y�O��",
		//"�|������","�o����ݔ","ӛ�±�",	   
	}, 
	{
		"EBOOK","PHOTOS","USB",
		"APP","TIME","SETTINGS",
		//"PAINT","WIRELESS","NOTEPAD",	   
	},  
};						  

//��ʼ��spb��������, ������������	
//mode:0,��������icos������
//     1,�������֣�������ͼƬ
void mui_init(uint8_t mode)
{
	u16 i,j;
	u16 offx=0,offy=0;
	//if(lcddev.width<240||lcddev.height<320)return ;//��Ļ�ߴ粻��С��320*240;
	offx=(lcddev.width-320)/2;
	offy=(lcddev.height-240)/2;
	muidev.status=0x0F;
	for(i=0;i<2;i++)  //ѭ��Ϊÿ��ͼ���ļ�������
	{
		for(j=0;j<3;j++)
		{
			muidev.icos[i*3+j].x=10+j*105+offx;
			muidev.icos[i*3+j].y=10+i*115+offy;
			muidev.icos[i*3+j].width=95;   //ͼ�����ؿ��90+5
			muidev.icos[i*3+j].height=105; //ͼ�����ظ߶�90+���ָ߶�12+3
			muidev.icos[i*3+j].path=(u8*)mui_icos_path_tbl[i*3+j];
			muidev.icos[i*3+j].name=(u8*)icos_name_tbl[gui_phy.language][i*3+j]; 
		}
	} 
	if(mode==0)return;
	mui_load_icos();
	//ָ��lcd
	gui_phy.read_point=LCD_ReadPoint;
	gui_phy.draw_point=LCD_Fast_DrawPoint;
	gui_phy.lcdwidth=lcddev.width;
	gui_phy.lcdheight=lcddev.height; 
	pic_phy.read_point=LCD_ReadPoint;
	pic_phy.draw_point=LCD_Fast_DrawPoint; 
}
//������������������
//language:0,��������;1,��������;2,Ӣ��
void mui_language_set(u8 language)
{
	u8 i;
	if(language>2)return;
	for(i=0;i<6;i++)muidev.icos[i].name=(u8*)icos_name_tbl[language][i]; 
}
//װ��ICOS
void mui_load_icos(void)
{
	u8 i,j; 
	LCD_Fill(0,0,lcddev.width,8,BLACK);
	LCD_Fill(0,0,8,lcddev.height,BLACK);
	LCD_Fill(0,lcddev.height-8,lcddev.width,lcddev.height,BLACK);
	LCD_Fill(lcddev.width-8,0,lcddev.width,lcddev.height,BLACK);  
	gui_draw_arcrectangle(2,2,lcddev.width-4,lcddev.height-4,6,1,MUI_IN_BACKCOLOR,MUI_IN_BACKCOLOR);
	for(i=0;i<2;i++)
	{
		for(j=0;j<3;j++)
		{
			minibmp_decode(muidev.icos[i*3+j].path,muidev.icos[i*3+j].x+5,muidev.icos[i*3+j].y,muidev.icos[i*3+j].width,muidev.icos[i*3+j].height,0,0);
			gui_show_strmid(muidev.icos[i*3+j].x,muidev.icos[i*3+j].y+92,muidev.icos[i*3+j].width,12,MUI_FONT_COLOR,12,muidev.icos[i*3+j].name);//��ʾ����  
		}
	}  	
} 
//����ѡ���ĸ�ͼ��
//sel:0~8����ǰҳ��ѡ��ico
void mui_set_sel(u8 sel)
{
	u8 oldsel=muidev.status&0X0F;
	if(oldsel<6) //���ͼ���ѡ��״̬
	{
		LCD_Fill(muidev.icos[oldsel].x-5,muidev.icos[oldsel].y-5,muidev.icos[oldsel].x+muidev.icos[oldsel].width,muidev.icos[oldsel].y+muidev.icos[oldsel].height,MUI_IN_BACKCOLOR);
		minibmp_decode(muidev.icos[oldsel].path,muidev.icos[oldsel].x+0,muidev.icos[oldsel].y,muidev.icos[oldsel].width,muidev.icos[oldsel].height,0,0);		
		gui_show_strmid(muidev.icos[oldsel].x,muidev.icos[oldsel].y+92,muidev.icos[oldsel].width,12,MUI_FONT_COLOR,12,muidev.icos[oldsel].name);//��ʾ����
	}
	muidev.status=(muidev.status&0XF0)+sel;  

	LCD_Fill(muidev.icos[sel].x-5,muidev.icos[sel].y-5,muidev.icos[sel].x+muidev.icos[sel].width,muidev.icos[sel].y+muidev.icos[sel].height,MUI_BACK_COLOR);
	minibmp_decode(muidev.icos[sel].path,muidev.icos[sel].x+0,muidev.icos[sel].y,muidev.icos[sel].width,muidev.icos[sel].height,0,0);		

  	gui_show_strmid(muidev.icos[sel].x,muidev.icos[sel].y+92,muidev.icos[sel].width,12,MUI_FONT_COLOR,12,muidev.icos[sel].name);//��ʾ����
} 

//��Ļ�������
//����ֵ:0~8,��˫����ͼ����.		    
//       0xff,û���κ�ͼ�걻˫�����߰���
u8 mui_touch_chk(void)
{		 
	u8 i=0xff;
	tp_dev.scan(0);//ɨ��																		 
	if(tp_dev.sta&TP_PRES_DOWN)//�а���������
	{
		muidev.status|=0X80;	//�������Ч����
		muidev.tpx=tp_dev.x[0];
		muidev.tpy=tp_dev.y[0];
	}else if(muidev.status&0X80)//�����ɿ���,��������Ч����
	{
		
		for(i=0;i<6;i++)
		{
			if((muidev.tpx>muidev.icos[i].x)&&(muidev.tpx<muidev.icos[i].x+muidev.icos[i].width)&&(muidev.tpx>muidev.icos[i].x)&&
			   (muidev.tpy<muidev.icos[i].y+muidev.icos[i].height))
			{
				break;//�õ�ѡ�еı��	
			}
		}
		if(i<6)
		{
			if(i!=(muidev.status&0X0F))	//ѡ���˲�ͬ��ͼ��,�л�ͼ��
			{
				mui_set_sel(i);
				i=0xff;
			}else 
			{ 
				muidev.status|=0X0F;//����ѡ�е�ͼ��
			}
		}else i=0xff;//��Ч�ĵ㰴.
		muidev.status&=0X7F;//���������Ч��־
	} 	 
	return i; 
}














































