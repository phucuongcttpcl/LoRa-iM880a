/// Viet cho cac loai chip giao tiep SPI




#include "board.h"





void ic_Write( uint8_t addr, uint8_t data );


uint8_t ic_Read( uint8_t addr );


void ic_WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size );


void ic_ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size );

void ic_WriteFifo( uint8_t *buffer, uint8_t size );


void ic_ReadFifo( uint8_t *buffer, uint8_t size );

