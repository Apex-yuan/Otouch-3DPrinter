#include "menu_filebrowse.h"
#include "menucontrol.h"
#include "menu_setting.h"
#include "cardreader.h"

UINT bww; //������
char buf[100]; //������

//�ڵ�ǰ�汾�в��ò������������������Ϊ����������˺ü����汾�ȽϾ��䣬�ʱ�������������ο���
//�����ض��ļ�����������ɴ�������������Ļ������޸ĵõ���
uint8_t File_Perview(void)
{			
	//******************************��ʼ�������ڲ�����******************************//	 	   
	u8 res_touch;   //�������İ���
	u8 res_key;     //�����İ���
	u8 rval=0;			//����ֵ	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
 	_btn_obj* rbtn;		//���ذ�ť�ؼ�
  _filelistbox_obj * flistbox;  //�ļ���
	_filelistbox_list * filelistx; 	//�ļ� 
	
  //******************************�����ļ��б�******************************//
	app_filebrower((u8*)"����",0X07);//�����ļ��������
  flistbox = filelistbox_creat(0,gui_phy.tbheight,lcddev.width,lcddev.height-gui_phy.tbheight*2,1,gui_phy.listfsize);//����һ��filelistbox
	if(flistbox==NULL)rval=1;							//�����ڴ�ʧ��.
	else  
	{
		//������ʾ������չ�����ļ�������filelistbox.h�ļ��в鿴��������Ϊ0xff�����ʾ�������͵��ļ�������������GPIO����
		flistbox->fliter=0xff;//FLBOX_FLT_GCODE;//|FLBOX_FLT_TEXT|FLBOX_FLT_LRC;	//�����ı��ļ�
		filelistbox_add_disk(flistbox);					//��Ӵ���·��(��filelist����ǰ,����ִ��һ�θú���)
 		filelistbox_draw_listbox(flistbox);     //�����ļ��б�
	} 	
	//******************************�������ذ�ť******************************//
	rbtn=btn_creat(lcddev.width-2*gui_phy.tbfsize-8-1,lcddev.height-gui_phy.tbheight,2*gui_phy.tbfsize+8,gui_phy.tbheight-1,0,0x03);//�������ְ�ť	
	if(rbtn==NULL)rval=1;	//û���㹻�ڴ湻����
	else
	{																				
	 	rbtn->caption=(u8*)"����";//���� 
	 	rbtn->font=gui_phy.tbfsize;//�����µ������С	 	 
		rbtn->bcfdcolor=RED;	//����ʱ����ɫ
		rbtn->bcfucolor=WHITE;	//�ɿ�ʱ����ɫ
		btn_draw(rbtn);//����ť
	}
	//******************************������Ϣ����******************************//
   	while(rval==0)
	{
		tp_dev.scan(0) ;    
		in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);	//�õ�������ֵ   
		delay_ms(5); //����
		filelistbox_check(flistbox,&in_obj);    //ɨ���ļ���������ļ��еĻ�����ļ���
		if(flistbox->dbclick==0X81)//˫���ļ���
		{
			flistbox->dbclick =0X00; //���˫���ļ���״̬
			//��������޸Ķ�����ʵ�֣�˫��Ŀ���ļ�ʹ֮û�з�Ӧ����ʱͨ���޸�
			//GUI��filelistbox.c�ļ��е�276�У���filelistbox_check(flistbox,&in_obj);������
			//��˫��Ŀ���ļ�ʱ��ʹ֮�����κδ����������������ʱ�ǵüӻ�����
			//ԭ�У�if(filelistbox->dbclick==0X81&&filelistbox->list!=NULL)filelist_delete(filelistbox);//ɾ��֮ǰ����Ŀ,�ͷ��ڴ�
      //�޸ĺ�//if(filelistbox->dbclick==0X81&&filelistbox->list!=NULL)filelist_delete(filelistbox);//ɾ��֮ǰ����Ŀ,�ͷ��ڴ�		
		}	
		res_touch = btn_check(rbtn,&in_obj);		//ɨ�践�ش������İ������
		res_key = KEY_Scan(0);                  //���ذ���ɨ��Ľ��
		if(res_touch==1)  //������Ļ���ذ�����Ϣ
		{
			if(((rbtn->sta&0X80)==0))//��ť״̬�ı���  //�˴��и�bug���������Ұ�KEY0��Ӧ�ý����if��䣬��ʵ�ʻ���롣�����ﵽ����Ҫ��Ч��
			{
				filelistx=filelist_search(flistbox->list,flistbox->selindex);//�õ���ʱѡ�е�list����Ϣ
				if(filelistx->type==FICO_DISK)//�Ѿ�������������,����������
				{				 
//					btn_delete(rbtn);				//ɾ����ť	 //���Ӵ˾书��Ҳ�������Ӵ˾�Ϊ���ͷŰ�ťռ���ڴ�
					//mui_init(1); 	//Main UI ��ʼ��
					currentMenu = main_interface;
					break;
				}else filelistbox_back(flistbox);//�˻���һ��Ŀ¼	 
			}				
		}
		if(res_key==KEY0_PRESS) //�������˵�
		{
			res_key = 0;
			currentMenu = main_interface;break;
//			filelistx=filelist_search(flistbox->list,flistbox->selindex);//�õ���ʱѡ�е�list����Ϣ
//			do
//			{		
//				filelistbox_back(flistbox);//�˻���һ��Ŀ¼
//				filelistx=filelist_search(flistbox->list,flistbox->selindex);//�õ���ʱѡ�е�list����Ϣ
//			}while(filelistx->type!=FICO_DISK); //ֻҪ��ǰĿ¼�����Ͷ��岻�Ǵ��̣���һֱ����
//			btn_delete(rbtn);				//ɾ����ť	 //���Ӵ˾书��Ҳ�������Ӵ˾�Ϊ���ͷŰ�ťռ���ڴ�
//			mui_init(1); 	//Main UI ��ʼ������������
		}
	}
	filelistbox_delete(flistbox);	//ɾ��filelist���ͷ�filelistռ���ڴ�
	//�����������������ʹ������������ڰ�ť������ʾ֮ǰ��ť�ı���ɫ�����Բ��ܼ��ڴ˴���
	//��Ϊ���ͷ�����ռȡ���ڴ潫��ŵ�ǰ��ʵ�֡���Ϊ���������ɾ����ť���ָ�����ɫ��
	btn_delete(rbtn);				//ɾ����ť���ͷŰ�ťռ���ڴ� 	 	   	
	return rval; 	
}

