#ifndef _KT_SOFT_I2C_
	#define _KT_SOFT_I2C_
	#include "stm32l1xx.h"
	
	#define SDA_1 GPIO_SetBits(GPIOB, GPIO_Pin_9)
	#define SDA_0 GPIO_ResetBits(GPIOB, GPIO_Pin_9)
	#define SCL_1 GPIO_SetBits(GPIOB, GPIO_Pin_8)
	#define SCL_0 GPIO_ResetBits(GPIOB, GPIO_Pin_8)
	#define SDA_VAL GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9)
	
	void Delay10us(void);
	#define I2C_LCD_ADDR 0x4E
	#define LCD_EN 2
	#define LCD_RW 1
	#define LCD_RS 0
	#define LCD_D4 4
	#define LCD_D5 5
	#define LCD_D6 6
	#define LCD_D7 7
	#define LCD_BL 3
	void I2C_Start(void);
	void I2C_Stop(void);
	uint8_t I2C_Write(uint8_t u8Data);
	uint8_t I2C_Read(uint8_t u8Ack);
	void VX_I2C_Init(void);
#endif
