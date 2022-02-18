#include "2450addr.h"
#include "option.h"
#include "my_lib.h"

//+++++++++++++ won_add_new_code ++++++++++++++++
#include "./Images/bat_2.h"
#include "./Images/stop_2.h"

//color Define
#define BLACK	0x0000
#define WHITE	0xFFFF
#define BLUE	0x001F
#define GREEN	0x03E0
#define RED		0x7C00

void bet_limit_init(void);
void bet_randomUp_init(void);
void whole_time_init(void);

void bet_limit(int msec);
void bet_randomUp(int msec);
void whole_time(int msec);

void Timer2_ISR(void) __attribute__ ((interrupt ("IRQ")));
volatile int betSecond;
volatile int betSecond_end;
volatile int raiseSecond;
volatile int raiseSecond_end;
volatile int whSecond;
volatile int whSecond_end;
extern int raiseSecond_end_reset;

void Timer2_ISR()
{
	if(whSecond >= 200){
		whSecond = 0;
		whSecond_end = 0;
		betSecond = 0;
		betSecond_end = 0;
		raiseSecond = 0;
		raiseSecond_end = 0;
	}
	//Uart_Printf("Timer2_ISR\n");
	rINTMSK1 |= (1<<12);	// 인터럽트 중복 방지용
	
	rSRCPND1 |= (0xffffffff); //인터럽트 요청 상태 
	rINTPND1 |= (0xffffffff);	//인터럽트 요청 상태 

	whSecond = whSecond + 1;
	if(whSecond <= 100){
		betSecond = betSecond + 1;
	}else{
		betSecond_end = 1;
	}
	if(whSecond <= 150){
		raiseSecond = raiseSecond + 1;
	}else{
		raiseSecond_end = 1;
	}
	if(whSecond <= 200){
		whSecond_end = 1;
	}
	//Uart_Printf("betSecond   :%d second\n",betSecond);
	//Uart_Printf("raiseSecond :%d second\n",raiseSecond);
	//Uart_Printf("whSecond    :%d second\n",whSecond);
		Lcd_Select_Frame_Buffer(0);
		Lcd_Printf(384,10, 0xFFFF, 0x7C00, 1,1,"     %d     ",100 - (betSecond));

	if(betSecond_end){ //Betting Time is ended
		Lcd_Select_Frame_Buffer(0);
		Lcd_Draw_BMP(360, 136, stop_2);
		Uart_Printf("betSecond_ended %d\n",betSecond_end);
	}else{
		Lcd_Draw_BMP(360, 136, bat_2);
	}
	rINTMSK1 &= ~(1<<12);			// 다시 인터럽트 온 
}

void bet_limit_init(void)
{ //Timer 0
/*		rTCFG0|=0xffff;
		rTCFG1=0x3;
		rTCON=(0x1<<3); //[3] = Determine auto reload on/off for Timer 0.
		//rTCON &= ~(0x0<<3);
		rTCNTB0 = 0;
		rTCMPB0 = 0;
		*/
}

void bet_randomUp_init(void)
{//Timer 1
/*		rTCFG0|=0xffff;
		rTCFG1=(0x2<<4);
		rTCON=(0x1<<11);  //[11] = Determine the auto reload on/off for Timer1. 
		//rTCON &= ~(0x0<<11);
		rTCNTB1 = 0; 
		rTCMPB1 = 0;
		*/
}
void whole_time_init(void)
{//Timer 2
		if((whSecond >= 20) || (raiseSecond_end_reset == 1)){
		whSecond = 0;
		whSecond_end = 0;
		betSecond = 0;
		betSecond_end = 0;
		raiseSecond = 0;
		raiseSecond_end = 0;
		}
		rTCFG0|=0xffff;
		rTCFG1=(0x3<<8);
		rTCON |= (0x1<<15);  //[15] = Determine auto reload on/off for Timer 2. 
		rTCNTB2 = 0; //Timer 2 buffer 
		rTCMPB2 = 0; //Timer 2 buffer
		pISR_TIMER2 = (unsigned int)Timer2_ISR;  
		rINTMSK1 &= ~(1<<12);
		//[10] 을 0으로 한다. 1 = mask , 0 = available
}


/*
void bet_limit(int msec)
{
	rTCNTB0 = 16.113*msec;
*/
/* Timer0 에 대한 
[0] = 타이머 스타트 스탑 결정					1 = start
[1] = 수동 업데이트 할지 결정 				1 = Update TCNTB0 & TCMPB0
[2] = Output 인버터를 on/off 할지 	1 = 인버터 on for TOUT0
[3] = 자동 사이클로 돌릴지 					1 = 무한 사이클 
while(rTCNTO0);	 <- Timer0가 0이 될ㅤㄸㅒㅤ까지 기다려 주는 코드 
*/
/*
	rTCON |= (1<<1) | (0); 
	rTCON &= ~(1<<1);
	rTCON |= (1<<0);
	//while(rTCNTO0){};
	//Uart_Printf("bet_limit rTCNTB0 %d\n",rTCNTB0);
	//Uart_Printf("bet_limit msec %d\n",msec);
	rTCON &= ~(1<<0);
}
void bet_randomUp(int msec){
	rTCNTB1 = 16.113*msec;
	rTCON |= (1<<9) | (0<<8);
	rTCON &= ~(1<<9);
	rTCON |= (1<<8);
	//while(rTCNTO1);
	//Uart_Printf("bet_randomUp rTCNTB1 %d\n",rTCNTB1);
	//Uart_Printf("bet_randomUp msec %d\n",msec);
	rTCON &= ~(1<<8);
}
*/
void whole_time(int msec){
	//rTCNTB2 = 16.113*msec;
	rTCNTB2 = 16.113*msec;
	rTCON |= (1<<13) | (0<<12);
	rTCON &= ~(1<<13);
	rTCON |= (1<<12);
	while(rTCNTO2);
	//Uart_Printf("whole_time rTCNTB2 %d\n",rTCNTB2);
	//Uart_Printf("whole_time msec %d\n",msec);
	rTCON &= ~(1<<12);
}

//+++++++++++++ won_add_new_code ++++++++++++++++



void Timer0_Init(void)
{
	/* 
	* 	Timer0 Init 
	* Prescaler value : 255, dead zone length = 0
	* Divider value : 1/16, no DMA mode
	* New frequency : (PCLK/(Prescaler value+1))*Divider value = (66Mhz/(256))*(1/16)
	*				= 16.113Khz(16113Hz)
	*/
	rTCFG0 = (0<<8)|(0xff); 
	rTCFG1 = (0<<20)|(3); 
	
	/* TCON설정 :Dead zone disable,  auto reload on, output inverter off
	*  manual update no operation, timer0 stop, TCNTB0=0, TCMPB0 =0
	*/
	rTCON  = (0<<4)|(0<<3)|(0<<2)|(0<<1)|(0);
	rTCNTB0 = 0;
	rTCMPB0 = 0;
 
}

void Timer0_Delay(int msec)
{
	/*
	* 1) TCNTB0설정 : 넘겨받는 data의 단위는 msec이다.
	*                  따라서 msec가 그대로 TCNTB0값으로 설정될 수는 없다.
	* 2) manual update후에  timer0를 start시킨다. 
	* 	 note : The bit has to be cleared at next writing.
	* 3) TCNTO0값이 0이 될때까지 기다린다. 	
	*/
	rTCNTB0 = 16.113*msec;	

	rTCON |= (1<<1)|(0);
	rTCON &= ~(1<<1);
	
	rTCON |= 1;	
	
	while(rTCNTO0 != 0);
	rTCON &=~ 1;	
}



