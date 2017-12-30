#include "stm32l1xx.h"
#include "soft_i2c.h"
#include "delay.h"
#include "string.h"
static uint8_t u8LCD_Buff[8];//bo nho dem luu lai toan bo
static uint8_t u8LcdTmp;

#define	MODE_4_BIT		0x28
#define	CLR_SCR			0x01
#define	DISP_ON			0x0C
#define	CURSOR_ON		0x0C // 0E point
#define	CURSOR_HOME		0x80

char buf[10];
void LCD_Write_4bit(uint8_t u8Data);
void FlushVal(void);
void I2C_LCD_WriteCmd(uint8_t u8Cmd);
char *strrev(char *);
char *itoa(uint16_t, char *, uint16_t);
void Lcd(uint16_t digit);
char* ConverItoC(uint8_t );
void I2C_LCD_Clear();

void LCD_Clear_Dspl(){
	
}

char *strrev(char *str) {
	char *p1, *p2;

	if (!str || !*str)
		return str;

	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}

	return str;
}

char *itoa(uint16_t n, char *s, uint16_t b) {
	static char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	int i=0, sign;

	if ((sign = n) < 0)
		n = -n;

	do {
		s[i++] = digits[n % b];
	} while ((n /= b) > 0);

	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';

	return strrev(s);
}

void Delay10us(void) {
	uint32_t i;
	for(i=0; i<150; ++i) {
		
	}
}

void FlushVal(void) {
	uint8_t i;
	for(i=0; i<8; ++i) {
		u8LcdTmp>>=1;
		if(u8LCD_Buff[i]) {
			u8LcdTmp|=0x80;
		}
	}
	I2C_Start();
	I2C_Write(I2C_LCD_ADDR);
	I2C_Write(u8LcdTmp);
	I2C_Stop();
}

void I2C_LCD_Init(void) {
	uint8_t i;
	DelayMs(5);
	VX_I2C_Init();	
	for(i=0; i<8; ++i) {
		u8LCD_Buff[i]=0;
	}
	FlushVal();
	u8LCD_Buff[LCD_RS]=0;
	FlushVal();
	u8LCD_Buff[LCD_RW]=0;
	FlushVal();
	LCD_Write_4bit(0x03);
	DelayMs(5);
	LCD_Write_4bit(0x03);
	DelayMs(1);
	LCD_Write_4bit(0x03);
	DelayMs(1);
	LCD_Write_4bit(MODE_4_BIT>>4);
	DelayMs(1);
	I2C_LCD_WriteCmd(MODE_4_BIT);
	I2C_LCD_WriteCmd(DISP_ON);
	I2C_LCD_WriteCmd(CURSOR_ON);
	I2C_LCD_WriteCmd(CLR_SCR);
}

void LCD_Write_4bit(uint8_t u8Data) {
	//4 bit can ghi chinh la 4 5 6 7
	//dau tien gan LCD_E=1
	//ghi du lieu
	//sau do gan LCD_E=0

	if(u8Data&0x08) {
		u8LCD_Buff[LCD_D7]=1;
	} else {
		u8LCD_Buff[LCD_D7]=0;
	}
	if(u8Data&0x04) {
		u8LCD_Buff[LCD_D6]=1;
	} else {
		u8LCD_Buff[LCD_D6]=0;
	}
	if(u8Data&0x02) {
		u8LCD_Buff[LCD_D5]=1;
	} else {
		u8LCD_Buff[LCD_D5]=0;
	}
	if(u8Data&0x01) {
		u8LCD_Buff[LCD_D4]=1;
	} else {
		u8LCD_Buff[LCD_D4]=0;
	}
	
	u8LCD_Buff[LCD_EN]=1;
	FlushVal();	
	
	u8LCD_Buff[LCD_EN]=0;
	FlushVal();
	
}

void LCD_WaitBusy(void) {
	char temp;
	
//	//dau tien ghi tat ca 4 bit thap bang 1
	u8LCD_Buff[LCD_D4]=u8LCD_Buff[LCD_D5]=u8LCD_Buff[LCD_D6]=u8LCD_Buff[LCD_D7]=1;
	FlushVal();
	
	u8LCD_Buff[LCD_RS]=0;
	FlushVal();
	
	u8LCD_Buff[LCD_RW]=1;
	FlushVal();
	do {
		u8LCD_Buff[LCD_EN]=1;
		FlushVal();
		I2C_Start();
		I2C_Write(I2C_LCD_ADDR+1);
		temp=I2C_Read(1);
		I2C_Stop();
		
		u8LCD_Buff[LCD_EN]=0;
		FlushVal();
		u8LCD_Buff[LCD_EN]=1;
		FlushVal();
		u8LCD_Buff[LCD_EN]=0;
		FlushVal();
		//break;
	}while (temp&0x08);
}

void I2C_LCD_WriteCmd(uint8_t u8Cmd) {
	
	LCD_WaitBusy();

	u8LCD_Buff[LCD_RS]=0;
	FlushVal();
	
	u8LCD_Buff[LCD_RW]=0;
	FlushVal();
	
	LCD_Write_4bit(u8Cmd>>4);
	LCD_Write_4bit(u8Cmd);
}

void LCD_Write_Chr(char chr){
	
	LCD_WaitBusy();
	u8LCD_Buff[LCD_RS]=1;
	FlushVal();
	u8LCD_Buff[LCD_RW]=0;
	FlushVal();
	LCD_Write_4bit(chr>>4);
	LCD_Write_4bit(chr);
	
}
//void I2C_LCD_Puts(char *sz) {
//	while(1) {
//		if(*sz) {
//			LCD_Write_Chr(*sz++);
//		} else {
//			break;
//		}
//	}
//}

void I2C_LCD_Puts(char* str) {
  uint8_t i=0;
  while(str[i])
  {
    LCD_Write_Chr(str[i]);
    i++;
  }
}
void I2C_LCD_Clear(void) {
	I2C_LCD_WriteCmd(CLR_SCR);
}

void I2C_LCD_NewLine(uint8_t loca) {
	I2C_LCD_WriteCmd(loca); // Curor 2 location 0xC0
}

void I2C_LCD_BackLight(uint8_t u8BackLight) {
	if(u8BackLight) {
		u8LCD_Buff[LCD_BL]=1;
	} else {
		u8LCD_Buff[LCD_BL]=0;
	}
	FlushVal();
}
void Lcd(uint16_t digit){
	
	  itoa(digit, buf, 10);
		I2C_LCD_Puts(buf);
}
char* ConverItoC(uint8_t im) {
	
	  itoa(im, buf, 10);
		return buf;
}