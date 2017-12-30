#include "stm32l1xx.h"
#include "soft_i2c.h"

void VX_I2C_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_Init(GPIOB,&GPIO_InitStructure);	
	SDA_1;
	SCL_1;
	Delay10us();
	Delay10us();
}

void I2C_Start(void) {
	SDA_1;
	Delay10us();
	SCL_1;
	Delay10us();
	SDA_0;
	Delay10us();
	SCL_0;
	Delay10us();
}

void I2C_Stop(void) {
	SCL_0;
	Delay10us();
	SDA_0;
	Delay10us();
	SCL_1;
	Delay10us();
	SDA_1;
	Delay10us();
}

uint8_t I2C_Write(uint8_t u8Data) {
	uint8_t i, ret;
	for(i=0; i<8; ++i) {
		if(u8Data&0x80) {
			SDA_1;
		} else {
			SDA_0;
		}
		Delay10us();
		SCL_1;
		Delay10us();
		Delay10us();
		SCL_0;
		Delay10us();
		u8Data<<=1;
	}
	//kiem tra ack
	SDA_1;
	Delay10us();
	SCL_1;
	Delay10us();
	if(SDA_VAL) {
		ret=1;
	} else {
		ret=0;
	}
	Delay10us();
	SCL_0;
	Delay10us();
	return ret;
}

uint8_t I2C_Read(uint8_t u8Ack) {
	uint8_t i, ret;
	for(i=0; i<8; ++i) {
		ret<<=1;
		Delay10us();
		SCL_1;
		Delay10us();
		if(SDA_VAL) {
			ret|=0x01;
		}
		Delay10us();
		SCL_0;
		Delay10us();
	}
	//kiem tra ack
	if(u8Ack) {
		SDA_1;
	} else {
		SDA_0;
	}
	Delay10us();
	SCL_1;
	Delay10us();
	Delay10us();
	SCL_0;
	Delay10us();
	return ret;
}
