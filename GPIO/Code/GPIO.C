/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2017 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

//***********************************************************************************************************
//  Website: http://www.nuvoton.com
//  E-Mail : MicroC-8bit@nuvoton.com
//  Date   : Jan/21/2017
//***********************************************************************************************************

//***********************************************************************************************************
//  File Function: N76E003 GPIO demo code
//***********************************************************************************************************
#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "Delay.h"
#include "Music_control.h"

#define USE_LED 0

#if USE_LED
#include "FFT.h"
#endif

#define DEBUG 0
#define HIGH 1
#define LOW 0

#define WAITING 2
#define PLAYING 1
#define STOP 0
#define ON 1
#define OFF 0
#define SOURCE_TF 2
#define SOURCE_FLASH 3
#define MINIT 	60
#define SECOND 1
#define SW_PRESS 1
#define SW_RELEASE 2
#define SW_NONE 0

#define SYS_VOLUME 35
#define AUDIO_CTRL P01
#define DEBUG_LED	P11

/*------------------------------------------------
The main C function.  Program execution starts
here after stack initialization.
------------------------------------------------*/

#define setbit(x,y) x|=(1<<y) //将X的第Y位置1
#define testbit(x,y) x&(1<<y) //测试X的第Y位置
#define clrbit(x,y) x&=!(1<<y) //将X的第Y位清0

int button_trig_state=HIGH;
int play_trig_state=HIGH;
int charge_trig_state=HIGH;

int Button_state=SW_NONE;
int Play_state=STOP;
int Charge_state=OFF;
int Reset_system=-1;
int Reset_audio=-1;
int Power_state=ON;
int charge_type=TYPE_NONE;
void PinInterrupt_ISR (void) interrupt 7
{
	if (testbit(PIF,7))	//SWITCH PIN
	{
		clrbit(PIF,7);
		clr_EPI;							//disable interrupt
		Delay_1ms(1);
		if(button_trig_state==LOW && P17==LOW){
			Enable_BIT7_RasingEdge_Trig;
			button_trig_state=HIGH;
			Button_state=SW_PRESS;
			DEBUG_LED=1;
			clr_PD;
		}else if(button_trig_state==HIGH && P17==HIGH){
			Enable_BIT7_FallEdge_Trig;
			button_trig_state=LOW;
			Button_state=SW_RELEASE;
			DEBUG_LED=0;
			clr_PD;
		}
		set_EPI;							//enable intterrupt
	}
	if (testbit(PIF,2))	//BUSY PIN
	{
		clrbit(PIF,2);
		if(Power_state==OFF)
			return;
		clr_EPI;							//disable interrupt
		Delay_1ms(3);	
		if(play_trig_state==LOW && P12==LOW){
			Enable_BIT2_RasingEdge_Trig;
			play_trig_state=HIGH;
			Play_state=PLAYING;
			DEBUG_LED=1;
		}else if(play_trig_state==HIGH && P12==HIGH){
			Enable_BIT2_FallEdge_Trig;
			play_trig_state=LOW;
			Play_state=STOP;
			DEBUG_LED=0;
		}
		set_EPI;							//enable intterrupt
		//clr_PD;
	}
	if (testbit(PIF,3))	//USB PIN
	{
		clrbit(PIF,3);
		clr_EPI;							//disable interrupt
		Delay_1ms(2);
		if(charge_trig_state==LOW && P13==LOW){
			Enable_BIT3_RasingEdge_Trig;
			charge_trig_state=HIGH;
			Charge_state=OFF;
			Reset_audio=-1;
			Reset_system=1;
			clr_PD;
		}else if(charge_trig_state==HIGH && P13==HIGH){
			Enable_BIT3_FallEdge_Trig;
			charge_trig_state=LOW;
			Charge_state=ON;
			Reset_system=-1;
			Reset_audio=1;
			clr_PD;
		}
		set_EPI;							//enable intterrupt
	}
}

unsigned long wake_time=0;
unsigned long timer_count=0;

