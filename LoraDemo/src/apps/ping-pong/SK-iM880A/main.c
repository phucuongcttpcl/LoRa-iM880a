/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Ping-Pong implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <string.h>
#include "board.h"
#include "radio.h"
#include "soft_i2c.h"
#include <stdio.h>
#include "DS18B20.h"
#include "math.h"
#include "uart-board.h"
#include "string.h"
#include "math.h"

#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER                              20        // dBm



#define LORA_BANDWIDTH                              2         // [0: 125 kHz, //2
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       12         // [SF7..SF12] //10
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define Pi 3.14
typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

typedef struct
{
		char lat[11];
		char lon[12];
} GPS_Data;

#define RX_TIMEOUT_VALUE                            10000
#define BUFFER_SIZE                                 5 // Define the payload size here

uint8_t *data;
uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];
uint8_t Buffer2[BUFFER_SIZE];

States_t State = LOWPOWER;
int16_t RssiValue = 0;
int16_t SnrValue = 0;
uint8_t Nsize = 0 ;
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
  #define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /* __GNUC__ */
/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );
/**
 * Main application entry point.
 */
//void I2C_LCD_Init( void);
//void I2C_LCD_Puts(char *sz);
//void I2C_LCD_NewLine(uint8_t loca);
//void I2C_LCD_BackLight(uint8_t u8BackLight);
//void LCD_Write_Chr(char chr);
//void Lcd(uint16_t digit);
//char *itoa(uint8_t n, char *s, uint8_t b);	
//void LCD_Write_4bit(uint8_t u8Data);
//void Mode(void);
//void ModeLcd (void);
void AdcInit( Adc_t *obj, PinNames adcInput );
uint16_t AdcReadChannel( Adc_t *obj );
void UartMcuInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx );
void UartMcuConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl );
void USART_SendData(USART_TypeDef* USARTx, uint16_t Data);
uint16_t USART_ReceiveData(USART_TypeDef* USARTx);
void USART1_IRQHandler( void );
uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data );
uint8_t UartMcuGetChar( Uart_t *obj, uint8_t *data );
float	ConvertData(int8_t );
char* ConverItoC(uint8_t im);
int GPS_Convert(uint8_t raw[], GPS_Data* data);
char* int2array(uint16_t number);

uint16_t dem3=0,dem2=0,dem1=0, DemReset=0;
bool accept=false;
uint8_t GlobalRSSI=0;
uint8_t GlobalSNR=0;
uint8_t GlobalSTT=0;

int main( void )
{
	
    bool isMaster = true;
    
    BoardInitMcu( );
    BoardInitPeriph( );

		UartMcuInit(&Uart1,0, UART_TX, UART_RX );	
		UartMcuConfig( &Uart1, RX_TX, 9600,UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL); 
		// Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
		 
    Radio.Init( &RadioEvents );

    Radio.SetChannel( RF_FREQUENCY );



    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000000 );
	 
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
    
//		/*
//		Receiving data code
//		*/
//		Radio.Rx(0);
//		printf("Hello");
//		/*
//		Sending data code
//		*/
		DelayMs(2000);
		printf("$PMTK314,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
		
		GpioWrite( &Led2, 0 );
		
		printf("Waiting for data...\n");
		
		GPS_Data gpsData;
		
    while( 1 )
    {
				if (gps_received == true)
				{
					char location[32] = "\0";
					char *cCount;
					printf("New GPS received: ");
					GPS_Convert(received_data, &gpsData);
					strcat(location, gpsData.lat);
					strcat(location, ", ");
					strcat(location, gpsData.lon);
					
//					printf("\nLatitude: ");
//					printf (gpsData.lat);
//					printf("\nLongitude: ");
//					printf (gpsData.lon);
//					printf("\n");
					gps_received = false;
					
					dem3++;
					if (dem3 == 500)
					NVIC_SystemReset();
//					if(dem3==0)
//					{
//					dem2++;
//					if(dem2==0) dem1++;
//					}
					
					cCount = int2array(dem3);
					strcat(location, " - ");
					strcat(location, cCount);
					printf(location);
					printf("\n");
					
					// Send the next PING frame            
//					Buffer[0] = 1;
//					Buffer[1] = dem1;		 //dem1
//					Buffer[2] = dem2;		 //dem2
//					Buffer[3] = dem3;		 //dem3
//					Buffer[4] = 0;       //RSSI
					DelayMs( 500 ); 
					Radio.Send( (uint8_t*)location, 32 );
					
					free(cCount);
//					dem3++;
//					if (dem3 == 500)
//						NVIC_SystemReset();
				}
				else
				{
					DelayMs( 1000 );
					printf ("Waiting for GPS\r\n");
//					GpioWrite( &Led2, GpioRead( &Led2 ) ^ 1 );
				}
		}
}
void OnTxDone( void )
{
//  I2C_LCD_Clear( );
//	I2C_LCD_Puts("OnTxDone");
	//Lcd(Buffer[0]);
	//I2C_LCD_Puts("Sleep");
	GpioWrite( &Led2, GpioRead( &Led2 ) ^ 1 );
//			for(int i=0; i<4; i++){
//				printf("%d-",Buffer[i]);
//			}				
//			printf("%d\n",Buffer[4]);	
			Radio.Sleep( );
			State = TX;
}
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
		char location[32] = "\0";	
  //I2C_LCD_Clear( );    
		BufferSize = size;
    memcpy( location, payload, 32 );
    RssiValue = rssi;
	  SnrValue = snr;
    GpioWrite( &Led2, GpioRead( &Led2 ) ^ 1 );
		//I2C_LCD_Puts("RSSI: ");
		//I2C_LCD_Puts("RSSI: ");
		//Lcd(RssiValue);
		//I2C_LCD_Puts(Buffer);
		//I2C_LCD_Puts("OnRxDone");
		//Nsize = strlen((char*)payload);				
			//uint8_t tempRssiValue=0;
			if (RssiValue<0) RssiValue=(uint8_t)-RssiValue;
			Buffer[4]=RssiValue;
			Buffer[0]=1;
