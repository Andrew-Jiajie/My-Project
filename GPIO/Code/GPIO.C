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
#define SOURCE_TF 2
#define SOURCE_FLASH 3

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
u8 music_in_root[]={0x41,0x00,0x01};
u8 folder_and_num[]={0x42,0x01,0x01};
u8 chip_sleep[]={0x35,0x03};
u8 chip_wakeup[]={0x35,0x02};

u8 music_state[]={0x10};
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
#if DEBUG
  		Send_Data_To_UART0(0xAA);
  		for(j=0; j<str_num; j++){
  			Send_Data_To_UART0(state_str[j]);
  		}
  		Send_Data_To_UART0(0xBB);
#endif
      	for(j=0; j<str_num-3; j++){
        	if(state_str[j]==0x7e && state_str[j+1]==0x02 && state_str[j+2]==0x00 && state_str[j+3]==0xef){
          		set_correct=1;
				return 0;
#if DEBUG
  				Send_Data_To_UART0(0xCC);
#endif
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
        	  break;
      	  }
    	}
  	}
	return -1;
#endif
}

int trig_state=HIGH;
int Button_state=-1;
void PinInterrupt_ISR (void) interrupt 7
{
	if (PIF == 0x80)
	{
		PIF =0;
		Delay_1ms(15);
		if(trig_state==LOW && P17==LOW){
			Enable_BIT7_RasingEdge_Trig;
			trig_state=HIGH;
			Button_state=1;
		}else if(trig_state==HIGH && P17==HIGH){
			Enable_BIT7_FallEdge_Trig;
			trig_state=LOW;
			Button_state=0;
		}
		clr_PD;
	}
	return;
}



#define TH0_INIT        0xff //5.0ms@XTAL=12MHz, Period = (10.85/2) ms@XTAL=22.1184MHz
#define TL0_INIT        0xff
#define TH1_INIT        0x00 //2.5ms@XTAL=12MHz, Period = (5.425/2) ms@XTAL=22.1184MHz
#define TL1_INIT        0xff
#define MINIT 			1000*60

unsigned long wake_time=0;


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
		wake_time+=40;
}

void power_down()
{
	clr_TR0;                              		  //Stop Timer0
	wake_time=0;
	set_PD;
	set_TR0;                                    //Timer0 run
}

void goto_idle()
{
	set_IDL;
}

void audio_power_on()
{
	int timeout=25;	//500ms for timeout
	int chip_ready=-1;
	while(timeout-- && chip_ready==-1){
		Delay_1ms(20);
		chip_ready=Specify_Volume(15);
		//Send_Data_To_UART0(0xCC);
	}
	Delay_1ms(110);
	//Send_Data_To_UART0(0xAA);
}

int get_current_source()
{
  	u8 timeout=2;
	int str_num;
	int j;
	
  	//while(timeout--){
    	send_cmd(current_source, sizeof(current_source));
    	str_num = get_result();
    	for(j=0; j<str_num-3; j++){
      	  if(state_str[j]==0x7e && state_str[j+2]==0x18){
        	  return state_str[j+3];
      	  }
    	}
		//Delay_1ms(20);
		//}
	return -1;
}
void change_source(int source)
{
#if 1
		if(source==SOURCE_TF){
			Control_CMD(source_TF, sizeof(source_TF));
		}else if(source==SOURCE_FLASH){
			Control_CMD(source_FLASH, sizeof(source_FLASH));
		}
		Delay_1ms(20);
	
#else
	int timeout=10;	//500ms for timeout
	int source_now=get_current_source();
	if(source_now!=source){
		if(source==SOURCE_TF){
			Control_CMD(source_TF, sizeof(source_TF));
		}else if(source==SOURCE_FLASH){
			Control_CMD(source_FLASH, sizeof(source_FLASH));
		}
		Delay_1ms(10);
		while(timeout--){
			source_now=get_current_source();
			Send_Data_To_UART0(source_now);
			if(source_now==source){
				break;
			}
			Delay_1ms(20);
		//Send_Data_To_UART0(0xCC);
		}
	}
	//Delay_1ms(30);
#endif
}

int Head_Music_Play=0;
int Body_Music_Play=0;
void main (void) 
{
	int music_num=1;
	int play_state=-1;
	//set_PD;									//powerdown directly 131.5uA
	Set_All_GPIO_Quasi_Mode;					// Define in Function_define.h
	
	InitialUART0_Timer1(9600);
	set_CLOEN; 
	audio_power_on();
	//Delay_1ms(250);			//800ms delay for audio chip get ready
	
	Specify_Volume(10);
	while(1){
				Control_CMD(source_TF, sizeof(source_TF));
				Delay_1ms(10);
				Control_CMD(music_in_root, sizeof(music_in_root));
				Delay_1ms(2000);
				Control_CMD(music_in_root, sizeof(music_in_root));
				Delay_1ms(2000);
							
				Control_CMD(source_FLASH, sizeof(source_FLASH));
				Delay_1ms(10);
				Control_CMD(music_in_root, sizeof(music_in_root));
				Delay_1ms(2000);
	}


#if 0
	
	Set_All_GPIO_Quasi_Mode;
	TIMER0_MODE0_ENABLE;                        //Timer 0 and Timer 1 mode configuration

	clr_T0M;
    TMOD |= 0x01;                           		//Timer0 is 16-bit mode
	
	TH0 = TH0_INIT;
	TL0 = TL0_INIT;
    
	set_ET0;                                    //enable Timer0 interrupt
	set_EA;                                     //enable interrupts
	
	set_TR0;                                    //Timer0 run

  	while(1){
		Send_Data_To_UART0(0xCC);
  		power_down();
  	}	

#endif



#if 1
	P17_Input_Mode;
	set_P0S_7;
	Enable_INT_Port1;
	Enable_BIT7_FallEdge_Trig;
	trig_state=LOW;

	P30_Input_Mode;
	Enable_BIT0_FallEdge_Trig;
	//Enable_INT_Port3;

	set_EPI;							// Enable pin interrupt
	set_EX0;
	set_EA;								// global enable bit

#endif
	Specify_Volume(10);
	Control_CMD(folder_and_num, sizeof(folder_and_num));
	while(1){
		//set_PD;					//powerdown here can be 145.8uA
		Button_state=-1;
		//Body_Music_Play=1;
		play_state=Get_Play_State();
		if(Head_Music_Play && play_state==PLAYING){
			Delay_1ms(50);
		}else if(Body_Music_Play){
			change_source(SOURCE_TF);
			//Control_CMD(music_stop, sizeof(music_stop));
			
			//Control_CMD(music_next, sizeof(music_next));
			Control_CMD(folder_and_num, sizeof(folder_and_num));
			Delay_1ms(10);
			Body_Music_Play=0;
#if 0
			play_state=Get_Play_State();
			if(play_state==PLAYING){
				Body_Music_Play=0;
			}else{
				//Delay_1ms(15);
				folder_and_num[2]=music_num;
				music_num++;
				if(LOBYTE(music_num)>15){
					music_num=1;
				}
			}
#endif
			Head_Music_Play=0;
		}else{
			Head_Music_Play=0;
			set_PD;
		}
		if(Button_state==1){
			change_source(SOURCE_FLASH);
			folder_and_num[1]=0;
			folder_and_num[2]=1;
			Control_CMD(folder_and_num, sizeof(folder_and_num));
			Head_Music_Play=1;
		}else if(Button_state==0){
			folder_and_num[1]=0;
			folder_and_num[2]=1;//music_num++;
			Body_Music_Play=1;
		}
	}
}