/************************************************************************************************************
*    TIMER 0 interrupt subroutine
************************************************************************************************************/
void Timer0_ISR (void) interrupt 1          //interrupt address is 0x000B
{
	clr_TF0;
	clr_TR0;                              		  //Stop Timer0
    TL0 = LOBYTE(TIMER_DIV12_VALUE_40ms); 		//Find  define in "Function_define.h" "TIMER VALUE"
    TH0 = HIBYTE(TIMER_DIV12_VALUE_40ms);
	set_TR0;                              		  //Start Timer0
	timer_count+=40;
	if(timer_count >= 1000){
		timer_count=0;
		wake_time++;
	}
}
void audio_power_on()
{
	AUDIO_CTRL = LOW;
	charge_type = Get_charge_type(16); //25*16=400ms for timeout
	Power_state = ON;
	Send_Data_To_UART0(0xcc);
	Send_Data_To_UART0(charge_type);
}
void audio_power_off(int sleep_flag)
{
	int org_p06=P06;
	int org_p07=P07;
	int org_p03=P03;
	int org_p04=P04;
	int org_p12=P12;
clr_EPI;
	clr_TR0;                              		  //Stop Timer0
	wake_time=0;
	Power_state=OFF;								//Must set to OFF before power down, or will trigger.
	AUDIO_CTRL=HIGH;
	org_p03=P03;
	org_p04=P04;
	org_p06=P06;
	org_p07=P07;
	org_p12=P12;
	P03_Quasi_Mode;
	P04_Quasi_Mode;
	P12_Quasi_Mode;
	P03=0;
	P04=0;
	P06=0;
	P07=0;
	P12=0;
set_EPI;
	if(sleep_flag==1){
		set_PD;										//go to sleep mode
	}
clr_EPI;
	P12=org_p12;
	P12_Input_Mode;
	P03=org_p03;
	P04=org_p04;
	P06=org_p06;
	P07=org_p07;
	P12=org_p12;
	set_TR0;                                    //Timer0 run
set_EPI;
}
void reboot_audio()
{
	audio_power_off(0);
	Delay_1ms(10);
	audio_power_on();
}
// led num range is 0~1024
void LED_R(int num){
	PWM1H = HIBYTE(num);				
	PWM1L = LOBYTE(num);
	PWM1_OUTPUT_INVERSE;
	set_LOAD;
}
void LED_G(int num){
	PWM3H = HIBYTE(num);				
	PWM3L = LOBYTE(num);
	PWM3_OUTPUT_INVERSE;
	set_LOAD;
}
void LED_B(int num){
	PWM2H = HIBYTE(num);				
	PWM2L = LOBYTE(num);
	PWM2_OUTPUT_INVERSE;
	set_LOAD;
}
void init_LED(){
	PWM1_P14_OUTPUT_ENABLE;
	PWM3_P00_OUTPUT_ENABLE;
	PWM2_P05_OUTPUT_ENABLE;

	PWM_IMDEPENDENT_MODE;
	PWM_CLOCK_DIV_8;
	PWMPH = 0x07;
	PWMPL = 0xCF;
	
	LED_R(0);
	LED_G(0);
	LED_B(0);
	set_PWMRUN;
}
//-----------------------------------------------------------------------------------

#if USE_LED
int get_adc(void)
{
	clr_ADCF;
	set_ADCS;									// ADC start trig signal
	while(ADCF == 0);
  	return (int)(ADCRH<<2) + (int)((ADCRL&0x0f)>>2);//(((int)ADCRH)<<4+ADCRL&0x0f);
}
 void ADC_Finish()
{
    int ADC_Count=0;
	Enable_ADC_AIN5;
    while(ADC_Count<=64)
    {
      Fft_Real[LIST_TAB[ADC_Count]]=get_adc()-256; //按LIST_TAB表里的顺序，进行存储 采样值,,
      //  ADC_CONTR = ADC_POWER | ADC_SPEEDHH| ADC_START | channel;   // 为了采集负电压，采用 偏置采集。电压提高到1/2 vcc，，所以要减去256
      ADC_Count++;
    }
}
#endif

