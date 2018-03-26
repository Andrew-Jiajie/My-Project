#define SMALL_SIZE 1

#if SMALL_SIZE
#define ARRAY_SIZE 64
//-----------------------------------------------------------------------------------------------
char SIN_TAB[ARRAY_SIZE] = { 0x00, 0x0c, 0x18, 0x24, 0x30, 0x3b, 0x46, 0x50, 
0x59, 0x62, 0x69, 0x70, 0x75, 0x79, 0x7c, 0x7e, 
0x7f, 0x7e, 0x7c, 0x79, 0x75, 0x70, 0x69, 0x62, 
0x59, 0x50, 0x46, 0x3b, 0x30, 0x24, 0x18, 0x0c, 
0x00, -0x0c, -0x18, -0x24, -0x30, -0x3b, -0x46, -0x50, 
-0x59, -0x62, -0x69, -0x70, -0x75, -0x79, -0x7c, -0x7e, 
-0x7f, -0x7e, -0x7c, -0x79, -0x75, -0x70, -0x69, -0x62, 
-0x59, -0x50, -0x46, -0x3b, -0x30, -0x24, -0x18, -0x0c, 
 };
//放大128倍后的cos整数表（128）
char COS_TAB[ARRAY_SIZE] = { 0x7f, 0x7e, 0x7c, 0x79, 0x75, 0x70, 0x69, 0x62, 
0x59, 0x50, 0x46, 0x3b, 0x30, 0x24, 0x18, 0x0c, 
0x00, -0x0c, -0x18, -0x24, -0x30, -0x3b, -0x46, -0x50, 
-0x59, -0x62, -0x69, -0x70, -0x75, -0x79, -0x7c, -0x7e, 
-0x7f, -0x7e, -0x7c, -0x79, -0x75, -0x70, -0x69, -0x62, 
-0x59, -0x50, -0x46, -0x3b, -0x30, -0x24, -0x18, -0x0c, 
0x00, 0x0c, 0x18, 0x24, 0x30, 0x3b, 0x46, 0x50, 
0x59, 0x62, 0x69, 0x70, 0x75, 0x79, 0x7c, 0x7e, 
 };
//采样存储序列表
char LIST_TAB[ARRAY_SIZE] = { 0, 32, 16, 48, 8, 40, 24, 56,
4, 36, 20, 52, 12, 44, 28, 60,
2, 34, 18, 50, 10, 42, 26, 58,
6, 38, 22, 54, 14, 46, 30, 62,
1, 33, 17, 49, 9, 41, 25, 57,
5, 37, 21, 53, 13, 45, 29, 61,
3, 35, 19, 51, 11, 43, 27, 59,
7, 39, 23, 55, 15, 47, 31, 63
};

//char test[ARRAY_SIZE*2];
#else
#define ARRAY_SIZE 128
//放大128倍后的sin整数表（128）
char SIN_TAB[ARRAY_SIZE] = { 0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 59, 65, 70, 75, 80, 85, 89, 94, 98, 102,
                           105, 108, 112, 114, 117, 119, 121, 123, 124, 125, 126, 126, 126, 126, 126, 125, 124, 123, 121, 119, 117, 114, 112,
                           108, 105, 102, 98, 94, 89, 85, 80, 75, 70, 65, 59, 54, 48, 42, 36, 30, 24, 18, 12, 6, 0, -6, -12, -18, -24, -30,
                           -36, -42, -48, -54, -59, -65, -70, -75, -80, -85, -89, -94, -98, -102, -105, -108, -112, -114, -117, -119, -121,
                           -123, -124, -125, -126, -126, -126, -126, -126, -125, -124, -123, -121, -119, -117, -114, -112, -108, -105, -102,
                           -98, -94, -89, -85, -80, -75, -70, -65, -59, -54, -48, -42, -36, -30, -24, -18, -12, -6
                         };
//放大128倍后的cos整数表（128）
char COS_TAB[ARRAY_SIZE] = { 127, 126, 126, 125, 124, 123, 121, 119, 117, 114, 112, 108, 105, 102, 98, 94,
                           89, 85, 80, 75, 70, 65, 59, 54, 48, 42, 36, 30, 24, 18, 12, 6, 0, -6, -12, -18, -24, -30, -36, -42, -48, -54, -59,
                           -65, -70, -75, -80, -85, -89, -94, -98, -102, -105, -108, -112, -114, -117, -119, -121, -123, -124, -125, -126, -
                           126, -126, -126, -126, -125, -124, -123, -121, -119, -117, -114, -112, -108, -105, -102, -98, -94, -89, -85, -80,
                           -75, -70, -65, -59, -54, -48, -42, -36, -30, -24, -18, -12, -6, 0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 59, 65, 70,
                           75, 80, 85, 89, 94, 98, 102, 105, 108, 112, 114, 117, 119, 121, 123, 124, 125, 126, 126
                         };
//采样存储序列表
char LIST_TAB[ARRAY_SIZE] = { 0, 64, 32, 96, 16, 80, 48, 112,
                            8, 72, 40, 104, 24, 88, 56, 120,
                            4, 68, 36, 100, 20, 84, 52, 116,
                            12, 76, 44, 108, 28, 92, 60, 124,
                            2, 66, 34, 98, 18, 82, 50, 114,
                            10, 74, 42, 106, 26, 90, 58, 122,
                            6, 70, 38, 102, 22, 86, 54, 118,
                            14, 78, 46, 110, 30, 94, 62, 126,
                            1, 65, 33, 97, 17, 81, 49, 113,
                            9, 73, 41, 105, 25, 89, 57, 121,
                            5, 69, 37, 101, 21, 85, 53, 117,
                            13, 77, 45, 109, 29, 93, 61, 125,
                            3, 67, 35, 99, 19, 83, 51, 115,
                            11, 75, 43, 107, 27, 91, 59, 123,
                            7, 71, 39, 103, 23, 87, 55, 119,
                            15, 79, 47, 111, 31, 95, 63, 127
                          };
#endif

int Fft_Real[ARRAY_SIZE]={0XFF}; 
int Fft_Image[ARRAY_SIZE];               // fft的虚部 
int LED_TAB[ARRAY_SIZE/2];       //记录红色柱状 

void FFT()
{
	uchar i,j,k,b,p;
	uchar COUNT=15,COUNT1=0;
               
	int Temp_Real,Temp_Imag,temp;                // 中间临时变量  
	uint TEMP1; 
	
    for( i=1; i<=7; i++)                            /* for(1) */
    {
        b=1;
        b <<=(i-1);                                       //碟式运算，用于计算 隔多少行计算 例如 第一极 1和2行计算，，第二级
        for( j=0; j<=b-1; j++)                              /* for (2) */
        {
            p=1;
            p <<= (7-i);
            p = p*j;
            for( k=j; k<ARRAY_SIZE; k=k+2*b)                /* for (3) 基二fft */
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
  for(j=0;j<ARRAY_SIZE/2;j++) 
  {
    TEMP1=((((Fft_Real[j]* Fft_Real[j]))+((Fft_Image[j]*Fft_Image[j]))));//求功率

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

  }
}