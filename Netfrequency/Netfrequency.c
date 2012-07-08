/* Application to display the net frequency with 9 LEDs
   on a ATtiny2313 running at 12MHz
   (instead of an AT89C2051P as in German Elektor 01-2012,
   pages 22 and following)
   INT0 is used to couple the net frequency
   LEDs on PORTB: PB7-0 and PORTD: PD6
   red:  PB7
   yellow: PB6,5
   green: PB4,3,2
   yellow: PB1,0
   red:  PD6
   
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/***************************************************************************/
//used standard includes 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
/***************************************************************************/
//#define RS232
#ifdef RS232
#define BAUD 19200              // RS232 speed for debugging purposes
#endif
#define NET 50					// set Net default frequency
#define DIVIDER 8				// Prescaler of Timers
#define DELTA 30
// derived values for frequency detection
#define FACTOR (unsigned int) (F_CPU / DIVIDER / NET) // relevant count takts per Hz
#define DELTA2 DELTA / 2
// define the LEDs display frequencies
// example for 50Hz as in Europe:
// Factor = 12.000.000 / 8 / 50 = 30.000 counts equal 50 Hz
// Limits in Europe are 50,2 Hz and 49,8Hz
// 50,2Hz equal 30.120 counts, 49,8Hz equal 29.880 counts
// distributed on 9 LEDs with 50Hz in the center give a delta per LED of 30 counts
#define FRQ_1 (unsigned int) (FACTOR - 4 * DELTA)
#define FRQ_2 (unsigned int) (FACTOR - 3 * DELTA)
#define FRQ_3 (unsigned int) (FACTOR - 2 * DELTA)
#define FRQ_4 (unsigned int) (FACTOR - DELTA)
#define FRQ_5 (unsigned int) (FACTOR)
#define FRQ_6 (unsigned int) (FACTOR + DELTA)
#define FRQ_7 (unsigned int) (FACTOR + 2 * DELTA)
#define FRQ_8 (unsigned int) (FACTOR + 3 * DELTA)
#define FRQ_9 (unsigned int) (FACTOR + 4 * DELTA)
/***************************************************************************/
//Header Definition of used  routined (I don't like header files :-))
#ifdef RS232
void init_USART( unsigned int );
void send_USART( unsigned char );
void sendchars_USART( char* );
void send_int( int );
void send_uint( unsigned int );
#endif
void LED_init( void );
void LED_set( unsigned char );
void LED_clr( unsigned char );
void LED_off( void );
void LED_toggle( unsigned char );
/***************************************************************************/
//Global variables for Interrupt computation
volatile unsigned int  mean;		// mean Timer-value
volatile unsigned long sum;
volatile unsigned char cnt;
/***************************************************************************/
//Interrupt Routines for computing INT0
ISR(INT0_vect)
{
	unsigned int timer;
	timer = TCNT1;
	TCNT1 = 0;
	sum += timer;
	cnt++;
}
// check, if Timer overflows
ISR(TIMER1_OVF_vect)
{
	LED_set(1); LED_set(9);
	_delay_ms(5);
    LED_clr(1); LED_clr(9);
}
/***************************************************************************/
#ifdef RS232
//RS-232 Routines
void init_USART( unsigned int bps )
{
  unsigned int baud;
  baud = ( F_CPU / 16UL / bps ) - 1;       // compute Teiler
  UBRRH = (unsigned char) (baud>>8);       // set Baudrate
  UBRRL = (unsigned char) baud;
  UCSRB = (1<<RXEN)|(1<<TXEN);             // RS232-Rx/Tx on
  UCSRC = (3<<UCSZ0);                      // set Protocol 8N1
}

void send_USART( unsigned char c )
{
  while ( !( UCSRA & (1<<UDRE)) );  // wait until transfer buffer empty
  UDR = c;                          // write char
}

void sendchars_USART( char *s )
{
  while ((*s != 0))
  {                                 // output all chars
     send_USART(*s++);
  } 
}

void send_int( int n )
{
  unsigned char dig[5], num = 0, i; // compute 5 digit integer with sign
  if (n<0) { send_USART('-'); n = -n; }
  if (n<10) send_USART('0');        // leading zero for small numbers
  do
  {
    dig[num++] = ( n % 10 ) + 0x30;
	n = n / 10;
  } while (n != 0);
  for (i=num; i!=0; i--) send_USART(dig[i-1]);
}

