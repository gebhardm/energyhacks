#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SW_DDR DDRB				// Datenrichtung für Schalterport
#define SW_PORT PORTB			// Port für Schalter
#define SW_PIN PINB				// Lesen der gesetzten Schalter
#define PUMPE PB1				// Pin für Pumpenschalter

#define BUT_DDR	DDRB			// Datenrichtung für Eingabetaster
#define BUT_PORT PORTB			// Port für Taster
#define BUT_PIN PINB			// Lesen der Taster
#define PLUS PB5				// (+)-Taste
#define ENTER PB4				// Eingabetaste
#define MINUS PB3				// (-)-Taste

#define LAUFZEIT 300			// maximale Pumpenlaufzeit in Sekunden
#define DT_AN 10				// Pumpe an bei 10K Differenz
#define DT_AUS 5				// Pumpe aus bei 5K Differenz
#define DT_MAX 10				// Abstand zur Maximaltemperatur für Pumpe aus
#define WW_MIN 40				// Minimaltemperatur für Zirkulation
#define ZI_MAX 40				// Maximaltemperatur für Zirkulation
#define T_DOWN 15				// Temperaturbegrenzung untere Schranke
#define T_UP 95					// Temperaturbegrenzung obere Schranke
#define TCLOCK 206				// Timerstart 50Hz Sek >> 256-50
#define SMPL 5					// Sampling alle SMPL Sekunden

#define LCD_DDR	DDRD			// Datenrichtung für LCD
#define LCD_PORT PORTD			// Port für LCD
#define LCD_PIN PIND			// Lesen des LCD Ports
#define LCD_CLK PD6				// Clock-Pin für Schieberegister
#define LCD_DATA PD5			// Daten-Pin für Schieberegister
#define LCD_LIGHT PD7			// Hintergrundbeleuchtung
#define RS_CMD 0				// Sende Kommando an LCD
#define RS_DATA 1				// Sende Daten an LCD
#define LICHT 60				// Zeit für Hintergrundbeleuchtung
#define MAXMENU 10				// Anzahl der Menüs; letztes immer manuell

volatile unsigned char SampleCnt;		// Zähler bis nächstes Sampling 
volatile int T_Ww, T_Zi;				// gemessene Temperaturen für Warmwasser und Zirkulation
volatile int T_max;						// gemessene Maximaltemperatur
volatile int T_last;					// letzte gemessene Temperatur
volatile int dT_an = DT_AN;				// DeltaTemp für Pumpe an
volatile int dT_aus = DT_AUS;			// DeltaTemp für Pumpe aus
volatile int dT_max = DT_MAX;			// DeltaTemp zu MaxTemp für Pumpe aus
volatile int T_Zi_max = ZI_MAX;			// Maximaltemperatur der Zirkulation
volatile int T_Ww_min = WW_MIN;			// Minimaltemperatur der Zirkulation
volatile unsigned int Lfz = 0;			// Laufzeit der Pumpe
volatile unsigned int t_Lauf = LAUFZEIT;// Maximallaufzeit der Pumpe
volatile unsigned char Bkl = 0;			// Laufzeit Hintergrundbeleuchtung
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
void Schalte_Licht( char );
