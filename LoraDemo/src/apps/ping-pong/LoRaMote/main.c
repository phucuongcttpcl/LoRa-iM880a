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
#include <stdio.h>
#include "timer-board.h"

#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER                              20        // dBm



#define LORA_BANDWIDTH                              0         // [0: 125 kHz, //2
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       10         // [SF7..SF12] //10
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

#define RX_TIMEOUT_VALUE                            10000
#define BUFFER_SIZE                                 5 // Define the payload size here

uint8_t *data;
uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];
uint8_t Buffer2[BUFFER_SIZE];
char location[25];

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
void I2C_LCD_Init( void);
void I2C_LCD_Puts(char *sz);
void I2C_LCD_NewLine(uint8_t loca);
void I2C_LCD_BackLight(uint8_t u8BackLight);
void LCD_Write_Chr(char chr);
void Lcd(uint16_t digit);
char *itoa(uint8_t n, char *s, uint8_t b);	
void LCD_Write_4bit(uint8_t u8Data);
void Mode(void);
void ModeLcd (void);
void AdcInit( Adc_t *obj, PinNames adcInput );
uint16_t AdcReadChannel( Adc_t *obj );
void UartMcuInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx );
void UartMcuConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl );
void USART_SendData(USART_TypeDef* USARTx, uint16_t Data);
uint16_t USART_ReceiveData(USART_TypeDef* USARTx);
void USART1_IRQHandler( void );
uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data );
uint8_t UartMcuGetChar( Uart_t *obj, uint8_t *data );
void sentData(void);
float	ConvertData(int8_t );
char* ConverItoC(uint8_t im);

int dem=0;
volatile uint8_t dem3=0,dem2=0,dem1=0, DemReset=0;
bool accept=false;
uint8_t GlobalRSSI=0;
uint8_t GlobalSNR=0;
uint8_t GlobalSTT=0;
TimerTime_t time = 1;
int main( void )
{
	
    bool isMaster = true;
    //uint8_t i;
		//uint16_t j;
		//uint8_t *data2;
		//uint8_t *data3;
//    // Target board initialisation
//	
    BoardInitMcu( );
    BoardInitPeriph( );
//		ds18b20_init();
		//AdcInit(&Adc, PA_1 );
//		I2C_LCD_Init( );
//		I2C_LCD_BackLight(1);
//		Mode();
//		ModeLcd();
		UartMcuInit(&Uart1,0, UART_TX, UART_RX );	
		UartMcuConfig( &Uart1,RX_TX, 9600,UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL); 
////    // Radio initialization
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
    //Radio.Rx(1000);
		//TimerHwInith ();
		//TimerHwStart(100);
		
		GpioWrite( &Led1, 0 );
		DelayMs(2000);
		GpioWrite( &Led1, 1 );
		DelayMs(2000);
    while( 1 )
    {
			DelayMs(1000);
			GpioWrite( &Led2, GpioRead( &Led2 ) ^ 1 );
			GpioWrite( &Led1, GpioRead( &Led1 ) ^ 1 );
			//100s
////			time=TimerHwGetTime();
////			if (time>1000000*60) {
////			//s
////			TimerHwStop();
////			time=0;
////			NVIC_SystemReset();
////			}
       //sentData();
		
		}
}
void OnTxDone( void )
{
//  I2C_LCD_Clear( );
//	I2C_LCD_Puts("OnTxDone");
	//Lcd(Buffer[0]);
	//I2C_LCD_Puts("Sleep");
	GpioWrite( &Led2, GpioRead( &Led2 ) ^ 1 );
			for(int i=0; i<4; i++){
				printf("%d-",Buffer[i]);
			}				
			printf("%d\n",Buffer[4]);	
			Radio.Sleep( );
			State = TX;
}
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
			
  //I2C_LCD_Clear( );    
    BufferSize = size;
    memcpy( location, payload, 25 );
    RssiValue = rssi;
    GpioWrite( &Led2, GpioRead( &Led2 ) ^ 1 );
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
		//I2C_LCD_Puts("Ham MODE");
}
void ModeLcd (void) {
	
		if(RssiValue < 0) {
			//I2C_LCD_NewLine(0xC0); //Xuong dong, bat dau tu vi tri 0
			//Lcd(ConvertData(RssiValue)-15);
			//I2C_LCD_Puts("0001");
			
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
		//	I2C_LCD_Puts("OnRxTimeout");
		NVIC_SystemReset();
	//Radio.Sleep( );
    State = RX_TIMEOUT;
}

void OnRxError( void )
{
	//		I2C_LCD_Puts("OnRxError");
	NVIC_SystemReset();
    State = RX_ERROR;
}
void sentData(void)
{
	dem3++;
				if(dem3==0){
					dem2++;
					if(dem2==0) dem1++;
					}
	// Send the next PING frame            
      Buffer[0] = 1;
			Buffer[1] = dem1;		 //dem1
      Buffer[2] = dem2;		 //dem2
      Buffer[3] = dem3;		 //dem3
      Buffer[4] = 0;       //RSSI
			DelayMs( 500 ); 
			Radio.Send( Buffer, BufferSize );	
}
//void TIM2_IRQHandler( void )
//{
//	static uint32_t time=0;
//    if( TIM_GetITStatus( TIM2, TIM_IT_Update ) != RESET )
//    {
//				if(++time>4000)
//      {

//       GpioWrite( &Led3, GpioRead( &Led3 ) ^ 1 );
//				sentData();
//        time = 0;
//      }
//			
////        TimerIncrementTickCounter( );
//        TIM_ClearITPendingBit( TIM2, TIM_IT_Update );
//    
////        if( TimerTickCounter == TimeoutCntValue )
////        {
////          TimerIrqHandler( );
////        }
//			}
//    
//}

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