void send_uint( unsigned int n )
{
  unsigned char dig[5], num = 0, i; // compute 5 digit integer without sign
  do
  {
    dig[num++] = ( n % 10 ) + 0x30;
	n /= 10;
  } while (n != 0);
  for (i=num; i!=0; i--) send_USART(dig[i-1]);
}
#endif
/***************************************************************************/
// LED Routines
void LED_init( void )
{
   DDRB = 0xff;
   DDRD = (1<<PD6);
   PORTB = 0xff;
   PORTD = (1<<PD6);
}
// LED on
void LED_set( unsigned char n )
{
  if (n<9) PORTB &= ~(1<<(8-n));
  else if (n==9) PORTD &= ~(1<<PD6);
}
// LED off
void LED_clr( unsigned char n )
{
  if (n<9) PORTB |= (1<<(8-n));
  else if (n==9) PORTD |= (1<<PD6);
}
void LED_off( void )
{
	PORTB = 0xff; PORTD = (1<<PD6);
}
void LED_toggle( unsigned char n )
{
  if (n<9) PORTB ^= (1<<(8-n));
  else if (n==9) PORTD ^= (1<<PD6);
}
/***************************************************************************/
// main program
int main( void )
{
  unsigned char i; 		     // counter variable
  // Ausgabe vorbereiten
  #ifdef RS232
  init_USART( BAUD );
  send_USART(27); sendchars_USART("[2J"); // clear screen
  sendchars_USART("Net loupe");
  #endif
  // Net frequecy read preparation
  MCUCR |= (0x02 << ISC00);  // Interrupt on falling edge of INT0
  //MCUCR |= (0x03 << ISC00);  // Interrupt on rising edge on INT0
  GIMSK |= (1 << INT0);   	 // release INT0
  // Timer initialization
  switch (DIVIDER)
  {
  	case 1: TCCR1B |= (0x01 << CS10); break; 
	case 8: TCCR1B |= (0x02 << CS10); break; 
    case 64: TCCR1B |= (0x03 << CS10); break;
  }
  TIMSK  |= (1 << TOIE1);  // Interrupt on timer overflow
  // LEDs initialization
  LED_init();
  // LEDs count through (used just as a appetizer)
  for (i=1;i<=9;i++)
  {
    LED_set(i);
    _delay_ms(250);
	LED_clr(i);
  }
  sum = 0;
  cnt = 0;
  // Frequency measuring preparation
  sei();                    // activate Interrupt control
  TCNT1 = 0;         		// Timer start
  // program main loop - interrupt delivers mean value to display
  while (1)
  {
  	if (cnt == NET)
	{
		mean = (unsigned int) (sum / cnt);
		cnt = 0; sum = 0;
		#ifdef RS232
		send_uint(mean); send_USART(0x0d); send_USART(0x0a);
		#endif
		LED_off();
		if ((mean >= (FRQ_1 - DELTA2)) && (mean < (FRQ_1 + DELTA2))) LED_set(1);
		if ((mean >= (FRQ_2 - DELTA2)) && (mean < (FRQ_2 + DELTA2))) LED_set(2);
		if ((mean >= (FRQ_3 - DELTA2)) && (mean < (FRQ_3 + DELTA2))) LED_set(3);
		if ((mean >= (FRQ_4 - DELTA2)) && (mean < (FRQ_4 + DELTA2))) LED_set(4);
		if ((mean >= (FRQ_5 - DELTA2)) && (mean < (FRQ_5 + DELTA2))) LED_set(5);
		if ((mean >= (FRQ_6 - DELTA2)) && (mean < (FRQ_6 + DELTA2))) LED_set(6);
		if ((mean >= (FRQ_7 - DELTA2)) && (mean < (FRQ_7 + DELTA2))) LED_set(7);
		if ((mean >= (FRQ_8 - DELTA2)) && (mean < (FRQ_8 + DELTA2))) LED_set(8);
		if ((mean >= (FRQ_9 - DELTA2)) && (mean < (FRQ_9 + DELTA2))) LED_set(9);
	}
  }
  return(0);
}