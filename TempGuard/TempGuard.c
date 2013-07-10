/***** Temperature Guard to control a circulation pump *****************
By two KTY81 temperature sensors the outgoing and circulation
temperature of the warm water supply is measured and at a defined
difference the circulation pump is activated

Uses an ATmega8

Copyright 2009-2013 Markus Gebhard, Karlsruhe, Germany

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
************************************************************************/
#include "TempGuard.h"

// Implementations
/***************************  ADC - Routinen ***************************/
void init_ADC( void )
{
	ADCSRA = (1<<ADEN)|(3<<ADPS1); // Wandler ein, Taktteiler :64
}

// read analog value
int read_ADC( unsigned char channel )
{
	ADMUX  =  (1<<REFS0)|channel; 	// setze ADC Eingang gegen ARef=AVcc
	ADCSRA |= (1<<ADSC);          	// starte Wandlung
	while (ADCSRA & (1<<ADSC)) { }	// warte bis Wandlung abgeschlossen
	return((int)(55*ADCW/100-180)); // KTY81-110 linear gegen 1k8
}

/*********************Begin of LCD routines***************************/
void LCD_clock( void )
{
	LCD_PORT |= (1<<LCD_CLK);
	_delay_us(2);
	LCD_PORT &= ~(1<<LCD_CLK);
	_delay_us(2);
}

void LCD_write_nibble(unsigned char rs, unsigned char d)
{
	unsigned char n, mask;
	LCD_PORT &= ~(1<<LCD_CLK);
	LCD_PORT &= ~(1<<LCD_DATA);
	// clear all stages of shift register
	for (n=8;n;n--) LCD_clock();
	// set Enable at Q7
	LCD_PORT |= (1<<LCD_DATA);
	LCD_clock();
	// set rs at Q6
	if (rs) LCD_PORT |= (1<<LCD_DATA); else LCD_PORT &= ~(1<<LCD_DATA);
	LCD_clock();
	// shift in 4 bits
	mask = 8;
	for (n=4;n;n--)
	{
		if (d & mask) LCD_PORT |= (1<<LCD_DATA);
		else LCD_PORT &= ~(1<<LCD_DATA);
		LCD_clock();
		mask >>= 1;
	}
	// shift in 0-bit
	LCD_PORT &= ~(1<<LCD_DATA);
	LCD_clock();
	// set Enable
	LCD_PORT |= (1<<LCD_DATA);
	LCD_PORT &= ~(1<<LCD_DATA);
}

void LCD_write(unsigned char rs, unsigned char d)
{
	LCD_write_nibble(rs, d >> 4);
	LCD_write_nibble(rs, d & 0xF);
}

void LCD_clear( void )
{
	LCD_write( RS_CMD, 0x01 );	// display clear
	_delay_ms(2);
}

void LCD_home( void )
{
	LCD_write( RS_CMD, 0x02 );	// display home
	_delay_ms(2);
}

void LCD_init( void )
{
	LCD_DDR |= (1<<LCD_DATA)|(1<<LCD_CLK)|(1<<LCD_LIGHT);
	// Initialization corresponding to HITACHI specification for HD44780U
	_delay_ms(200);						// wait to power up
	LCD_write_nibble( RS_CMD, 0x3 );	// Function set (IF is 8 bit long)
	_delay_ms(20);
	LCD_write_nibble( RS_CMD, 0x3 );	// Function set (IF is 8 bit long)
	_delay_ms(2);
	LCD_write_nibble( RS_CMD, 0x3 );	// Function set (IF is 8 bit long)
	_delay_ms(2);
	LCD_write_nibble( RS_CMD, 0x2 );	// Function set (IF set to 4 bit)
	_delay_ms(2);
	LCD_write( RS_CMD, 0x28 );			// IF 4bit, 2 lines
	_delay_ms(2);
	//LCD_write( RS_CMD, 0x0e );			// Display on, cursor on, blinking off
	LCD_write( RS_CMD, 0x0c );			// Display on, cursor off, blinking off
	_delay_ms(2);
	LCD_write( RS_CMD, 0x01 );			// Display clear
	_delay_ms(2);
	LCD_write( RS_CMD, 0x06 );			// Entry mode increment
	_delay_ms(2);
	// End of initialization
}

