
#define HEAD_MUSIC_NUM 2
#define TYPE_NONE 0
#define TYPE_AC 1
#define TYPE_USB 2


uchar music_play[]={0x01};
uchar music_stop[]={0x0E};
uchar music_next[]={0x03};
uchar volume_set[]={0x31,0x10};
uchar source_TF[]={0x35,0x01};
uchar source_FLASH[]={0x35,0x04};
uchar music_for_head[]={0x42,0x01,0x02};
uchar music_for_next[]={0x41,0x00,0x01};
uchar music_in_root[]={0x41,0x00,0x01};
uchar folder_and_num[]={0x42,0x01,0x01};
uchar chip_sleep[]={0x35,0x03};
uchar chip_wakeup[]={0x35,0x02};

uchar music_state[]={0x10};
uchar music_number[]={0x17};
uchar volume_get[]={0x11};
uchar current_source[]={0x18};

uchar return_ok[]={0x7E,0x02,0x00,0xEF};
uchar return_finished[]={0x7E,0x04,   0x3E,    0x00,0x00,0xEF};  //current music finished
uchar return_state[]={0x7E,0x03,0x10,0x00,   //0(STOP) 1(PLAY) 2(PAUS) 3(FF) 4(FR)
                  0xEF,0x7E,0x02,0x00,0xEF}; //when music_state send, will return the value
uchar state_str[12];

uchar get_result()
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

void send_cmd(uchar* cmd, uchar num){
  uchar count=0;
  uchar command[15];
  uchar tmp_cnt=0;
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

int Control_CMD(uchar* cmd, uchar num){
	int set_correct=0;
    uchar timeout=2;
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
  	uchar timeout=2;
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
  	uchar timeout=2;
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

int Play_head_music(){
	Control_CMD(music_for_head, sizeof(music_for_head));
	music_for_head[2]++;
	if(LOBYTE(music_for_head[2])>HEAD_MUSIC_NUM){
		music_for_head[2]=1;
	}
}

int Play_body_music()
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
int Get_charge_type(int timeout){
	int str_num;
	int j;
  	while(timeout--){
    	str_num = get_result();		//25ms each time
    	for(j=0; j<str_num-4; j++){
      	  if(state_str[j]==0x7e && state_str[j+2]==0x3f && state_str[j+3]==0x04){
			  return TYPE_AC;
      	  }
      	  if(state_str[j]==0x7e && state_str[j+2]==0x3f && state_str[j+3]==0x0c){
			  return TYPE_USB;
      	  }
    	}
  	}
	return TYPE_NONE;
}
void Stop_music()
{
	Control_CMD(music_stop, sizeof(music_stop));
}

int Specify_Volume(uchar num)
{
	
#if 1
	volume_set[1] = num;
	Control_CMD(volume_set, sizeof(volume_set));
#else
  	int set_correct=0;
  	uchar timeout=3;
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
