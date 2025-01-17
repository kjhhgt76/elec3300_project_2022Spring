/******************************************************************************
指纹模块设计
Central.LV
2017.11.8
******************************************************************************/
#include "led.h"
#include "delay.h"
#include "sys.h"
#include "oled.h" 
#include "RS485.h"
#include "relay.h"
#include "timer.h"
#include "as608.h"
#include "usart2.h"
//

#define usart2_baund  57600 //
SysPara AS608Para;          //
u16 ValidN;                 //


void Add_FR(void);	//添加指纹
void Del_FR(void);	//删除指纹
void press_FR(void);//比对指纹

/*************************************************************************************
DESC:在OLED上显示一些必要的数据
**************************************************************************************/
void DIS_OLED(int date_to_display,unsigned x, unsigned y)
{
   unsigned char dis_buf[16];
	 if(date_to_display>=0)
	 {
		 dis_buf[0]=' ';
	 }
	 if(date_to_display<0)
	 {
	   dis_buf[0]='-';
		 date_to_display=-date_to_display;
	 }
	 dis_buf[1]=date_to_display/1000+0x30;
	 dis_buf[2]=date_to_display%1000/100+0x30;
	 dis_buf[3]=date_to_display%100/10+0x30;
	 dis_buf[4]=date_to_display%10+0x30;
	 dis_buf[5]=' ';
	 dis_buf[6]=' ';
	 dis_buf[7]=' ';
	 dis_buf[8]=' ';
	 dis_buf[9]=' ';
	 dis_buf[10]=' ';
	 dis_buf[11]=' ';
	 dis_buf[12]=' ';
	 dis_buf[13]=' ';
	 dis_buf[14]=' ';
	 dis_buf[15]=' ';
	 OLED_ShowStr(x,y,(unsigned char*)dis_buf,2);
}	
/****************************************************************************
DESC:按键与LED
*****************************************************************************/
void KEY_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
  	
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
   	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9; 
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz; 
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	  GPIO_Init(GPIOB, &GPIO_InitStructure);
}
/****************************************************************************
DESC:MAIN FUNCATION
*****************************************************************************/
 int main(void)
 {
	char *str;
	u8 ensure; 
	KEY_Init();            //按键初始化
 	SystemInit(); 			   //系统时钟初始化为72M	  SYSCLK_FREQ_72MHz
	delay_init(72);	    	 //延时函数初始化	  
	NVIC_Configuration();  //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
 	LED_Init();			       //LED端口初始化
	I2C1_init();           //I2C1总线初始化
	OLED_Init();           //OLED初始化  //初始化显示
	OLED_ShowStr(0,0,(unsigned char*)" FINGERPRINT ID ",2);  
	OLED_ShowStr(0,2,(unsigned char*)"                ",2);
	OLED_ShowStr(0,4,(unsigned char*)"                ",2);
  OLED_ShowStr(0,6,(unsigned char*)"                ",2);	
	RS485_Init(9600);	   //初始化RS485
	RELAY_Init();
	usart2_init(usart2_baund);//初始化串口2，用来和指纹模块通信	
	PS_StaGPIO_Init();        //初始化指纹模块IO口
	 
	//与指纹模块握手
	while(PS_HandShake(&AS608Addr))
	{
	  OLED_ShowStr(0,2,(unsigned char*)" ShakHands....",2);
	}
	//握手通过  
	OLED_ShowStr(0,2,(unsigned char*)" Ready To Work ",2);
	
	delay_ms(100);

	
	while(1)
	{
		
		press_FR();
		//添加指纹
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)==0)
		   {	   
		     delay_ms(10);
			    if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)==0)
			     {
			       //setup=setup+2000;
						 Add_FR();
						 OLED_ShowStr(0,4,(unsigned char*)" FUN:ADD FRIGER ",2);
			     }	
		   }
			//对比指纹
				 if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)==0)
		   {	   
		     delay_ms(10);
			    if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)==0)
			     {
			       Del_FR();
			
						 OLED_ShowStr(0,4,(unsigned char*)" FUN:DEL FRIGER ",2);
			     }	
		   }
		
		
		}
 }
//////////////////////////////////////////////////////////////////////////

