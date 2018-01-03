/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Tx Continuous Wave implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <string.h>
#include "board.h"
#include "radio.h"
#include "spi-board.h"
#include "ic_Swich.h"

#if defined( USE_BAND_868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( USE_BAND_915 )

#define RF_FREQUENCY                                915000000 // Hz

#else

    #error "Please define a frequency band in the compiler options."

#endif

#define TX_OUTPUT_POWER                             14        // 14 dBm
#define TX_TIMEOUT                                  10        // seconds (MAX value)

#define MASTER
//#define SLAVE
uint8_t txBuffer[3] = { 0x01, 0x02, 0x03};
uint8_t rxBuffer[3];

/*!
 * Radio events function pointer
 */

/*!
 * \brief Function executed on Led 1 Timeout event
 */


/*!
 * \brief Function executed on Led 2 Timeout event
 */

/*!
 * \brief Function executed on Led 3 Timeout event
 */


/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnRadioTxTimeout( void )
{
    // Restarts continuous wave transmission when timeout expires
    Radio.SetTxContinuousWave( RF_FREQUENCY, TX_OUTPUT_POWER, TX_TIMEOUT );
}

/**
 * Main application entry point.
 */
int main( void )
{
    // Target board initialization
    BoardInitMcu( );
    BoardInitPeriph( );
#	if defined (MASTER) 
		{
    SpiInit( &SPI_ex.Spi, BLE_OLED_MOSI, BLE_OLED_MISO, BLE_OLED_SCK, 0 );
    SPI_Eint( );
		}
#elif defined (SLAVE) 
		{
		 SpiInit( &SPI_ex.Spi, BLE_OLED_MOSI, BLE_OLED_MISO, BLE_OLED_SCK, NC );
		 SPI_Eint( );
		}
#endif
    // Blink LEDs just to show some activity
    while( 1 )
    {
			txBuffer[1] += 1; 
			#	if defined (MASTER) 
					{
						ic_WriteBuffer( 0x80, txBuffer , sizeof(txBuffer) );
						DelayMs(100);
						GpioToggle(&Led1);
					}
			#elif defined (SLAVE) 
					{
						ic_ReadBuffer( 0x80, rxBuffer, sizeof(rxBuffer) );
					}
			#endif
    }
}
