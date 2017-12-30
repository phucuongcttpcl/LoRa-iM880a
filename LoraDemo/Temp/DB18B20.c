#include "ds18b20.h"
#include "stm32l1xx_rcc.h"
#include "stm32l1xx_gpio.h"

#define EnableINT()
#define DisableINT()

#define DS_PORT   GPIOA           //DS18B20 
#define DS_DQIO   GPIO_Pin_3     //GPIOD2

#define DS_RCC_PORT  RCC_AHBPeriph_GPIOD

#define DS_PRECISION 0x7f   
#define DS_AlarmTH  0x64
#define DS_AlarmTL  0x8a
#define DS_CONVERT_TICK 1000

#define ResetDQ() GPIO_ResetBits(DS_PORT,DS_DQIO)
#define SetDQ()  GPIO_SetBits(DS_PORT,DS_DQIO)
#define GetDQ()  GPIO_ReadInputDataBit(DS_PORT,DS_DQIO)

uint16_t i = 0;

void Delay_us(uint32_t Nus)
{
 for (i = 0 ; i < Nus*2 ; i++) {
 }
}

unsigned char ResetDS18B20(void)
{
 unsigned char resport;
 SetDQ();
 Delay_us(50);

 ResetDQ();
 Delay_us(500);  //500us 
 SetDQ();
 Delay_us(40);  //40us
 //resport = GetDQ();
 while(GetDQ());
 Delay_us(500);  //500us
 SetDQ();
 return resport;
}

void DS18B20WriteByte(unsigned char Dat)
{
 unsigned char i;
 for(i=8;i>0;i--)
 {
   ResetDQ();     //
  Delay_us(5);    //5us
  if(Dat & 0x01)
   SetDQ();
  else
   ResetDQ();
  Delay_us(65);    //65us
  SetDQ();
  Delay_us(2);    //
  Dat >>= 1;
 }
}


unsigned char DS18B20ReadByte(void)
{
 unsigned char i,Dat;
 SetDQ();
 Delay_us(5);
 for(i=8;i>0;i--)
 {
   Dat >>= 1;
    ResetDQ();     //
  Delay_us(5);   //5us
  SetDQ();
  Delay_us(5);   //5us
  if(GetDQ())
    Dat|=0x80;
  else
   Dat&=0x7f;
  Delay_us(65);   //65us
  SetDQ();
 }
 return Dat;
}


void ReadRom(unsigned char *Read_Addr)
{
 unsigned char i;

 DS18B20WriteByte(ReadROM);

 for(i=8;i>0;i--)
 {
  *Read_Addr=DS18B20ReadByte();
  Read_Addr++;
 }
}


void DS18B20Init(unsigned char Precision,unsigned char AlarmTH,unsigned char AlarmTL)
{
 DisableINT();
 ResetDS18B20();
 DS18B20WriteByte(SkipROM);
 DS18B20WriteByte(WriteScratchpad);
 DS18B20WriteByte(AlarmTL);
 DS18B20WriteByte(AlarmTH);
 DS18B20WriteByte(Precision);

 ResetDS18B20();
 DS18B20WriteByte(SkipROM);
 DS18B20WriteByte(CopyScratchpad);
 EnableINT();

 while(!GetDQ());  //
}


void DS18B20StartConvert(void)
{
 DisableINT();
 ResetDS18B20();
 DS18B20WriteByte(SkipROM);
 DS18B20WriteByte(StartConvert);
 EnableINT();
}

void DS18B20_Configuration(void)
{
 GPIO_InitTypeDef GPIO_InitStructure;

 RCC_AHBPeriphClockCmd(DS_RCC_PORT, ENABLE);

 GPIO_InitStructure.GPIO_Pin = DS_DQIO;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz; //
 GPIO_Init(DS_PORT, &GPIO_InitStructure);
 
 
}


void ds18b20_init(void)
{
 DS18B20_Configuration();
 DS18B20Init(DS_PRECISION, DS_AlarmTH, DS_AlarmTL);
 DS18B20StartConvert();
}


float ds18b20_read(void)
{
	unsigned char DL, DH;
	unsigned short TemperatureData;
	float Temperature;

	DisableINT();
	DS18B20StartConvert();
	ResetDS18B20();
	DS18B20WriteByte(SkipROM);
	DS18B20WriteByte(ReadScratchpad);
	DL = DS18B20ReadByte();
	DH = DS18B20ReadByte();
	EnableINT();

	TemperatureData = DH;
	TemperatureData <<= 8;
	TemperatureData |= DL;

	Temperature = (float)((float)TemperatureData * 0.06); //0.0625

	return  Temperature;
}