/*************************************************************************
 DESC:显示错误信息
************************************************************************/
void ShowErrMessage(u8 ensure)
{

}
/***********************************************************************
DESC:添加指纹
************************************************************************/
void Add_FR(void)
{
	u8 i=0,ensure ,processnum=0;
	u16 ID;
	while(1)
	{
		switch (processnum)
		{
			case 0:
				i++;
				
			  OLED_ShowStr(0,6,(unsigned char*)" Touch finger!  ",2);
			  OLED_ShowStr(0,4,(unsigned char*)" FUN:ADD FRIGER ",2);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					ensure=PS_GenChar(CharBuffer1);//????
					if(ensure==0x00)
					{
						
						OLED_ShowStr(0,6,(unsigned char*)" Figer Correct  ",2);
						OLED_ShowStr(0,4,(unsigned char*)" FUN:ADD FRIGER ",2);
						i=0;
						processnum=1;					
					}else ShowErrMessage(ensure);				
				}else ShowErrMessage(ensure);						
				break;
			
			case 1:
				i++;
			
			  OLED_ShowStr(0,6,(unsigned char*)" Touch Again!   ",2);
			  OLED_ShowStr(0,4,(unsigned char*)" FUN:ADD FRIGER ",2);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					ensure=PS_GenChar(CharBuffer2);//			
					if(ensure==0x00)
					{
						
						OLED_ShowStr(0,6,(unsigned char*)" Figer Correct  ",2);
						OLED_ShowStr(0,4,(unsigned char*)" FUN:ADD FRIGER ",2);
						i=0;
						processnum=2;
					}else ShowErrMessage(ensure);	
				}else ShowErrMessage(ensure);		
				break;

			case 2:
				
			  OLED_ShowStr(0,6,(unsigned char*)" Figer Compare  ",2);
				OLED_ShowStr(0,4,(unsigned char*)" FUN:ADD FRIGER ",2);
				ensure=PS_Match();
				if(ensure==0x00) 
				{

					OLED_ShowStr(0,6,(unsigned char*)" Figer are same ",2);
					OLED_ShowStr(0,4,(unsigned char*)" FUN:ADD FRIGER ",2);
					processnum=3;
				}
				else 
				{
					
					ShowErrMessage(ensure);
					i=0;
					processnum=0;		
				}
				delay_ms(1000);
				break;

			case 3:

			  OLED_ShowStr(0,6,(unsigned char*)" Touch Again!   ",2);
			  OLED_ShowStr(0,4,(unsigned char*)" FUN:ADD FRIGER ",2);
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{

					OLED_ShowStr(0,6,(unsigned char*)" Comper Success!",2);
			    OLED_ShowStr(0,4,(unsigned char*)" FUN:ADD FRIGER ",2);
					processnum=4;//?????
				}else {processnum=0;ShowErrMessage(ensure);}
				delay_ms(1000);
				break;
				
			case 4:	
					
				do
					ID=1;
				while(!(ID<300));
				ensure=PS_StoreChar(CharBuffer2,ID);
				if(ensure==0x00) 
				{
          OLED_ShowStr(0,6,(unsigned char*)"  ADD Success!  ",2);
			    OLED_ShowStr(0,4,(unsigned char*)" FUN:ADD FRIGER ",2);					
					PS_ValidTempleteNum(&ValidN);
					delay_ms(1500);
					
					return ;
				}else {processnum=0;ShowErrMessage(ensure);}					
				break;				
		}
		delay_ms(800);
		if(i==5)//
		{
		
			break;	
		}				
	}
}

/**************************************************************
DESC:对比指纹
*****************************************************************/
void press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	OLED_ShowStr(0,6,(unsigned char*)" Begin Compare ",2);
	ensure=PS_GetImage();
	if(ensure==0x00)// 
	{	
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //
		{		
			ensure=PS_HighSpeedSearch(CharBuffer1,0,300,&seach);
			if(ensure==0x00)//
			{				
				OLED_ShowStr(0,6,(unsigned char*)" Comper PASS!   ",2);
			  OLED_ShowStr(0,4,(unsigned char*)" FUN:CPR FRIGER ",2);
			}
			else 
			{
			    OLED_ShowStr(0,6,(unsigned char*)" Comper Fail!   ",2);
			    OLED_ShowStr(0,4,(unsigned char*)" FUN:CPR FRIGER ",2);
			}	
			
		}
			else 
			{
			    OLED_ShowStr(0,6,(unsigned char*)" Comper Fail!   ",2);
			    OLED_ShowStr(0,4,(unsigned char*)" FUN:CPR FRIGER ",2);
			}	
		
	 delay_ms(1000);//

	}
		
}
/******************************************************************************
DESC:删除指纹
*******************************************************************************/

void Del_FR(void)
{
	u8  ensure;
//	u16 num;OLED_ShowStr(0,6,(unsigned char*)" FINGER DELL OK ",2);
	//LCD_Fill(0,100,lcddev.width,160,WHITE);
	//Show_Str_Mid(0,100,"Delete fingerprint",16,240);//??????
	//Show_Str_Mid(0,120,"Input ID and touch Enter!",16,240);//????ID???揈nter�
	//Show_Str_Mid(0,140,"0=< ID <=299",16,240);
	delay_ms(50);
	//AS608_load_keyboard(0,170,(u8**)kbd_delFR);
	//num=GET_NUM();//???????
	//if(num==0xFFFF)
		//goto MENU ; //?????
	//else if(num==0xFF00)
		ensure=PS_Empty();//?????
	//else 
		//ensure=PS_DeletChar(num,1);//??????
	if(ensure==0)
	{
		OLED_ShowStr(0,6,(unsigned char*)" FINGER DELL OK ",2);
	  OLED_ShowStr(0,4,(unsigned char*)" FUN:DEL FRIGER ",2);
		//LCD_Fill(0,120,lcddev.width,160,WHITE);
		//Show_Str_Mid(0,140,"Delete fingerprint success!!!",16,240);//??????		
	}
  //else
		//ShowErrMessage(ensure);	
	delay_ms(1500);//???????
	//PS_ValidTempleteNum(&ValidN);//??????
	//LCD_ShowNum(80,80,AS608Para.PS_max-ValidN,3,16);//????????
//MENU:	
	//LCD_Fill(0,100,lcddev.width,160,WHITE);
	//delay_ms(50);
	//AS608_load_keyboard(0,170,(u8**)kbd_menu);
}

