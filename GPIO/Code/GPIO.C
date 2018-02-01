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

#define uchar unsigned char
#define uint unsigned int

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
	if (testbit(PIF,2))	//BUSY PIN
	{
		clrbit(PIF,2);
		Delay_1ms(2);	
		if(play_trig_state==LOW && P12==LOW){
			Enable_BIT2_RasingEdge_Trig;
			play_trig_state=HIGH;
			Play_state=PLAYING;
		}else if(play_trig_state==HIGH && P12==HIGH){
			Enable_BIT2_FallEdge_Trig;
			play_trig_state=LOW;
			Play_state=STOP;
		}
		clr_PD;
	}
	if (testbit(PIF,3))	//USB PIN
	{
		clrbit(PIF,3);
		Delay_1ms(10);
		if(charge_trig_state==LOW && P13==LOW){
			Enable_BIT3_RasingEdge_Trig;
			charge_trig_state=HIGH;
			Charge_state=ON;
		}else if(charge_trig_state==HIGH && P13==HIGH){
			Enable_BIT3_FallEdge_Trig;
			charge_trig_state=LOW;
			Charge_state=OFF;
			//Reset_system=1;
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
	P01=HIGH;
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
	P01=LOW;
	Power_state=OFF;
	Send_Data_To_UART0(0xBB);
	set_PD;
	set_TR0;                                    //Timer0 run
}
//-----------------------------------------------------------------------------------------------
char SIN_TAB[64] = { 0x00, 0x0c, 0x18, 0x24, 0x30, 0x3b, 0x46, 0x50, 
0x59, 0x62, 0x69, 0x70, 0x75, 0x79, 0x7c, 0x7e, 
0x7f, 0x7e, 0x7c, 0x79, 0x75, 0x70, 0x69, 0x62, 
0x59, 0x50, 0x46, 0x3b, 0x30, 0x24, 0x18, 0x0c, 
0x00, -0x0c, -0x18, -0x24, -0x30, -0x3b, -0x46, -0x50, 
-0x59, -0x62, -0x69, -0x70, -0x75, -0x79, -0x7c, -0x7e, 
-0x7f, -0x7e, -0x7c, -0x79, -0x75, -0x70, -0x69, -0x62, 
-0x59, -0x50, -0x46, -0x3b, -0x30, -0x24, -0x18, -0x0c, 
 };
//放大128倍后的cos整数表（128）
char COS_TAB[64] = { 0x7f, 0x7e, 0x7c, 0x79, 0x75, 0x70, 0x69, 0x62, 
0x59, 0x50, 0x46, 0x3b, 0x30, 0x24, 0x18, 0x0c, 
0x00, -0x0c, -0x18, -0x24, -0x30, -0x3b, -0x46, -0x50, 
-0x59, -0x62, -0x69, -0x70, -0x75, -0x79, -0x7c, -0x7e, 
-0x7f, -0x7e, -0x7c, -0x79, -0x75, -0x70, -0x69, -0x62, 
-0x59, -0x50, -0x46, -0x3b, -0x30, -0x24, -0x18, -0x0c, 
0x00, 0x0c, 0x18, 0x24, 0x30, 0x3b, 0x46, 0x50, 
0x59, 0x62, 0x69, 0x70, 0x75, 0x79, 0x7c, 0x7e, 
 };

//采样存储序列表
char LIST_TAB[64] = { 0, 32, 16, 48, 8, 40, 24, 56,
4, 36, 20, 52, 12, 44, 28, 60,
2, 34, 18, 50, 10, 42, 26, 58,
6, 38, 22, 54, 14, 46, 30, 62,
1, 33, 17, 49, 9, 41, 25, 57,
5, 37, 21, 53, 13, 45, 29, 61,
3, 35, 19, 51, 11, 43, 27, 59,
7, 39, 23, 55, 15, 47, 31, 63
};

uchar COUNT=15,COUNT1=0,ADC_Count=0,LINE=15,G,T;
uchar i,j,k,b,p;                 
int Temp_Real,Temp_Imag,temp;                // 中间临时变量  
uint TEMP1;
uchar PWM;  
int Fft_Real[64]={0XFF}; 
int Fft_Image[64];               // fft的虚部 
uchar LED_TAB2[32];        //记录 漂浮物 是否需要 停顿一下
uchar LED_TAB[32];       //记录红色柱状 
uchar LED_TAB1[32];        //记录 漂浮点

void FFT()
{     
  //uchar x;              
    for( i=1; i<=6; i++)                            /* for(1) */
    { 
        b=1;
        b <<=(i-1);                                       //碟式运算，用于计算 隔多少行计算 例如 第一极 1和2行计算，，第二级 
        for( j=0; j<=b-1; j++)                              /* for (2) */
        { 
            p=1;
            p <<= (7-i);            
            p = p*j;
            for( k=j; k<64; k=k+2*b)                /* for (3) 基二fft */
            { 
                Temp_Real = Fft_Real[k]; 
        Temp_Imag = Fft_Image[k]; 
        temp = Fft_Real[k+b];
                Fft_Real[k] = Fft_Real[k] + ((Fft_Real[k+b]*COS_TAB[p])>>7) + ((Fft_Image[k+b]*SIN_TAB[p])>>7);
                Fft_Image[k] = Fft_Image[k] - ((Fft_Real[k+b]*SIN_TAB[p])>>7) + ((Fft_Image[k+b]*COS_TAB[p])>>7);
                Fft_Real[k+b] = Temp_Real - ((Fft_Real[k+b]*COS_TAB[p])>>7) - ((Fft_Image[k+b]*SIN_TAB[p])>>7);
                Fft_Image[k+b] = Temp_Imag + ((temp*SIN_TAB[p])>>7) - ((Fft_Image[k+b]*COS_TAB[p])>>7);     
                // 移位.防止溢出. 结果已经是本值的 1/64               
                Fft_Real[k] >>= 1;             
                Fft_Image[k] >>= 1; 
                Fft_Real[k+b]  >>= 1;                 
                Fft_Image[k+b]  >>= 1; 

                                                                               
            }     
        } 
    } 
  Fft_Real[0]=Fft_Image[0]=0;     //去掉直流分量
  //Fft_Real[63]=Fft_Image[63]=0;
   // if(fractional_frequency==64)
//  j_value=64;
//  else
//  j_value=20;
#if 0
    for(j=0; j<16; j++)
    {
     TEMP1=((((Fft_Real[j]* Fft_Real[j]))+((Fft_Image[j]*Fft_Image[j]))));//求功率
    
           if(TEMP1<9)i=1;             //求模并量化
    else if(TEMP1<10)i=1;
    else if(TEMP1<20)i=2;
    else if(TEMP1<30)i=3;
    else if(TEMP1<40)i=4;
    else if(TEMP1<60)i=5;
    else if(TEMP1<80)i=6;
    else if(TEMP1<100)i=7;
    else if(TEMP1<120)i=8;
    else if(TEMP1<140)i=9;
    else if(TEMP1<160)i=10;
    else if(TEMP1<196)i=11;
    else if(TEMP1<220)i=12;
    else if(TEMP1<250)i=13;
    else i=20;
    
    LED_TAB[j]=i; 

  }       
#else
    for(j=0;j<17;j++) 
  {
    TEMP1=((((Fft_Real[j]* Fft_Real[j]))+((Fft_Image[j]*Fft_Image[j]))));//求功率
  
        if(TEMP1<6)
      TEMP1=0;
        else if(TEMP1<10)
      TEMP1=1;
        else if(TEMP1<16)
      TEMP1=2;
        else if(TEMP1<25)
      TEMP1=3;
        else if(TEMP1<36)
      TEMP1=4;
        else if(TEMP1<49)
      TEMP1=5;
        else if(TEMP1<55)
      TEMP1=6;
        else if(TEMP1<60)
      TEMP1=7;
        else if(TEMP1<65)
      TEMP1=8;
        else if(TEMP1<70)
      TEMP1=9;
        else if(TEMP1<75)
      TEMP1=10;
        else if(TEMP1<80)
      TEMP1=11;
        else if(TEMP1<96)
      TEMP1=12;
        else if(TEMP1<125)
      TEMP1=13;
        else if(TEMP1<156)
      TEMP1=14;
        else if(TEMP1<189)
      TEMP1=15;
        else if(TEMP1<224)
      TEMP1=16;
        else if(TEMP1<261)
      TEMP1=17;
        else if(TEMP1<300)
      TEMP1=18;
        else if(TEMP1<341)
      TEMP1=19;
        else if(TEMP1<384)
      TEMP1=20;
        else if(TEMP1<429)
      TEMP1=21;
        else if(TEMP1<476)
      TEMP1=22;
        else if(TEMP1<525)
      TEMP1=23 ;
        else if(TEMP1<576)
      TEMP1=24;
        else if(TEMP1<629)
      TEMP1=25;
        else if(TEMP1<684)
      TEMP1=26;
        else if(TEMP1<741)
      TEMP1=27;
        else if(TEMP1<800)
      TEMP1=28;
        else if(TEMP1<861)
      TEMP1=29;
        else if(TEMP1<1024)
      TEMP1=30;
        else 
      TEMP1=31;
  
      //if(TEMP1>(LED_TAB[j]))
      LED_TAB[j]=TEMP1;        
        if(TEMP1>(LED_TAB1[j]))
        {   
      LED_TAB1[j]=TEMP1;
            LED_TAB2[j]=7;  //12                                              //提顿速度=12
        }
  }
  #endif
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
	while(0){
		LED_R(1024);
		Delay_1ms(500);
		LED_R(0);
		LED_G(1024);
		Delay_1ms(500);
		LED_G(0);
		LED_B(1024);
		Delay_1ms(500);
		LED_B(0);
	}
}
//-----------------------------------------------------------------------------------

int get_adc(void)
{
	clr_ADCF;
	set_ADCS;									// ADC start trig signal
	while(ADCF == 0);
  	return (int)(ADCRH<<2) + (int)((ADCRL&0x0f)>>2);//(((int)ADCRH)<<4+ADCRL&0x0f);
}

int Head_Music_Play=0;
int Body_Music_Play=0;
void main (void) 
{
	int music_num=1;
	int play_state=-1;
	
	//set_PD;									//powerdown directly 131.5uA
	Set_All_GPIO_Quasi_Mode;					// Define in Function_define.h
	P01_PushPull_Mode;
	P01=0;
	//P0=0;
	//P1=0;
	Delay_1ms(500);
	P01=1;
	while(0){
		P01=0;
		P0=0;
		P1=0;
		Delay_1ms(2000);
		P01=1;
		Delay_1ms(2000);
	}
	
	
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
	//Switch detect
	P17_Input_Mode;
	set_P0S_7;
	Enable_BIT7_FallEdge_Trig;
	button_trig_state=LOW;
	
	//USB detect
	P13_Input_Mode;
	set_P0S_3;
	Enable_BIT3_FallEdge_Trig;
	charge_trig_state=LOW;
	
	//audio BUSY pin
	P12_Input_Mode;
	set_P0S_2;
	Enable_BIT2_FallEdge_Trig;
	play_trig_state=LOW;

	Enable_INT_Port1;
	set_EPI;							// Enable pin interrupt
	set_EX0;
	set_EA;								// global enable bit

#endif
	init_LED();
	Specify_Volume(20);
	//Control_CMD(music_next, sizeof(music_next));
	while(1){
		//set_PD;					//powerdown here can be 145.8uA
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
#if 1
			int i=0;
			int red=0, green=0, blue=0;
			set_PWMRUN;
			//Send_Data_To_UART0(0xBB);
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
			red=red*4;
			green=green*4;
			green=green*4;
			LED_R(red);
			LED_G(green);
			LED_B(blue);
#endif
		    wake_time = 0;
			//set_IDL;
		}
		if(Play_state==STOP){
			LED_R(0);
			LED_G(0);
			LED_B(0);
			//clr_PWMRUN;
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