int Head_Music_Play=0;
int Body_Music_Play=0;
void main (void) 
{
	int music_num=1;
	int play_state=-1;
	
	//set_PD;									//powerdown directly 131.5uA
	Set_All_GPIO_Quasi_Mode;					// Define in Function_define.h
	DEBUG_LED=0;
	P01_PushPull_Mode;
	AUDIO_CTRL=1;
	Delay_1ms(300);								//Delay 300ms in case download problem
	InitialUART0_Timer1(9600);
	Send_Data_To_UART0(0xaa);
	Send_Data_To_UART0(0xaa);
	set_CLOEN; 
	audio_power_on();

/*-------------------------------Init Timer---------------------------------------------*/
#if 1
	TIMER0_MODE0_ENABLE;                        //Timer 0 and Timer 1 mode configuration
	clr_T0M;
    TMOD |= 0x01;                           		//Timer0 is 16-bit mode
    TL0 = LOBYTE(TIMER_DIV12_VALUE_40ms); 		//Find  define in "Function_define.h" "TIMER VALUE"
    TH0 = HIBYTE(TIMER_DIV12_VALUE_40ms);
	set_ET0;                                    //enable Timer0 interrupt
	set_EA;                                     //enable interrupts
	set_TR0;                                    //Timer0 run
#endif

/*-------------------------------Init Interrupts---------------------------------------------*/
#if 1
	//Switch detect
	P17_Input_Mode;
	set_P0S_7;
	Enable_BIT7_FallEdge_Trig;
	button_trig_state=LOW;
	
	//USB detect
	P13_Input_Mode;
	set_P0S_3;
	Enable_BIT3_RasingEdge_Trig;
	charge_trig_state=HIGH;
	
	//audio BUSY pin
	P12_Input_Mode;
	set_P0S_2;
	Enable_BIT2_FallEdge_Trig;
	play_trig_state=LOW;

	Enable_INT_Port1;
	set_EPI;							// Enable pin interrupt
	//set_EA;								// global enable bit

#endif
/*---------------------------------Main function-----------------------------------------------*/
	init_LED();
	Specify_Volume(SYS_VOLUME);
	while(1){
		//set_PD;					//powerdown here can be 145.8uA
		if(Button_state==SW_PRESS && Charge_state==OFF){
			Button_state=SW_NONE;
			Body_Music_Play=STOP;
			if(Power_state==OFF){
				audio_power_on();
				Specify_Volume(SYS_VOLUME);
			}
			Play_head_music();
		}
		if(Button_state==SW_RELEASE && Charge_state==OFF && Body_Music_Play==STOP){
			Button_state=SW_NONE;
			Body_Music_Play=WAITING;
		}
		if(Play_state==STOP && Body_Music_Play==WAITING){
			if(Power_state==ON){
				Play_body_music();
				Delay_1ms(10);
				if(Play_state==PLAYING){
					Body_Music_Play=PLAYING;
				}
			}
		}
		if(Play_state==PLAYING){
#if USE_LED
			int i=0;
			int red=0, green=0, blue=0;
			set_PWMRUN;
			ADC_Finish();
			FFT();
#if 0
		    for(i=0; i<16; i++){
		      Send_Data_To_UART0(LED_TAB[i]);
		    }
			Send_Data_To_UART0('\n');
#endif
			red=LED_TAB[1]+LED_TAB[2]+LED_TAB[3]+LED_TAB[4]+LED_TAB[5];
			green=LED_TAB[6]+LED_TAB[7]+LED_TAB[8]+LED_TAB[9]+LED_TAB[10];
			blue=LED_TAB[11]+LED_TAB[12]+LED_TAB[13]+LED_TAB[14]+LED_TAB[15];
			red=red*3;
			green=green*3;
			green=green*3;
			LED_R(red);
			LED_G(green);
			LED_B(blue);
#else
			set_IDL;				//save power, if LED do not turn on
#endif
		    wake_time = 0;
		}
		if(Play_state==STOP){
			LED_R(0);
			LED_G(0);
			LED_B(0);
			set_IDL;
		}

		DEBUG_LED=1;
		Delay_1ms(10);
		DEBUG_LED=0;
		Delay_1ms(20);
		
		if(Reset_system==1){
			Delay_1ms(300);				//Add delay 300ms, in case USB connection problem.
			if(Reset_system==1){
				Reset_system=-1;
				SW_Reset();
			}
		}
		if(Reset_audio==1){
			Delay_1ms(100);				//Add delay 300ms, in case USB connection problem.
			if(Reset_audio==1){
				Reset_audio=-1;
				reboot_audio();			//reboot audio, to get the charger type.
			}
		}
		if(Play_state==PLAYING && Charge_state==ON){
			Stop_music();
		}
		if(Play_state==STOP && Charge_state==OFF && wake_time > SECOND*30)
		{
			Send_Data_To_UART0(0xaa);
			audio_power_off(1);
		}
		if(Charge_state==ON && charge_type==TYPE_USB && wake_time > MINIT*20)	//when USB state, power off audio after 20min, to make sure device can charge to full.
		{
			Send_Data_To_UART0(0xbb);
			audio_power_off(1);
		}
		if(Charge_state==ON && charge_type!=TYPE_USB && wake_time > MINIT)	//when AC state, power off audio after 1min,power off audio, to make sure device can charge to full.
		{
			Send_Data_To_UART0(0xcc);
			audio_power_off(1);
		}

	}
}