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

#define DEBUG 0
#define HIGH 1
#define LOW 0

#define PLAYING 1
#define STOP 0
#define ON 1
#define OFF 0
#define SOURCE_TF 2
#define SOURCE_FLASH 3
#define MINIT 	60

#define HEAD_MUSIC_NUM 2

void Delay_1ms(int time)
{
	int i,j;
	for(j=0; j<time; j++)
		for (i = 0; i < 729; i++);
}

void Delay_100us(int time)
{
	int i,j;
	for(j=0; j<time; j++)
		for (i = 0; i < 72; i++);
}

/*------------------------------------------------
The main C function.  Program execution starts
here after stack initialization.
------------------------------------------------*/
typedef unsigned char u8;
u8 music_play[]={0x01};
u8 music_stop[]={0x0E};
u8 music_next[]={0x03};
u8 volume_set[]={0x31,0x10};
u8 source_TF[]={0x35,0x01};
u8 source_FLASH[]={0x35,0x04};
u8 music_for_head[]={0x42,0x01,0x02};
u8 music_for_next[]={0x41,0x00,0x01};
u8 music_in_root[]={0x41,0x00,0x01};
u8 folder_and_num[]={0x42,0x01,0x01};
u8 chip_sleep[]={0x35,0x03};
u8 chip_wakeup[]={0x35,0x02};

u8 music_state[]={0x10};
u8 music_number[]={0x17};
u8 volume_get[]={0x11};
u8 current_source[]={0x18};

u8 return_ok[]={0x7E,0x02,0x00,0xEF};
u8 return_finished[]={0x7E,0x04,   0x3E,    0x00,0x00,0xEF};  //current music finished
u8 return_state[]={0x7E,0x03,0x10,0x00,   //0(STOP) 1(PLAY) 2(PAUS) 3(FF) 4(FR)
                  0xEF,0x7E,0x02,0x00,0xEF}; //when music_state send, will return the value
u8 state_str[12];
u8 get_result()
{
    int result_num=0;
	int timeout=250;

	while(!RI && timeout--){
		Delay_100us(1);		//delay 25ms
	}
    while (RI) {
      state_str[result_num++]=Receive_Data_From_UART0();
	  timeout=8000;
	  while(!RI && timeout--);
    }
    return result_num;
}

void send_cmd(u8* cmd, u8 num){
  u8 count=0;
  u8 command[15];
  u8 tmp_cnt=0;
  int out_cnt=0;
	
  command[count++]=0x7E;
  command[count++]=0x01+num;
  while(tmp_cnt<num){
    command[count++]=*(cmd+tmp_cnt);
    tmp_cnt++;
  }
  command[count]=0xEF;
  while(out_cnt<=count){
    Send_Data_To_UART0(command[out_cnt++]);
  }
}

int Control_CMD(u8* cmd, u8 num){
	int set_correct=0;
    u8 timeout=2;
  	int str_num;
  	int j;
    while(!set_correct && timeout--){
      	send_cmd(cmd, num);
    	str_num = get_result();
      	for(j=0; j<str_num-3; j++){
        	if(state_str[j]==0x7e && state_str[j+1]==0x02 && state_str[j+2]==0x00 && state_str[j+3]==0xef){
          		set_correct=1;
				return 0;
          	  	break;
        	}
      	}
    }
	return -1;
}


int Get_Play_State()
{
  	u8 timeout=2;
	int str_num;
	int j;
	
  	while(timeout--){
    	send_cmd(music_state, sizeof(music_state));
    	str_num = get_result();
    	for(j=0; j<str_num-3; j++){
      	  if(state_str[j]==0x7e && state_str[j+2]==0x10){
        	  return state_str[j+3];
      	  }
    	}
		Delay_1ms(20);
  	}
	return -1;
}

int Get_total_number()
{
  	u8 timeout=2;
	int str_num;
	int j;
	
  	while(timeout--){
    	send_cmd(music_number, sizeof(music_number));
    	str_num = get_result();
    	for(j=0; j<str_num-4; j++){
      	  if(state_str[j]==0x7e && state_str[j+2]==0x17){
			  return state_str[j+4];
      	  }
    	}
		Delay_1ms(20);
  	}
	return -1;
}

int play_head_music(){
	Control_CMD(music_for_head, sizeof(music_for_head));
	music_for_head[2]++;
	if(LOBYTE(music_for_head[2])>HEAD_MUSIC_NUM){
		music_for_head[2]=1;
	}
}

int play_body_music()
{
	int total_music=Get_total_number();
	if(total_music > HEAD_MUSIC_NUM){
		Control_CMD(music_for_next, sizeof(music_for_next));
		music_for_next[2]++;
		if(LOBYTE(music_for_next[2])>total_music-HEAD_MUSIC_NUM){
			music_for_next[2]=1;
		}
	}
}

int Specify_Volume(u8 num)
{
	
#if 0
	volume_set[1] = num;
	Control_CMD(volume_set, sizeof(volume_set));
#else
  	int set_correct=0;
  	u8 timeout=3;
	int str_num;
	int j;
	volume_set[1] = num;
	
  	while(!set_correct && timeout--){
    	send_cmd(volume_set, sizeof(volume_set));
		Delay_1ms(13);	// 13ms is the best timedelay for volume set!
    	send_cmd(volume_get, sizeof(volume_get));
    	str_num = get_result();
    	for(j=0; j<str_num-3; j++){
      	  if(state_str[j]==0x7e && state_str[j+2]==0x11 && state_str[j+3]==num){
        	  set_correct=1;
			  return 0;
      	  }
    	}
  	}
	return -1;
#endif
}

