/* TempGuard.h version 3.10.1 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SW_DDR DDRB				// direction of switch port
#define SW_PORT PORTB			// port of switches
#define SW_PIN PINB				// read set switches
#define PUMP PB1				// pin of pump switch

#define BUT_DDR	DDRB			// direction of button port
#define BUT_PORT PORTB			// port of buttons
#define BUT_PIN PINB			// read buttons
#define PLUS PB5				// (+)-button
#define ENTER PB4				// enter button
#define MINUS PB3				// (-)-button

#define RUNTIME 300				// maximum runtime of pump in seconds
#define DT_AN 10				// pump on at 10 Kelvin difference
#define DT_AUS 5				// pump off at 5 Kelvin difference
#define DT_MAX 10				// difference to maximum temperature at pump off
#define WW_MIN 40				// minimal temperature of circulation
#define ZI_MAX 40				// maximum temperature of circulation
#define T_DOWN 15				// temperature lower limit
#define T_UP 95					// temperature upper limit
#define TCLOCK 206				// Timer start 50Hz: Second >> 256-50
#define SMPL 5					// Sampling all SMPL seconds

#define LCD_DDR	DDRD			// direction of LCD port
#define LCD_PORT PORTD			// port of LCD
#define LCD_PIN PIND			// read LCD port
#define LCD_CLK PD6				// clock-pin of shift register
#define LCD_DATA PD5			// data-pin of shift register
#define LCD_LIGHT PD7			// background light
#define RS_CMD 0				// send command to LCD
#define RS_DATA 1				// send data to LCD
#define LICHT 60				// time background light on
#define MAXMENU 10				// number of menus; last one is always "manual control"

volatile unsigned char SampleCnt;		// counter to next sampling
volatile int T_Ww, T_Zi;				// measured temperatures of warm water and circulation
volatile int T_max;						// measured maximum temperature
volatile int T_last;					// last measured temperature
volatile int dT_an = DT_AN;				// DeltaTemp of pump on
volatile int dT_aus = DT_AUS;			// DeltaTemp of pump off
volatile int dT_max = DT_MAX;			// DeltaTemp to MaxTemp of pump off
volatile int T_Zi_max = ZI_MAX;			// maximum temperature of circulation
volatile int T_Ww_min = WW_MIN;			// minimum temperature of circulation
volatile unsigned int Lfz = 0;			// runtime of pump
volatile unsigned int t_Lauf = LAUFZEIT;// maximum runtime of pump
volatile unsigned char Bkl = 0;			// runtime background light
volatile struct time_struct {
	unsigned char Std:5;
	unsigned char Min:6;
	unsigned char Sek:6;
} Zeit;

volatile unsigned char Std, Min, Sek, Tag, Mnt, Jhr, Menu;

void init_ADC( void );
int read_ADC( unsigned char );
void init_switch( void );
void init_button( void );
void init_TIMER( void );
void Schalte_Pumpe( char );
void Schalte_Licht( char );
void Benutzer( void );
void Verwalte_Pumpe( void );

void LCD_clock( void );
void LCD_write_nibble(unsigned char, unsigned char );
void LCD_write(unsigned char, unsigned char );
void LCD_clear( void );
void LCD_home( void );
void LCD_init( void );
void LCD_text( char* );
void LCD_pos( unsigned char, unsigned char );
void LCD_int( int );
