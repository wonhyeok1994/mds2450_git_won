/*
 * =====================================================================
 * NAME         : Main.c
 *
 * Descriptions : Main routine for S3C2450
 *
 * IDE          : GCC-4.1.0
 *
 * Modification
 *	  
 * =====================================================================
 */
#include <stdlib.h>
#include <time.h>
#include "2450addr.h"
#include "my_lib.h"
#include "option.h"
#include "macro.h"
// images==================================================
#include "./Images/realui.h"
#include "./Images/Title1.h"
#include "./Images/bat.h"
#include "./Images/stop.h"
#include "./Images/howtoplay.h"

//color Define
#define BLACK	0x0000
#define WHITE	0xFFFF
#define BLUE	0x001F
#define GREEN	0x03E0
#define RED		0x7C00

#define  NonPal_Fb   ((volatile unsigned short(*)[480]) FRAME_BUFFER)

// ISR Functions declaration
#define LCD_XSIZE 		(480)	
#define LCD_YSIZE 		(272)

//+++++++++++++ won_add_new_code ++++++++++++++++
extern int whSecond;
extern int betSecond;
extern int betSecond_end;
extern int raiseSecond;
extern int raiseSecond_end;
extern int whSecond;
extern int whSecond_end;
volatile int raiseSecond_end_reset;

//**ISR Declaration
void Touch_ISR(void) __attribute__ ((interrupt ("IRQ")));
//+++++++++++++ won_add_new_code ++++++++++++++++

//============================================================
// Functions Declaration
//============================================================

void HW_Initial(void);


//============================================================
// Global Variables Declaration
//============================================================

// ÅÍÄ¡ÆÐµå°¡ ´­¸²À» ¾Ë¼ö ÀÖ´Â °ª
volatile int Touch_pressed = 0;
// ADC °ª
volatile int ADC_x=0, ADC_y=0;
// Calibration Á¤º¸ ÀúÀå °ª
volatile int Cal_x1=848;
volatile int Cal_y1=656;
volatile int Cal_x2=186;
volatile int Cal_y2=349; 
// ÁÂÇ¥ º¯È¯ °ª
volatile int Touch_x, Touch_y;
// Calibration ¿Ï·á °ª
volatile unsigned int Touch_config=1;


// ========================================================
//extern unsigned int HandleTIMER0;
// ========================================================

void Touch_ISR(void) __attribute__ ((interrupt ("IRQ")));
// touch interrupt
	int step = 0;
