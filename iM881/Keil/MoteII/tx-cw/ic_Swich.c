////////////////

#include "ic_Swich.h"


void ic_Write( uint8_t addr, uint8_t data )
{
    ic_WriteBuffer( addr, &data, 1 );
}

uint8_t ic_Read( uint8_t addr )
{
    uint8_t data;
    ic_ReadBuffer( addr, &data, 1 );
    return data;
}

void ic_WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    GpioWrite( &SPI_ex.Spi.Nss, 0 );

    SpiInOut( &SPI_ex.Spi, addr | 0x80 );
    for( i = 0; i < size; i++ )
    {
        SpiInOut( &SPI_ex.Spi, buffer[i] );
    }

    //NSS = 1;
    GpioWrite( &SPI_ex.Spi.Nss, 1 );
}

void ic_ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    GpioWrite( &SPI_ex.Spi.Nss, 0 );

    SpiInOut( &SPI_ex.Spi, addr & 0x7F );

    for( i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( &SPI_ex.Spi, 0 );
    }

    //NSS = 1;
    GpioWrite( &SPI_ex.Spi.Nss, 1 );
}

void ic_WriteFifo( uint8_t *buffer, uint8_t size )
{
    ic_WriteBuffer( 0, buffer, size );
}

void ic_ReadFifo( uint8_t *buffer, uint8_t size )
{
    ic_ReadBuffer( 0, buffer, size );
}