//�������Ŀ¼������ʾ���е�.gcode�ļ���
//����ֵ�Ƿ�˫����gcode�ļ���0��˫���ˣ�����ֵ��δ˫����
uint8_t GcodeFile_Browse(void)
{			
	//******************************��ʼ�������ڲ�����******************************//	 	   
	u8 res_touch;   //�������İ���
	u8 res_key;     //�����İ���
	u8 res;         //��¼�����а��µİ�����1��ȷ�ϼ���2�����ؼ�
	u8 rval=0;			//
 	_btn_obj* rbtn;		//���ذ�ť�ؼ�
  _filelistbox_obj * flistbox;  //�ļ���
	_filelistbox_list * filelistx; 	//�ļ� 
	
//	u8 selx=0XFF;	
	u8 *pname=0;
// 	_filelistbox_list * filelistx;	
	
  //******************************�����ļ��б�******************************//
	app_filebrower((u8*)"����",0X07);//�����ļ��������
  flistbox = filelistbox_creat(0,gui_phy.tbheight,lcddev.width,lcddev.height-gui_phy.tbheight*2,1,gui_phy.listfsize);//����һ��filelistbox
	if(flistbox==NULL)rval=1;							//�����ڴ�ʧ��.
	else  
	{
		//������ʾ������չ�����ļ�������filelistbox.h�ļ��в鿴��������Ϊ0xff�����ʾ�������͵��ļ�������������GPIO����
		flistbox->fliter=FLBOX_FLT_GCODE;     	//����gcode�ļ�
		filelistbox_add_disk(flistbox);					//��Ӵ���·��(��filelist����ǰ,����ִ��һ�θú���)
 		filelistbox_draw_listbox(flistbox);     //�����ļ��б�
	} 	
	//******************************�������ذ�ť******************************//
	rbtn=btn_creat(lcddev.width-2*gui_phy.tbfsize-8-1,lcddev.height-gui_phy.tbheight,2*gui_phy.tbfsize+8,gui_phy.tbheight-1,0,0x03);//�������ְ�ť	
	if(rbtn==NULL)rval=1;	//û���㹻�ڴ湻����
	else
	{																				
	 	rbtn->caption=(u8*)"����";//���� 
	 	rbtn->font=gui_phy.tbfsize;//�����µ������С	 	 
		rbtn->bcfdcolor=RED;	//����ʱ����ɫ
		rbtn->bcfucolor=WHITE;	//�ɿ�ʱ����ɫ
		btn_draw(rbtn);//����ť
	}
	//******************************������Ϣ����******************************//
   	while(rval==0)
	{
		tp_dev.scan(0) ;    
		in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);	//�õ�������ֵ   
		delay_ms(5); //����
		filelistbox_check(flistbox,&in_obj);    //ɨ���ļ���������ļ��еĻ�����ļ���
		if(flistbox->dbclick==0X81)//˫���ļ���
		{
			flistbox->dbclick =0X00; //���˫���ļ���״̬
			//currentMenu = main_interface;
			//return 0; //����ֵ����ʾ˫���ļ��ˡ�
			res=window_msg_box((lcddev.width-200)/2,(lcddev.height-80)/2,200,80,"",(u8*)"�Ƿ��ӡ",12,0,0X03,0);
			if(res==1) 
			{
//				rval=f_opendir(&ebookdir,(const TCHAR*)flistbox->path); //��ѡ�е�Ŀ¼
//				if(rval)break;
//				dir_sdi(&ebookdir,flistbox->findextbl[flistbox->selindex-flistbox->foldercnt]);
//				rval=f_readdir(&ebookdir,&ebookinfo);//��ȡ�ļ���Ϣ
//				if(rval)break;//�򿪳ɹ�    
//				fn=(u8*)(*ebookinfo.lfname?ebookinfo.lfname:ebookinfo.fname);
//				pname=gui_memin_malloc(strlen((const char*)fn)+strlen((const char*)flistbox->path)+2);//�����ڴ�
				//currentMenu = sysset_play;
				
				filelistx = filelist_search(flistbox->list,flistbox->selindex);
				strcpy((char *)card.filename,(const char*)filelistx->name);
				pname=gui_memin_malloc(strlen((const char*)filelistx->name)+strlen((const char*)flistbox->path)+2);
				if(pname)  
				{	
					pname=gui_path_name(pname,flistbox->path,filelistx->name);
					rval=f_open(&card.fgcode,(const TCHAR*)pname,FA_READ);
					f_read(&card.fgcode, buf,100,&bww); //��ȡgcode�ļ����ݲ��ԣ��ɹ���lcd��Ļ����ʾ��gcode���ļ����ݡ�
						
					gui_memin_free(pname);
					if (rval==0)
					{
						currentMenu = print_interface;
						//redraw_screen=true;
						//windows_flag=false;	
						filelistbox_delete(flistbox);												
						card_startFileprint();
						//starttime=millis();
					}
				}	
				break;
			}
			else
			{
					filelistbox_draw_listbox(flistbox); //�ػ����ļ��б���������м��ж��Ƿ��ӡ�Ĵ��岻����ʧ������ֻ֪���������Խ���������б�Ҫ�����Ż�
          //break;	//���ⲻ����break��䣬����break������������������䣬�����ֱ���������̽��棬������о�������			
			}
		}	
		res_touch = btn_check(rbtn,&in_obj);		//ɨ�践�ش������İ������
		res_key = KEY_Scan(0);                  //���ذ���ɨ��Ľ��
		if(res_touch==1)  //������Ļ���ذ�����Ϣ
		{
			if(((rbtn->sta&0X80)==0))//��ť״̬�ı��� 
			{
				filelistx=filelist_search(flistbox->list,flistbox->selindex);//�õ���ʱѡ�е�list����Ϣ
				if(filelistx->type==FICO_DISK)//�Ѿ�������������,����������
				{				 
					currentMenu = main_interface; //����ǰ����ָ����������
					break;
				}else filelistbox_back(flistbox);//�˻���һ��Ŀ¼	 
			}				
		}
		if(res_key==KEY0_PRESS) //�������˵�
		{
			res_key = 0;
			currentMenu = main_interface; //����ֱ�ӽ���ǰ����ָ���������漴��ʵ�ַ���������Ĺ���
			break;
		}
	}
	filelistbox_delete(flistbox);	//ɾ��filelist���ͷ�filelistռ���ڴ�
	btn_delete(rbtn);				//ɾ����ť���ͷŰ�ťռ���ڴ� 	 	   	
	return rval; 	
}	