void LCD_text( char* s )
{
	while (*s) LCD_write( RS_DATA, *s++ );
}

void LCD_pos( unsigned char x, unsigned char y )
{
	unsigned char pos;
	pos = 0x7F + x;
	if (y==2) pos += 0x40;
	LCD_write( RS_CMD, pos );
}

void LCD_int( int n )
{
	unsigned char dig[5], num = 0, i; // compute 5 digit number
	if (n<0) { LCD_text("-"); n = -n; }
	if (n<10) LCD_text("0");        // send leading zero
	do
	{
		dig[num++] = ( n % 10 ) + 0x30;
		n = n / 10;
	} while (n != 0);
	for (i=num;i;i--) LCD_write( RS_DATA, dig[i-1]);
	//LCD_write( RS_DATA, 0x20 );
}

void Schalte_Licht( char status )
{
	if (status) LCD_PORT |= (1<<LCD_LIGHT);
	else LCD_PORT &= ~(1<<LCD_LIGHT);
}
/*********************End of LCD routines*****************************/

/************************** switch and control ************************/
void init_switch( void )
{
	// PUMP output
	SW_DDR |= (1<<PUMPE);
	// Pump off
	Schalte_Pumpe(0);
}

void init_button( void )
{
	BUT_DDR &= ~((1<<PLUS)|(1<<ENTER)|(1<<MINUS));
	BUT_PORT |= (1<<PLUS)|(1<<ENTER)|(1<<MINUS);    // internal Pullup
}

void Schalte_Pumpe( char status )
{
	if (status) SW_PORT |= (1<<PUMPE);
	else SW_PORT &= ~(1<<PUMPE);
}


/*********************** Interruptverarbeitung ***********************/
void init_TIMER( void )
{
	// Set Timer0 (external trigger at T0/PD4)
	//TCCR0 |= (1<<CS02)|(1<<CS01)|(1<<CS00); // Ext.Takt T0, steigende Flanke
	TCCR0 |= (1<<CS02)|(1<<CS01); // Ext.Takt T0, falling edge
	TCNT0 = TCLOCK;
	// Timer Interrupt
	TIMSK  |= (1<<TOIE0); // Interrupt Timer0 overflow
}

// interrupt routine on timer0 overflow
SIGNAL(SIG_OVERFLOW0)
{
	TCNT0 = TCLOCK;   // reset Timer0
	// compute time
	Zeit.Sek++;
	if (Zeit.Sek==60) { Zeit.Sek=0; Zeit.Min++; }
	if (Zeit.Min==60) { Zeit.Min=0; Zeit.Std++; }
	if (Zeit.Std==24) Zeit.Std=0;
	// seconds backlight on
	if (LCD_PIN & (1<<LCD_LIGHT)) Bkl++; else Bkl=0;
	// seconds pump on
	if (SW_PIN & (1<<PUMPE)) Lfz++; else Lfz=0;
	// control pump by temperature if not in manual mode
	if (Menu!=MAXMENU)
	{
		SampleCnt++;
		if (SampleCnt==SMPL)
		{
			SampleCnt = 0;
			Verwalte_Pumpe();
		}
	}
}

