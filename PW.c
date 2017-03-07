/*********程序说明**********/
    
//本程序旨在完成一个有关环境实时监测仪器的制作，
//涉及到的传感器有DHT11温湿度传感器、雨滴传感器、
//烟雾传感器、光敏传感器、LCD液晶显示屏

/**********头文件***********/
#include<stdio.h>
#include<wiringPi.h>
#inclue<lcd.h>

/*********类型定义**********/
typedef unsigned char uchar;  
typedef unsigned int uint;  
typedef unsigned long uint32;

/******传感器参数设置*******/
#define     ADC_CS    0  
#define     ADC_CLK   1 //ADC0832数模转换芯片接口

#define     Rain_D    2 //雨滴传感器接口

#define     Smog_D    3 //烟雾传感器接口

#define     Light_D   4 //光敏传感器接口

#define     DHT11     5 //DHT11温湿度传感器接口

/*******lcd参数设置********/
#define lcdRows 2
#define lcdCols 16
#define lcdBits 8
#define lcdRS   15
#define lcdEn   14
#define lcdD0   6
#define lcdD1   7
#define lcdD2   8
#define lcdD3   9
#define lcdD4   10
#define lcdD5   11
#define lcdD6   12
#define lcdD7   13

/*******全局变量定义*******/
int TMPz, TMPf, RHz, RHf;
double Rain, Smog, Light;


double get_ADC_Result(int SensorX)  //使用ADC0832芯片对传感器输出的模拟信号进行转换
{  
    uchar i;  
    uchar dat1=0, dat2=0; 
	double analog;
   
    digitalWrite(ADC_CS, 0);  
    digitalWrite(ADC_CLK,0);  
    digitalWrite(SensorX,1);    delayMicroseconds(2);  
    digitalWrite(ADC_CLK,1);    delayMicroseconds(2);  
   
    digitalWrite(ADC_CLK,0);      
    digitalWrite(SensorX,1);    delayMicroseconds(2);  
    digitalWrite(ADC_CLK,1);    delayMicroseconds(2);  
   
    digitalWrite(ADC_CLK,0);      
    digitalWrite(SensorX,0);    delayMicroseconds(2);//这里若写ADC_DIO为1，则选用通道1进行AD转换  
    digitalWrite(ADC_CLK,1);      
    digitalWrite(SensorX,1);    delayMicroseconds(2);  
    digitalWrite(ADC_CLK,0);      
    digitalWrite(SensorX,1);    delayMicroseconds(2);  
       
    for(i=0;i<8;i++)  
    {  
        digitalWrite(ADC_CLK,1);    delayMicroseconds(2);  
        digitalWrite(ADC_CLK,0);    delayMicroseconds(2);  
   
        pinMode(SensorX, INPUT);  
        dat1=dat1<<1 | digitalRead(SensorX);  
    }  
       
    for(i=0;i<8;i++)  
    {  
        dat2 = dat2 | ((uchar)(digitalRead(SensorX))<<i);  
        digitalWrite(ADC_CLK,1);    delayMicroseconds(2);  
        digitalWrite(ADC_CLK,0);    delayMicroseconds(2);  
    }  
   
    digitalWrite(ADC_CS,1);  
       
    if(dat2==dat1)
	{
		analog = double(dat2)/256*100;
		return analog;
	}
		
}  

int DHT_Read(int pinnumber)//温湿度读取函数
{
	uint32 JYW;
	int i;
	uint32 data = 0;
	
	pinMode(pinnumber, OUTPUT);
	digitalWrite(pinnumber, HIGH);
	delay(2000);
	
	pinMode(pinnumber, OUTPUT);
	digitalWrite(pinnumber, LOW);
	delay(25);//主机把总线拉低必须大于18毫秒，保证DHT11能检测到起始信号
	digitalWrite(pinnumber, HIGH);
	pinMode(pinnumber, INPUT);
	delayMicroseconds(27);//延时等待20-40微秒，开始读取信号
	
	if(digitalRead(pinnumber)==0)
	{
		while(!digitalRead(pinnumber));
		for(i=0;i<32;i++)
		{
			while(digitalRead(pinnumber));
			while(!digitalRead(pinnumber));
			delayMicroseconds(32);
			
			data*= 2;//对二进制数进行左移位操作
			if(digitalRead(pinnumber))
			{
				data++;//读取数据为1，存储在data中
			}			
		}
		for(i=0;i<8;i++)
		{
			while(digitalRead(pinnumber));
			while(!digitalRead(pinnumber));
			delayMicroseconds(32);
			
			JYW*= 2;
			if(digitalRead(pinnumber))
			{
				JYW++;
			}
			
		}
		TMPz = (data>>8)&0xff;
		TMPf = data&0xff;
		RHz  = (data>>24)&0xff;
		RHf  = (data>>16)&0xff;
		JYW  = JYW&0xff;
		if(JYW == (TMPz + TMPf + RHz + RHf))
		{
			return 1;
		}
		else
		{
			return 0;
		}
		
	}
	else
	{
		return 0;
	}
}

int LCD_print(int *fp)
{
		if(*fp==-1)
	{
		printf("LCD display failed!\n");
		return -1;
	}
	delay(1000);
	lcdPosition(*fp, 0, 0);
	lcdPuts(*fp, "Weather Station");
	lcdPosition(*fp, 0, 1);
	lcdPuts(*fp, "  Raspberry Pi  ");
	delay(1000);
}

int data_print()
{
	printf("+-----树莓派气象站------+\n");
	printf("|室内温度： %d.%d℃   |\n",TMPz,TMPf);
	printf("|室内湿度： %d.%d%%  |\n",RHz,RHf);
	printf("|室内采光度： %d%%   |\n",Light);
	printf("|空气颗粒指数： %d%% |\n",Smog);
	printf("|室外阴雨指数： %d%% |\n",Rain);
	printf("+-----------------------+\n");
    return 0;
}

int main ()
{
	wiringPiSetup();
	int i;
	for(i=0;i<16;i++)
	{
	pinMode(i, OUTPUT);
	}
	int fp = lcdInit(lcdRows,lcdCols,lcdBits,lcdRS,lcdEn,lcdD0,lcdD1,lcdD2,lcdD3,lcdD4,lcdD5,lcdD6,lcdD7);
	int *pt;
	pt = &fp;
	LCD_print(pt);
	while(1)
	{
	DHT_Read(DHT11);
	Rain = get_ADC_Result(Rain_D);
	Smog = get_ADC_Result(Smog_D);
	Light = get_ADC_Result(Light_D);
	data_print();
	delay(10000);
 	}
}


