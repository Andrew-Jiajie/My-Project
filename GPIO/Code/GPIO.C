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
u8 music_in_root[]={0x34,0x03,0x01};
u8 folder_and_num[]={0x42,0x02,0x01};

u8 music_state[]={0x10};
u8 volume_get[]={0x11};

u8 return_ok[]={0x7E,0x02,0x00,0xEF};
u8 return_finished[]={0x7E,0x04,   0x3E,    0x00,0x00,0xEF};  //current music finished
u8 return_state[]={0x7E,0x03,0x10,0x00,   //0(STOP) 1(PLAY) 2(PAUS) 3(FF) 4(FR)
                  0xEF,0x7E,0x02,0x00,0xEF}; //when music_state send, will return the value
u8 state_str[12];
u8 get_state()
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

void Control_CMD(u8* cmd, u8 num){
	int set_correct=0;
    u8 timeout=3;
  	int str_num;
  	int j;
    while(!set_correct && timeout--){
      	send_cmd(cmd, num);
    	str_num = get_state();
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
#if DEBUG
  				Send_Data_To_UART0(0xCC);
#endif
          	  	break;
        	}
      	}
    }

}


void Specify_Volume(u8 num)
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
    str_num = get_state();
#if DEBUG
	Send_Data_To_UART0(0xAA);
	for(j=0; j<str_num; j++){
		Send_Data_To_UART0(state_str[j]);
	}
	Send_Data_To_UART0(0xBB);
#endif
    for(j=0; j<str_num-3; j++){
      if(state_str[j]==0x7e && state_str[j+2]==0x11 && state_str[j+3]==num){
        set_correct=1;
#if DEBUG
		Send_Data_To_UART0(0xCC);
#endif
        break;
      }
    }
  }
#endif
}

int trig_state=HIGH;
void set_trig_state(){
	if(trig_state==0){
		Enable_BIT7_HighLevel_Trig;
		trig_state=1;
	}else if(trig_state==1){
		Enable_BIT7_LowLevel_Trig;
		trig_state=0;
	}
}

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
		}
		if(trig_state==HIGH && P17==HIGH){
			Enable_BIT7_FallEdge_Trig;
			trig_state=LOW;
			Button_state=0;
		}
		clr_PD;
	}
	return;
}



void main (void) 
{
	int  Pin_last_state;
	int volume;
	Set_All_GPIO_Quasi_Mode;					// Define in Function_define.h
	
#if 1
	InitialUART0_Timer1(9600);
	set_CLOEN; 
	Delay_1ms(50);
	//Specify_Volume(15);
	//send_cmd(music_play, sizeof(music_play));
#if 1
P17_Input_Mode;
set_P0S_7;
Enable_INT_Port1;
Enable_BIT7_FallEdge_Trig;
trig_state=LOW;
//set_trig_state();

set_EPI;							// Enable pin interrupt
set_EX0;
set_EA;								// global enable bit

#endif
	
	Pin_last_state=P17;
	volume=10;
	while(1){
		Delay_1ms(15);
		Send_Data_To_UART0(P17);
		#if 0
		if(Pin_last_state==1 && P17==0){
			Button_state=1;
		}
		if(Pin_last_state==0 && P17==1){
			Button_state=0;
		}
		#endif
		Pin_last_state=P17;
		Button_state=-1;
		set_PD;
		if(Button_state==1){
			//println("pressed");
			folder_and_num[1]=1;
			folder_and_num[2]=4;
			Control_CMD(folder_and_num, sizeof(folder_and_num));
			//send_cmd(music_next, sizeof(music_next));
			Specify_Volume(volume++);
		}
		if(Button_state==0){
			//println("released");
			folder_and_num[1]=2;
			folder_and_num[2]=4;
			Control_CMD(folder_and_num, sizeof(folder_and_num));
			//send_cmd(music_next, sizeof(music_next));
			Specify_Volume(volume++);

		}
		if(volume>25)
			volume=10;
	}
#endif
}