/************************************ Schaltwerk *************************/
void Verwalte_Pumpe( void )
{
	// lokale Variablen
	int deltaTwz, deltaTww, deltaTmax;
	char Pumpe = 0;
	// Temperaturmessung
	T_Ww = read_ADC(0x00);  // lese Analogwert von ADC0 - Warmwasser
	T_Zi = read_ADC(0x01);  // lese Analogwert von ADC1 - Zirkulation
	// Temperaturdifferenzen
	deltaTwz = T_Ww - T_Zi;			// Warmwasser zu Zirkulation
	deltaTww = T_Ww - T_last;		// Warmwasser zur letzten Messung
	if (T_max < T_Ww) T_max = T_Ww;
	deltaTmax = T_max - T_Ww;		// Warmwasser zur Maximaltemperatur
	/******* Anschaltkriterium für Pumpe *******/
	if ((T_Ww > T_Zi) && 
		(T_Ww >= T_Ww_min) && 
		(T_Zi < T_Zi_max) &&
		(deltaTwz >= dT_an)) { Pumpe = 1; }
	if ((SW_PIN & (1<<PUMPE)) && (deltaTwz > dT_aus)) Pumpe = 1;
	/******* Abschaltkriterien *******/
	// Zirkulationsleitung warm genug
	if (T_Zi >= T_Zi_max) Pumpe = 0;
	// Pumpenlaufzeit überschritten
	if (Lfz >= t_Lauf) {
	   Pumpe = 0;
	   if (Menu == MAXMENU) Menu = 1; // return to automatic mode 
	}
	// Warmwasser kühlt ab
	if ((deltaTww < 0) || (deltaTmax > 0)) Pumpe = 0;
	// Warmwasserleitung abgekühlt: T_max zurücksetzen
	if (T_Ww < T_Ww_min) T_max = T_Ww;
	// Temperatur sichern
	T_last = T_Ww;
	// Schalten
	if (Pumpe) Schalte_Pumpe(1); else Schalte_Pumpe(0);
}

