/*
BackyardBrains 18. Dec. 2019
Used for Muscle SpikerBox 
Samples EMG data at 10kHz and packs and sends data according to custom protocol via serial  
                     ____________
               VCC--|1         14|--GND
  TPI CLK      PA0--|2         13|--PB3 RX <--USART_Receive()
 TPI DATA      PA1--|3         12|--PB2 TX -->USART_Transmit()
(TPI RESET)    PA2--|4         11|--PB1 
               PA3--|5         10|--PB0 
               PA4--|6          9|--PA7
 ADC input     PA5--|7          8|--PA6 debug
                     \__________/
             Atmel ATtiny104 
Written by Stanislav Mircic
*/

#define F_CPU 11059000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define BAUD 230400
#define MYUBRR F_CPU/8/BAUD-1        // +1, 0 or -1 to adjust the timming.
uint8_t sendMessage = 0;
void USART_Init( unsigned int ubrr){  // Initialize USART
  CCP = (0b11011000);//pass to access  protected I/O register
  CLKMSR = (0b00000010);//use external clock
  CCP = (0b11011000);//pass to access  protected I/O register
  CLKPSR = (0b00000000);//prescaler protected register

  ubrr = 5;//Atmel ATtiny102 / ATtiny104 [DATASHEET] page 107
  UBRRH = (unsigned char)(ubrr>>8); // Set the baud rate
  UBRRL = (unsigned char)ubrr;
  UCSRA = (0b00000010);//double speed 
  UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);      // Enable Receiver and Transmitter and receiver interrupt
  UCSRC = (0b00000110);     // Set Format: 8 data, 2 stop bit Asynchronous USART, parity disabled
  sei();
  }

  
ISR(USART_RXC_vect) {
	uint8_t digit1; 
	while ( UCSRA & (1<<RXC) ){
		digit1 = UDR;
		if(digit1 =='b')
		{
			sendMessage = 1;
		}
	};
}

void USART_Transmit( unsigned char data ){
  while ( !( UCSRA & (1<<UDRE)) );  
  UDR = data;                       
}

unsigned char USART_Receive( void ){
  while ( !(UCSRA & (1<<RXC)) );    
  return UDR;                       
}

void USART_Flush( void ){
  while ( UCSRA & (1<<RXC) ){};
}

void ADC_Init( void ){         


 //Set Voltage reference
  ADMUX =
            (0 << REFS1) |     // VCC as V. reference, bit 1
            (0 << REFS0) |     // VCC as V. reference, bit 0
            (0 << MUX2)  |     // use ADC3 (PA6), MUX bit 2
            (1 << MUX1)  |     // use ADC3 (PA6), MUX bit 1
            (0 << MUX0);       // use ADC3 (PA6), MUX bit 0
      
  //Set prescaler 
  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // Set prescaler= 16 b100 = 16; b110 = 64
            (0 << ADPS1) |     // Set prescaler=16 
            (0 << ADPS0);      // Set prescaler=16 
      
}

//-----------------------------------------------------------------------------

int main( void ){
  USART_Init(MYUBRR);
  ADC_Init();
  uint16_t data1;
  uint16_t data2;
  uint16_t data3;
  uint8_t digit1; 
  uint8_t digit2; 

  DDRA |= (1 << PINA6);
  while(1){
	if(sendMessage)
	{
		//send message for board
		sendMessage = 0;
		USART_Transmit( 255 ); 
		USART_Transmit( 255 ); 
		USART_Transmit( 1 ); 
		USART_Transmit( 1 ); 
		USART_Transmit( 128 ); 
		USART_Transmit( 255 ); 
		USART_Transmit( 'H' ); 
		USART_Transmit( 'W' ); 
		USART_Transmit( 'T' ); 
		USART_Transmit( ':' ); 
		USART_Transmit( 'M' ); 
		USART_Transmit( 'U' ); 
		USART_Transmit( 'S' ); 
		USART_Transmit( 'C' ); 
		USART_Transmit( 'L' ); 
		USART_Transmit( 'E' ); 
		USART_Transmit( 'S' ); 
		USART_Transmit( 'S' ); 
		USART_Transmit( ';' ); 
		USART_Transmit( 255 ); 
		USART_Transmit( 255 ); 
		USART_Transmit( 1 ); 
		USART_Transmit( 1 ); 
		USART_Transmit( 129 ); 
		USART_Transmit( 255 ); 

	}
	else
	{
		PORTA |= (0b01000000);
		ADCSRA |= (1 << ADSC);          // start ADC measurement
		while (ADCSRA & (1 << ADSC) );  // wait till conversion complete 
		
		
		data1= ADCL;
		data2 = ADCH;
		data3 = data1+(data2<<8);       //format frame
		digit2 = data3 & 0x7F; 
		digit1 = (data3>>7)|0x80; 
		USART_Transmit( digit1 );       // Transmit sample
		USART_Transmit( digit2 ); 
		PORTA &= (0b10111111);
		//pause for a while to get to 10kHz
		data3 = 0;
		while(data3<200)
		{
			data3++;
		}
	}
   }

}  // End of main.