#define setbit(x,y) x|=(1<<y) //将X的第Y位置1
#define testbit(x,y) x&(1<<y) //测试X的第Y位置
#define clrbit(x,y) x&=!(1<<y) //将X的第Y位清0

int button_trig_state=HIGH;
int play_trig_state=HIGH;
int charge_trig_state=HIGH;

int Button_state=-1;
int Play_state=STOP;
int Charge_state=OFF;
int Reset_system=-1;
void PinInterrupt_ISR (void) interrupt 7
{
	if (testbit(PIF,7))	//SWITCH PIN
	{
		clrbit(PIF,7);
		Delay_1ms(15);
		if(button_trig_state==LOW && P17==LOW){
			Enable_BIT7_RasingEdge_Trig;
			button_trig_state=HIGH;
			Button_state=1;
		}else if(button_trig_state==HIGH && P17==HIGH){
			Enable_BIT7_FallEdge_Trig;
			button_trig_state=LOW;
			Button_state=0;
		}
		clr_PD;
	}
	if (testbit(PIF,3))	//BUSY PIN
	{
		clrbit(PIF,3);
		Delay_1ms(2);
		if(play_trig_state==LOW && P13==LOW){
			Enable_BIT3_RasingEdge_Trig;
			play_trig_state=HIGH;
			Play_state=PLAYING;
		}else if(play_trig_state==HIGH && P13==HIGH){
			Enable_BIT3_FallEdge_Trig;
			play_trig_state=LOW;
			Play_state=STOP;
		}
		//Send_Data_To_UART0(0xcc);
		clr_PD;
	}
	if (testbit(PIF,2))	//USB PIN
	{
		clrbit(PIF,2);
		Delay_1ms(10);
		if(charge_trig_state==LOW && P13==LOW){
			Enable_BIT2_RasingEdge_Trig;
			charge_trig_state=HIGH;
			Charge_state=ON;
		}else if(charge_trig_state==HIGH && P13==HIGH){
			Enable_BIT2_FallEdge_Trig;
			charge_trig_state=LOW;
			Charge_state=OFF;
			Reset_system=1;
		}
		//Send_Data_To_UART0(0xcc);
		clr_PD;
	}
	return;
}

unsigned long wake_time=0;
unsigned long timer_count=0;
int Power_state=ON;

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
		//Send_Data_To_UART0(0xcc);
}

void audio_power_on()
{
	int timeout=25;	//500ms for timeout
	int chip_ready=-1;
	P11=HIGH;
	while(timeout-- && chip_ready==-1){
		Delay_1ms(20);
		chip_ready=Specify_Volume(28);
		//Send_Data_To_UART0(0xCC);
	}
	Delay_1ms(110);
	Power_state=ON;
}

void audio_power_off()
{
	clr_TR0;                              		  //Stop Timer0
	wake_time=0;
	P11=LOW;
	Power_state=OFF;
	Send_Data_To_UART0(0xBB);
	set_PD;
	set_TR0;                                    //Timer0 run
}

void init_LED(){
	
}


int Head_Music_Play=0;
int Body_Music_Play=0;
void main (void) 
{
	int music_num=1;
	int play_state=-1;
	
	//set_PD;									//powerdown directly 131.5uA
	Set_All_GPIO_Quasi_Mode;					// Define in Function_define.h
	P11_PushPull_Mode;
	InitialUART0_Timer1(9600);
	set_CLOEN; 
	audio_power_on();
	//Delay_1ms(250);			//800ms delay for audio chip get ready

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

#if 1
	P17_Input_Mode;
	set_P0S_7;
	Enable_BIT7_FallEdge_Trig;
	button_trig_state=LOW;

	P13_Input_Mode;
	set_P0S_3;
	Enable_BIT3_FallEdge_Trig;
	play_trig_state=LOW;
	
	
	P30_Input_Mode;
	Enable_BIT0_FallEdge_Trig;

	Enable_INT_Port1;
	set_EPI;							// Enable pin interrupt
	set_EX0;
	set_EA;								// global enable bit

#endif
	Specify_Volume(30);
	//Control_CMD(folder_and_num, sizeof(folder_and_num));
	while(1){
		//set_PD;					//powerdown here can be 145.8uA
		//play_state=Get_Play_State();
		if(Button_state==1){
			Button_state=-1;
			if(Power_state==OFF){
				audio_power_on();
			}
			play_head_music();
			//Head_Music_Play=1;
			Body_Music_Play=0;
		}
		if(Button_state==0){
			Button_state=-1;
			Body_Music_Play=1;
		}
		if(Play_state==PLAYING || Charge_state==ON){
			wake_time = 0;
			//Send_Data_To_UART0(0xBB);
			set_IDL;
		}
		if(Reset_system==1){
			Reset_system=-1;
			set_SWRST;
		}
		if(Charge_state==ON && Power_state==OFF){
			audio_power_on();
		}
		if(Play_state==STOP && Charge_state==OFF && wake_time > MINIT/12){
			audio_power_off();
		}
#if 0
		if(Play_state==STOP && Head_Music_Play==1){
			Head_Music_Play=0;
		}
#endif
		if(Play_state==STOP && Body_Music_Play==1){
			if(Power_state==ON){
				play_body_music();
			}
			Body_Music_Play=0;
		}
	}
}
