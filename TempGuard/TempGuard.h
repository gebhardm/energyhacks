/* TempGuard.h version 3.10.2 */

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
#define DT_ON 10				// pump on at 10 Kelvin difference
#define DT_OFF 5				// pump off at 5 Kelvin difference
#define DT_MAX 10				// difference to maximum temperature at pump off
#define WW_MIN 40				// minimal temperature of circulation
#define CI_MAX 40				// maximum temperature of circulation
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
#define LIGHT 60				// time background light on
#define MAXMENU 10				// number of menus; last one is always "manual control"

volatile unsigned char SampleCnt;		// counter to next sampling
volatile int T_Ww, T_Ci;				// measured temperatures of warm water and circulation
volatile int T_max;						// measured maximum temperature
volatile int T_last;					// last measured temperature
volatile int dT_on = DT_ON;				// DeltaTemp of pump on
volatile int dT_off = DT_OFF;			// DeltaTemp of pump off
volatile int dT_max = DT_MAX;			// DeltaTemp to MaxTemp of pump off
volatile int T_Ci_max = CI_MAX;			// maximum temperature of circulation
volatile int T_Ww_min = WW_MIN;			// minimum temperature of circulation
volatile unsigned int Rnt = 0;			// runtime of pump
volatile unsigned int t_run = RUNTIME;// maximum runtime of pump
volatile unsigned char Bkl = 0;			// runtime background light
volatile struct time_struct {
	unsigned char Hrs:5;
	unsigned char Min:6;
	unsigned char Sec:6;
} Time;

volatile unsigned char Hrs, Min, Sec, Day, Mnt, Yr, Menu;

void init_ADC( void );
int read_ADC( unsigned char );
void init_switch( void );
void init_button( void );
void init_TIMER( void );
void Switch_pump( char );
void Switch_light( char );
void User( void );
void Control_pump( void );

void LCD_clock( void );
void LCD_write_nibble(unsigned char, unsigned char );
void LCD_write(unsigned char, unsigned char );
void LCD_clear( void );
void LCD_home( void );
void LCD_init( void );
void LCD_text( char* );
void LCD_pos( unsigned char, unsigned char );
void LCD_int( int );