//			for(int i=0; i<5;i++)
//			{
//				printf("%d ",Buffer[i]);
//			}
		printf(location);
		printf(" - %d - ",RssiValue);
		printf("%d",SnrValue);
		printf("\n");
//	DemReset++;
//	if (DemReset>100){
//			DemReset=0;
//			NVIC_SystemReset();
//			}
//		State = RX;
}
void Mode (void) {
		//I2C_LCD_Puts(" ID  Temp  State");
		I2C_LCD_Puts("Ham MODE");
}
void ModeLcd (void) {
	
		if(RssiValue < 0) {
			I2C_LCD_NewLine(0xC0); //Xuong dong, bat dau tu vi tri 0
			//Lcd(ConvertData(RssiValue)-15);
			I2C_LCD_Puts("0001");
			
		}
//	if(RssiValue == 0) {
//		I2C_LCD_NewLine(0xC0);
//		//Lcd(65000);
//		I2C_LCD_Puts("0");
//		I2C_LCD_Puts("m");
		
//			Lcd(ConvertData(RssiValue));
			
//		}
		
//		if(SnrValue >-1) {
//			I2C_LCD_NewLine(0xCC);
//			I2C_LCD_Puts(" ");
//			I2C_LCD_NewLine(0xCD);
//			//Lcd(AdcReadChannel(&Adc));
//			Lcd(SnrValue);
//		}
//		else {
//			I2C_LCD_NewLine(0xCC);
//			I2C_LCD_Puts("-");
//			I2C_LCD_NewLine(0xCD);
//			//Lcd(AdcReadChannel(&Adc));
//			Lcd(SnrValue*(-1));
//		}
		
}
void OnTxTimeout( void )
{
  //I2C_LCD_Puts("OnTxTimeout");  
	Radio.Sleep( );
    State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{
	printf("RX Timeout\n");
		//	I2C_LCD_Puts("OnRxTimeout");
		NVIC_SystemReset();
	//Radio.Sleep( );
    State = RX_TIMEOUT;
}

void OnRxError( void )
{
	printf("RX Error\n");
	//		I2C_LCD_Puts("OnRxError");
	NVIC_SystemReset();
    State = RX_ERROR;
}
float ConvertData(int8_t RSSI) {
	float d = 0;
	float Temp = 0 ;
	Temp = (RSSI - 14 + 6 + 20*log10f((4*Pi)/0.34562212))/22;
	d = powl(10,-Temp);
	return d;
}

int GPS_Convert(uint8_t raw[], GPS_Data *data)
{
		int i = 0, lat_index = 0, lon_index = 0;
		
		int count_comma = 1;
		i = 7;

		while (count_comma != 5)
		{
			if (raw[i] == ',')
			{
				i++;
				count_comma++;
			}
			else
			{
				switch (count_comma)
				{
				case 1:
				case 2:
					data->lat[lat_index] = raw[i];
					i++;
					lat_index++;
					break;
				case 3:
				case 4:
					data->lon[lon_index] = raw[i];
					i++;
					lon_index++;
					break;
				}
			}
		}

		data->lat[lat_index] = '\0';
		data->lon[lon_index] = '\0';
		
		return 1;
}

char* int2array(uint16_t number)
{
		uint8_t length = (int)(log10((float)number)) + 1;
    char* reString = (char *) malloc(length * sizeof(char));
		int i = 0;
    do
		{
        reString[length - i - 1] = number % 10 + 48;
        number /= 10;
				i++;
    } while (number != 0);
		
		reString[i] = '\0';
		return reString;
}
////////////////////////////////////////////
GETCHAR_PROTOTYPE
{
  return USART_ReceiveData(USART1);
}

PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART1, (uint8_t) ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {}

  return ch;
}
