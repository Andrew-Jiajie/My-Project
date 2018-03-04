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
//�Ŵ�128�����cos������128��
char COS_TAB[64] = { 0x7f, 0x7e, 0x7c, 0x79, 0x75, 0x70, 0x69, 0x62, 
0x59, 0x50, 0x46, 0x3b, 0x30, 0x24, 0x18, 0x0c, 
0x00, -0x0c, -0x18, -0x24, -0x30, -0x3b, -0x46, -0x50, 
-0x59, -0x62, -0x69, -0x70, -0x75, -0x79, -0x7c, -0x7e, 
-0x7f, -0x7e, -0x7c, -0x79, -0x75, -0x70, -0x69, -0x62, 
-0x59, -0x50, -0x46, -0x3b, -0x30, -0x24, -0x18, -0x0c, 
0x00, 0x0c, 0x18, 0x24, 0x30, 0x3b, 0x46, 0x50, 
0x59, 0x62, 0x69, 0x70, 0x75, 0x79, 0x7c, 0x7e, 
 };
//�����洢���б�
char LIST_TAB[64] = { 0, 32, 16, 48, 8, 40, 24, 56,
4, 36, 20, 52, 12, 44, 28, 60,
2, 34, 18, 50, 10, 42, 26, 58,
6, 38, 22, 54, 14, 46, 30, 62,
1, 33, 17, 49, 9, 41, 25, 57,
5, 37, 21, 53, 13, 45, 29, 61,
3, 35, 19, 51, 11, 43, 27, 59,
7, 39, 23, 55, 15, 47, 31, 63
};

int Fft_Real[64]={0XFF}; 
int Fft_Image[64];               // fft���鲿 
//uchar LED_TAB2[32];        //��¼ Ư���� �Ƿ���Ҫ ͣ��һ��
int LED_TAB[32];       //��¼��ɫ��״ 
//uchar LED_TAB1[32];        //��¼ Ư����

float my_sqrt(float number) {
    float new_guess;
    float last_guess;
 
    if (number < 0) {
        //printf("Cannot compute the square root of a negative number!\n");
        return -1;
    }
    if(number==0) return 0;
    new_guess = 1;
    do {
        last_guess = new_guess;
        new_guess = (last_guess + number / last_guess) / 2;
    } while (new_guess != last_guess);
 
    return new_guess;
}

void FFT()
{
	uchar i,j,k,b,p;
	uchar COUNT=15,COUNT1=0;
               
	int Temp_Real,Temp_Imag,temp;                // �м���ʱ����  
	uint TEMP1; 
	
    for( i=1; i<=6; i++)                            /* for(1) */
    { 
        b=1;
        b <<=(i-1);                                       //��ʽ���㣬���ڼ��� �������м��� ���� ��һ�� 1��2�м��㣬���ڶ��� 
        for( j=0; j<=b-1; j++)                              /* for (2) */
        { 
            p=1;
            p <<= (7-i);            
            p = p*j;
            for( k=j; k<64; k=k+2*b)                /* for (3) ����fft */
            { 
                Temp_Real = Fft_Real[k]; 
        		Temp_Imag = Fft_Image[k]; 
        		temp = Fft_Real[k+b];
                Fft_Real[k] = Fft_Real[k] + ((Fft_Real[k+b]*COS_TAB[p])>>7) + ((Fft_Image[k+b]*SIN_TAB[p])>>7);
                Fft_Image[k] = Fft_Image[k] - ((Fft_Real[k+b]*SIN_TAB[p])>>7) + ((Fft_Image[k+b]*COS_TAB[p])>>7);
                Fft_Real[k+b] = Temp_Real - ((Fft_Real[k+b]*COS_TAB[p])>>7) - ((Fft_Image[k+b]*SIN_TAB[p])>>7);
                Fft_Image[k+b] = Temp_Imag + ((temp*SIN_TAB[p])>>7) - ((Fft_Image[k+b]*COS_TAB[p])>>7);     
                // ��λ.��ֹ���. ����Ѿ��Ǳ�ֵ�� 1/64               
                Fft_Real[k] >>= 1;             
                Fft_Image[k] >>= 1; 
                Fft_Real[k+b]  >>= 1;                 
                Fft_Image[k+b]  >>= 1; 

                                                                               
            }     
        } 
    } 
  Fft_Real[0]=Fft_Image[0]=0;     //ȥ��ֱ������
  //Fft_Real[63]=Fft_Image[63]=0;
   // if(fractional_frequency==64)
//  j_value=64;
//  else
//  j_value=20;
#if 0
    for(j=0; j<16; j++)
    {
     TEMP1=((((Fft_Real[j]* Fft_Real[j]))+((Fft_Image[j]*Fft_Image[j]))));//����
    
           if(TEMP1<9)i=1;             //��ģ������
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
    for(j=0;j<32;j++) 
  {
    TEMP1=((((Fft_Real[j]* Fft_Real[j]))+((Fft_Image[j]*Fft_Image[j]))));//����
#if 0
	if(TEMP1>1024)
		TEMP1=1024;
	if((TEMP1)<4)
		LED_TAB[j]=0;
	else
		LED_TAB[j]=my_sqrt(TEMP1)-1;
#else	
    if((TEMP1)<4)
		TEMP1=0;
    else if(TEMP1<9)
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
#endif      
#if 0
        if(TEMP1>(LED_TAB1[j]))
        {   
			LED_TAB1[j]=TEMP1;
            LED_TAB2[j]=7;  //12                                              //����ٶ�=12
        }
#endif
  }
  #endif
}