//��������Ļ�����
//����ֵ��1����������Ļ�κ�����0��δ������Ļ
uint8_t tp_touched(void)
{
	u8 res=0;
	tp_dev.scan(0);    
	if(tp_dev.sta&TP_PRES_DOWN)//�а�������
	{ 
     res = 1;
	} 
	while(res)//�ȴ������ɿ�
	{
		tp_dev.scan(0);  
		if((tp_dev.sta&TP_PRES_DOWN)==0)break;
		delay_ms(5);
	} 
	return res;
}

//�������Ŀ¼������ʾ���е�gcode�ļ���ͼƬ�ļ���
//˫����gcode�ļ��������ӡ����˫����ͼƬ�ļ�����ʾͼƬ���ݣ�������Ļ��������ء�
//�����ú���Ŀ�ģ��û��ڴ�ӡģ��ʱֻ�д�ӡ�ļ�������֪��ģ����ʲô���ģ���ȷ��Ҫ��Ҫ��ӡ��
//���˸ú����û����Խ���ӡ�ļ���ģ��ͼƬ����һ������ں����ӡģ��ʱ�Ͳ�����ôãȻ��
//����ֵ��δ�õ�
uint8_t GcoPicFile_Browse(void)
{			
	//******************************��ʼ�������ڲ�����******************************//	 	   
	u8 res_touch;   //�������İ���
	u8 res_key;     //�����İ���
	u8 res;         //��¼�����а��µİ�����1��ȷ�ϼ���2�����ؼ�
	u8 rval=0;			//
 	_btn_obj* rbtn;		//���ذ�ť�ؼ�
  _filelistbox_obj * flistbox;  //�ļ���
	_filelistbox_list * filelistx; 	//�ļ� 
	
//	u8 selx=0XFF;	
	u8 *pname=0;
	u8 filetype=0;	 //�ļ�����
	u8 gcode_flag=0;   //gcode�ļ���־
	u8 picture_flag=0; //ͼƬ�ļ���־
// 	_filelistbox_list * filelistx;	
	
  //******************************�����ļ��б�******************************//
	app_filebrower((u8*)"����",0X07);//�����ļ��������
  flistbox = filelistbox_creat(0,gui_phy.tbheight,lcddev.width,lcddev.height-gui_phy.tbheight*2,1,gui_phy.listfsize);//����һ��filelistbox
	if(flistbox==NULL)rval=1;							//�����ڴ�ʧ��.
	else  
	{
		//������ʾ������չ�����ļ�������filelistbox.h�ļ��в鿴��������Ϊ0xff�����ʾ�������͵��ļ�������������GPIO����
		flistbox->fliter=FLBOX_FLT_GCODE|FLBOX_FLT_PICTURE;     	//����ͼƬ�ļ�
		filelistbox_add_disk(flistbox);					//��Ӵ���·��(��filelist����ǰ,����ִ��һ�θú���)
 		filelistbox_draw_listbox(flistbox);     //�����ļ��б�
	} 	
	//******************************�������ذ�ť******************************//
	rbtn=btn_creat(lcddev.width-2*gui_phy.tbfsize-8-1,lcddev.height-gui_phy.tbheight,2*gui_phy.tbfsize+8,gui_phy.tbheight-1,0,0x03);//�������ְ�ť	
	if(rbtn==NULL)rval=1;	//û���㹻�ڴ湻����
	else
	{																				
	 	rbtn->caption=(u8*)"����";//���� 
	 	rbtn->font=gui_phy.tbfsize;//�����µ������С	 	 
		rbtn->bcfdcolor=RED;	//����ʱ����ɫ
		rbtn->bcfucolor=WHITE;	//�ɿ�ʱ����ɫ
		btn_draw(rbtn);//����ť
	}
	//******************************������Ϣ����******************************//
   	while(rval==0)
	{
		tp_dev.scan(0) ;    
		in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);	//�õ�������ֵ   
		delay_ms(5); //����
		filelistbox_check(flistbox,&in_obj);    //ɨ���ļ���������ļ��еĻ�����ļ���
		if(flistbox->dbclick==0X81)//˫���ļ���
		{
				flistbox->dbclick =0X00; //���˫���ļ���״̬//���÷��ļ����״̬
				filelistx=filelist_search(flistbox->list,flistbox->selindex);//�õ���ʱѡ�е�list����Ϣ
				//strcpy((char *)card.filename,(const char*)filelistx->name);
				pname=gui_memin_malloc(strlen((const char*)filelistx->name)+strlen((const char*)flistbox->path)+2); //Ϊ��·���ļ��������ڴ�
				if(pname==NULL)break;	//����ʧ��
				pname=gui_path_name(pname,flistbox->path,filelistx->name); //��ȡ��·���ļ���
				filetype = f_typetell(pname); //��ȡ�ļ�����
				if(filetype==T_GCODE)//gcode�ļ�
				{
					gcode_flag = 1;    //gcode��־λ��λ
			  }
				else if(filetype==T_GIF|T_JPEG|T_JPG|T_BMP)  //ͼƬ��ʽ�ļ�
				{
					picture_flag = 1;  //ͼƬ��־λ��λ
				}
				else break;
       if(gcode_flag == 1)   //��gcode�ļ��Ĵ���
			 {
				 	gcode_flag = 0; //���gcode��־λ
				  res = window_msg_box((lcddev.width-200)/2,(lcddev.height-80)/2,200,80,"",(u8*)"�Ƿ��ӡ",12,0,0X03,0); //�������Ƿ��ӡ������
					if(res==1) //�����˵�����ȷ�ϼ�
					{
						if(pname)  //��·�����ļ���pname�ǿ�
						{	
							//pname = gui_path_name(pname,flistbox->path,filelistx->name); //��ȡ��·�����ļ���
							rval=f_open(&card.fgcode,(const TCHAR*)pname,FA_READ); //�򿪵�ǰ�ļ�								
							gui_memin_free(pname); //�ͷ�pname���ڴ�
							if (rval==FR_OK) //�ļ��򿪳ɹ�
							{
								currentMenu = print_interface; //����ǰ�˵�ָ�򵽴�ӡ����
								//filelistbox_delete(flistbox);  //ɾ���ļ��б��ͷ��ڴ�	//break������������ִ�и���䣬��������ʡ��											
								card_startFileprint(); //����SD���ļ���ӡ
								//starttime=millis();
							}
						}	
						break; //����ѭ��
					}
					else
					{
							filelistbox_draw_listbox(flistbox); //�ػ����ļ��б���������м��ж��Ƿ��ӡ�Ĵ��岻����ʧ������ֻ֪���������Խ���������б�Ҫ�����Ż�
							//break;	//���ⲻ����break��䣬����break������������������䣬�����ֱ���������̽��棬������о�������			
					}
			 }
       if(picture_flag==1)	//��ͼƬ�ļ����д���		 
			 {
				 picture_flag=0;//���ͼƬ��־λ 
				 LCD_Clear(0x00);//���� 
					if(filetype==T_GIF) //ͼƬ�ļ�ΪGIF��ʽ
						gui_show_string(pname,5,5,lcddev.width-5,gui_phy.tbfsize,gui_phy.tbfsize,RED);	//��ʾGIF����
					ai_load_picfile(pname,0,0,lcddev.width,lcddev.height,1);			//�������ͼƬ
					if(filetype!=T_GIF)  //������ʽ��ͼƬ�ļ�
						gui_show_string(pname,5,5,lcddev.width-5,gui_phy.tbfsize,gui_phy.tbfsize,RED);	//��ʾͼƬ����						  
					//gui_memin_free(pname);			//�ͷ��ڴ� 
					//gui_show_string("����������ء�����",5,lcddev.height-40,lcddev.width-5,gui_phy.tbfsize,gui_phy.tbfsize,RED);	//��ʾGIF����
					gui_show_strmid(0,100,lcddev.width,lcddev.height,RED,16,(u8 *)"����������ء�����");	//��ʾ��������˳�
					while(1)//�ȴ����ⴥ������������
					{
						if(tp_touched())//������Ļ����λ��
						{ 
							//flistbox->dbclick =0X00;
							app_filebrower((u8*)APP_MFUNS_CAPTION_TBL[1][gui_phy.language],0X07);//ѡ��Ŀ���ļ�,���õ�Ŀ������
							btn_draw(rbtn);			//����ť
							filelistbox_rebuild_filelist(flistbox);//�ؽ�flistbox
							break; //����ѭ��
						}
					}				
			 }
		 }	
		res_touch = btn_check(rbtn,&in_obj);		//ɨ�践�ش�������Ť�����
		res_key = KEY_Scan(0);                  //���ذ���ɨ��Ľ��
		if(res_touch==1)  //������Ļ���ذ�����Ϣ
		{
			if(((rbtn->sta&0X80)==0))//��ť״̬�ı��� 
			{
				filelistx=filelist_search(flistbox->list,flistbox->selindex);//�õ���ʱѡ�е�list����Ϣ
				if(filelistx->type==FICO_DISK)//�Ѿ�������������,����������
				{				 
					currentMenu = main_interface; //����ǰ����ָ����������
					break;
				}else filelistbox_back(flistbox);//�˻���һ��Ŀ¼	 
			}				
		}
		if(res_key==KEY0_PRESS) //�������˵�
		{
			res_key = 0;
			currentMenu = main_interface; //����ֱ�ӽ���ǰ����ָ���������漴��ʵ�ַ���������Ĺ���
			break;
		}
	}
	gui_memin_free(pname);			//�ͷ��ڴ�
	filelistbox_delete(flistbox);	//ɾ��filelist���ͷ�filelistռ���ڴ�
	btn_delete(rbtn);				//ɾ����ť���ͷŰ�ťռ���ڴ� 	 	   	
	return rval; 	
}	