void Touch_ISR()
{
	rINTSUBMSK |= (0x1<<9);
	rINTMSK1 |= (0x1<<31);	
	
	/* TO DO: Pendng Clear on Touch */	
	rSUBSRCPND |= (0x1<<9);
	rSRCPND1 |= (0x1<<31);
	rINTPND1 |= (0x1<<31);
	
	// Touch UP
	if(rADCTSC&0x100)
	{
		rADCTSC&=0xff;
		Touch_pressed = 0;
	}
	// Touch Down
	else 
	{
		rADCTSC=(0<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(1<<3)|(1<<2)|(0);
		// SADC_ylus Down,Don't care,Don't care,Don't care,Don't care,XP pullup Dis,Auto,No operation
		rADCCON|=0x1;
		while(rADCCON & 0x1);
		while(!(0x8000&rADCCON));
		ADC_x=(int)(0x3ff&rADCDAT0);
		ADC_y=(int)(0x3ff&rADCDAT1);
		// Touch calibration complete
		if(Touch_config)
		{
			Touch_y=(ADC_y-Cal_y1)*(LCD_YSIZE-10)/(Cal_y2-Cal_y1)+5;
			Touch_x=(ADC_x-Cal_x2)*(LCD_XSIZE-10)/(Cal_x1-Cal_x2)+5;
			Touch_x=LCD_XSIZE-Touch_x;
			if(Touch_x<0) Touch_x=0;
			if(Touch_x>=LCD_XSIZE) Touch_x=LCD_XSIZE-1;
			if(Touch_y<0) Touch_y=0;
			if(Touch_y>=LCD_YSIZE) Touch_y=LCD_YSIZE-1;
		}
		// before calibration		
		else
		{
			Touch_x = ADC_x;
			Touch_y = ADC_y;
		}
		rADCTSC=(1<<8)|(1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(0<<2)|(3);
		// SADC_ylus Up,Don't care,Don't care,Don't care,Don't care,XP pullup En,Normal,Waiting mode 
		Touch_pressed = 1;
	}

	rINTSUBMSK &= ~(0x1<<9);
	rINTMSK1 &= ~(0x1<<31);
}

// ========================================================
// Lower Layer Functions
// ========================================================

void HW_Initial(void) // Hardware Init
{
	//MMU_Init();
	Exception_Init();	
	Timer0_Init();
	Graphic_Init(); // Lcd init
	//Touch_Init();
	Touch_Isr_Init(Touch_ISR);
	MMU_Init();

	//pISR_ADC = (unsigned int)Touch_ISR;
}
/*---------------------------------------------------*/
/*---------------------------------------------------

void Show_Welcome(char * msg)
{
	int a, j; 
	
	Uart_Printf("\n%s\n", msg);
	
	for(a=0;a<2;a++)
	{
		Led_Display(7);
		for(j=0; j<0x0ffff; j++);
		Led_Display(0);
		for(j=0; j<0x0ffff; j++);
	}
}


void RTC_Init(void)
{
	rRTCCON |= (0x1); 
	
	rRTCCON |= (0x1<<3);
	rRTCCON &= ~(0x1<<3);
	
	rBCDMIN = 0x0;
	rBCDSEC = 0x0;	   
	
	//rRTCCON &= ~(0x1);
}	*/

//===============================================================
// main~~~
//===============================================================
void Main(void) 
{
	Uart_Init(115200);
	HW_Initial();
	Uart_Printf("Complete Init!\n");
	
	int have_money = 50;
	Lcd_Clr_Screen(0x0000);  ///////////////////////////////////black screen
	Timer0_Delay(1000);
	// Show Title for 3sec
	Lcd_Draw_BMP(0,0, Title1); // Show Title
	Timer0_Delay(3000);		     // Title delay
	Lcd_Draw_BMP(0,0, howtoplay); // Show howtoplay
	gamesong();
	Timer0_Delay(8000);		     // howtoplay delay
	srand(time(NULL));

  while(1) // start game
  {	
  	//int choice_bat;
		int bat_money = 0;
		//int have_money = 100;
		//int mul_num = rand() % 20 + 1; // 1 ~ 20 make random num
		//srand(time(NULL));
		//double mul_nums[10] = {};
		//mul_nums[1] = double(mul_num);
		int earn_money = 0;
		double i;

//+++++++++++++ won_add_new_code ++++++++++++++++
		//bet_limit_init();
		//bet_randomUp_init();
		raiseSecond_end_reset = 1;
		whole_time_init();
		raiseSecond_end_reset = 0;	
		//Uart_Send_String("bet_limit\n");
		//bet_limit(1000);
		//Uart_Send_String("bet_randomUp\n");
		//bet_randomUp(1000);
		//whole_time(1000);
		//Uart_Send_String("whole_time\n");
//+++++++++++++ won_add_new_code ++++++++++++++++

		//int earn_money = 0;
		// Show Game screen
		Lcd_Select_Frame_Buffer(1); 
		Lcd_Draw_BMP(0,0, realui);// Show UI
		// Show time, have_money, bat_money
		Lcd_Printf(384,10, 0xFFFF, 0x7C00, 1,1," 00 ");
		Lcd_Printf(370,35, 0xFFFF, 0x0000, 1,1,"HAVE : %d $", have_money);
		Lcd_Printf(370,60, 0xFFFF, 0x0000, 1,1,"BAT : %d $", bat_money);
		Lcd_Draw_BMP(360,136, bat);

		Lcd_Copy(1,0);
		Lcd_Display_Frame_Buffer(0);
  	Uart_Printf("Complete Lcd!\n");

  	Touch_ISR_Enable(1);
		Uart_Printf("touch enable!\n");
    //==========================================================
  	// choice batting money 
  	//===========================================================
  	Uart_Printf("game Init!\n");
  	// bat mode
//+++++++++++++ won_add_new_code ++++++++++++++++
  	if(!(betSecond_end))
		{
			raiseSecond_end_reset = 0;
	  	while(1)
			{
				whole_time(100);
				//Uart_Printf("betSecond %d ",11-betSecond);
				if(betSecond_end){break;}
//+++++++++++++ won_add_new_code ++++++++++++++++
				if(Touch_pressed)  // 10 touch
				{	
					Uart_Printf("X : %d, Y: %d \n", Touch_x, Touch_y);	
					// 10 touch========================================================
					if(Touch_x>360 &&Touch_x<400&& Touch_y>85 && Touch_y<145)
					{	
						touchsound10();
						if(have_money-10 >= 0)
						{
							have_money -= 10;
							bat_money += 10;
						}
						Lcd_Select_Frame_Buffer(0);
						Lcd_Printf(370,35, 0xFFFF, 0x0000, 1,1,"HAVE : %d $", have_money);
						Lcd_Printf(370,60, 0xFFFF, 0x0000, 1,1,"BAT : %d $", bat_money);
						Uart_Printf("%d\n", have_money);
						Uart_Printf("%d\n", bat_money);
						Touch_pressed = 0;
					}	
					// All touch ===================================================
					else if(Touch_x>400 &&Touch_x<440&& Touch_y>85 && Touch_y<145)
					{	
						touchsoundAll();
						bat_money = have_money;
						have_money = 0;
						Lcd_Select_Frame_Buffer(0);
						Lcd_Printf(370,35, 0xFFFF, 0x0000, 1,1,"HAVE : %d $", have_money);
						Lcd_Printf(370,60, 0xFFFF, 0x0000, 1,1,"BAT : %d $", bat_money);
						Uart_Printf("%d\n", have_money);
						Uart_Printf("%d\n", bat_money);
						Touch_pressed = 0;

						//Lcd_Printf(170,360, WHITE, WHITE, 2,2,"HAVE : %d $", have_money);
						//Lcd_Printf(130,360, WHITE, WHITE, 2,2,"BAT : %d $", bat_money);
					}	
					// REMOVE===========================================================
					else if(Touch_x>440 &&Touch_x<480&& Touch_y>85 && Touch_y<145)
					{	
						touchsound0();
						have_money += bat_money;
						bat_money = 0;
						Lcd_Select_Frame_Buffer(0);
						Lcd_Printf(370,35, 0xFFFF, 0x0000, 1,1,"HAVE : %d $", have_money);
						Lcd_Printf(370,60, 0xFFFF, 0x0000, 1,1,"BAT : %d $", bat_money);
						Uart_Printf("%d\n", have_money);
						Uart_Printf("%d\n", bat_money);
						Touch_pressed = 0;
					} //remove
					// WHEN TOUCH "BATTING BUTTON" =====================================
					else if(Touch_x>360 &&Touch_x<480&& Touch_y>148 && Touch_y<238)
					{	
						touchsoundbat();
						Lcd_Draw_BMP(360, 136, stop);
						Touch_pressed = 0;
						Uart_Printf("You Batted %d $ !!!\n", bat_money);
						Uart_Printf("Now! Graph Begin Soon!!!\n");
	//+++++++++++++ won_add_new_code ++++++++++++++++
							break;
					} //BATTING BUTTON
				}	 //if pressed
		 	} //2nd while
		}
	// mul_num ///////////////////////////////////recopy please~~~~~
	//srand(time(NULL));
		int mul_num = rand() % 10 + 1; // 1 ~ 20 make random num
		for(i = 0.01; i <= mul_num; i=i+0.01) 
		{   ////////////////////////////////===========int to float=============
			earn_money = i * bat_money;/////////////earnmoney to show
			/////////////////graph!!!!
			Lcd_Put_Pixel(0+(40*i), 238-((17+i)*i), 0x7C00);
			Lcd_Put_Pixel(1+(40*i), 238-((17+i)*i), 0x7C00);
			Lcd_Put_Pixel(2+(40*i), 238-((17+i)*i), 0x7C00);
			Lcd_Put_Pixel(3+(40*i), 238-((17+i)*i), 0x7C00);
			

		
			Lcd_Printf(140, 82, 0xFFFF, 0x0000, 2,2,"x %.2f", i);   //// show mulnum
			Lcd_Printf(140, 116, 0xFFFF, 0x0000, 2,2,"@ %d$", earn_money); ////show earns
			
			Timer0_Delay(1); // mulnum plus speed
			/////if you press stop
			if(Touch_pressed && Touch_x>360 &&Touch_x<480&& Touch_y>148 && Touch_y<238)
			{	
				touchsoundstop();
				have_money += i * bat_money; /////////////you earn money
				break;
				Timer0_Delay(2000); //// show screen for 2sec
			}
		}
	//have_money -= bat_money;		
		Timer0_Delay(2000);
		if(have_money <= 0)  //////////if you lose all your money
		{
			// Lcd_Draw_BMP(0,0, realui);// Show clean UI   remove this
			          ///////=============
			/////black screen
			Lcd_Clr_Screen(0x0000);    //// show black screen
			Lcd_Printf(140, 102, 0xFFFF, 0x0000, 2,2,"YOU ARE LOSER");
			Lcd_Printf(195, 132, 0xFFFF, 0x0000, 2,2,"IDIOT");/////////////////////
			Timer0_Delay(3000);
			have_money = 50;
		}
  } //1st while
} // Main