/************************************ Benutzerführung ********************/
void Benutzer( void )
{
	unsigned char buttons, nokey;
	nokey   = ((1<<MINUS)|(1<<ENTER)|(1<<PLUS));
	buttons = (BUT_PIN & nokey);
	// Wertausgabe an LCD
	LCD_home();
	// Licht einschalten bei Tastendruck
	if (buttons != nokey)
	{
		Schalte_Licht(1);
		Bkl = 0;
		// Grundwerte ändern
		if (!(BUT_PIN & (1<<ENTER)))
		{
			Menu++; 
			if (Menu>MAXMENU) Menu = 1; 
			LCD_clear();
		}
	}
	switch (Menu)
	{
	// Temperaturanzeige
	case 1: 
		LCD_text("Warm: "); LCD_int(T_Ww);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");
		LCD_pos(1,2);
		LCD_text("Zirk: "); LCD_int(T_Zi);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");   
		break;
	// Sollwert dT_an
	case 2: 
		if (!(BUT_PIN & (1<<PLUS))) dT_an++;
		else if (!(BUT_PIN & (1<<MINUS))) dT_an--;
		LCD_text("*dT(an): "); LCD_int(dT_an);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");
		LCD_pos(1,2);
		LCD_text(" dT(aus):"); LCD_int(dT_aus);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");   
		break;
	// Sollwert dT_aus
	case 3: 
		if (!(BUT_PIN & (1<<PLUS))) dT_aus++;
		else if (!(BUT_PIN & (1<<MINUS))) dT_aus--;
		LCD_text(" dT(an): "); LCD_int(dT_an);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");
		LCD_pos(1,2);
		LCD_text("*dT(aus):"); LCD_int(dT_aus);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");   
		break;
	// Uhr
	case 4:
		if (!(BUT_PIN & (1<<PLUS)))
		{
			Zeit.Min++; if (Zeit.Min==60) Zeit.Min=0;
			Zeit.Sek=0;
		}
		if (!(BUT_PIN & (1<<MINUS)))
		{
			Zeit.Std++; if (Zeit.Std==24) Zeit.Std=0;
		}
		LCD_text("Uhrzeit ");
		LCD_pos(1,2);
		LCD_int(Zeit.Std); LCD_text(":");
		LCD_int(Zeit.Min); LCD_text(":");
		LCD_int(Zeit.Sek); LCD_text("   ");
		break;
	// Maximale Zirkulationstemperatur
	case 5:
		if (!(BUT_PIN & (1<<PLUS))) T_Zi_max++;
		else if (!(BUT_PIN & (1<<MINUS))) T_Zi_max--;
		if (T_Zi_max > T_UP) T_Zi_max = T_UP;
		if (T_Zi_max < T_DOWN) T_Zi_max = T_DOWN;
		LCD_text("*ZirkMax: ");
		LCD_int(T_Zi_max);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");
		LCD_pos(1,2);
		LCD_text(" WwMin: "); LCD_int(T_Ww_min);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");   
		break;
	// Minimal Warmwassertemperatur
	case 6:
		if (!(BUT_PIN & (1<<PLUS))) T_Ww_min++;
		else if (!(BUT_PIN & (1<<MINUS))) T_Ww_min--;
		if (T_Ww_min > T_UP) T_Ww_min = T_UP;
		if (T_Ww_min < T_DOWN) T_Ww_min = T_DOWN;
		LCD_text(" ZirkMax: ");
		LCD_int(T_Zi_max);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");
		LCD_pos(1,2);
		LCD_text("*WwMin: "); LCD_int(T_Ww_min);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");   
		break;
	// Reset
	case 7:
		if (!(BUT_PIN & ((1<<PLUS)|(1<<MINUS))))
		{
			dT_an = DT_AN;
			dT_aus = DT_AUS;
			T_Zi_max = ZI_MAX;
			T_Ww_min = WW_MIN;
			T_max = T_Ww;
			dT_max = DT_MAX;
		}
		LCD_text("Reset");
		LCD_pos(1,2);
		LCD_text("an "); LCD_int(dT_an);
		LCD_write(RS_DATA,0xdf); LCD_text("C aus "); LCD_int(dT_aus);
		LCD_write(RS_DATA,0xdf); LCD_text("C ");
		break;
	// Anzeige der gemessenen Maximaltemperatur
	case 8:
		if (!(BUT_PIN & ((1<<PLUS)|(1<<MINUS)))) T_max = 0;
		LCD_text("*MaxTemp: "); LCD_int(T_max);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");
		LCD_pos(1,2);
		LCD_text(" dT_max: "); LCD_int(dT_max);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");   
		break;
	// Abstand bis Maximaltemperatur
	case 9:
		if (!(BUT_PIN & (1<<PLUS))) dT_max++;
		else if (!(BUT_PIN & (1<<MINUS))) dT_max--;
		LCD_text(" MaxTemp: "); LCD_int(T_max);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");
		LCD_pos(1,2);
		LCD_text("*dT_max: "); LCD_int(dT_max);
		LCD_write(RS_DATA,0xdf); LCD_text("C  ");   
		break;
	// Manuelle Steuerung
	case MAXMENU:
		if (!(BUT_PIN & (1<<PLUS))) Schalte_Pumpe(1);
		else if (!(BUT_PIN & (1<<MINUS))) Schalte_Pumpe(0);
		LCD_text("Manuell ");
		if (SW_PIN & (1<<PUMPE)) LCD_text("an "); else LCD_text("aus");
		//unterdrücke Rücksprung ins Menü 1
		Bkl = 0;
		break;
	}
	//Delta-Temperaturen bereinigen: nicht kleiner als Null und Aus > Ein
	if (dT_aus < 1) dT_aus = 0;
	if (dT_an <= dT_aus) dT_an = dT_aus + 1;
	if (dT_max < 0) dT_max = 0; 
	//Hintergrundbeleuchtung
	if (Bkl>=LICHT) 
	{
		Schalte_Licht(0);	// Nach Laufzeit abschalten
		Menu = 1;			// Temperaturanzeige
	}
}

/***************************** Hauptprogramm ***************************/
int main( void )
{
	// Initialisierung
	init_ADC();
	init_switch();
	init_button();
	Menu = 1;
	// Anzeigesteuerung
	LCD_init();
	LCD_text("Version 3.10.1");
	_delay_ms(1000);
	LCD_clear();
	// Timer und Interrupts
	init_TIMER();
	sei();
	// Programm-Hauptschleife
	while (1)
	{
		_delay_ms(250);
		Benutzer();
	}
	return(0);
}
