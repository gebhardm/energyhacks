/**********************************************************************
Use an ATtiny 13 to drive the write enable pin of a MAX485 to pass
RS-232 characters for the time of a character write

Used to test the RS-485 interface of an SMA DC converter

Markus Gebhard, Karlsruhe, Copyright 2009

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
**********************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int main()
{
	unsigned char jmprs = 0;
	// init ATtiny13
	// PB0 - RSwrite Output
	// PB1/INT0 - Activity on RS232-TxD-line
	// PB2, 3, 4 - Jumper for Baudrate
	// PB5 unused due to being reset line
	// set PB0 output, all others input
	DDRB |= (1<<PB0);
	// internal Pullup resistors on
	PORTB |= (1<<PB1)|(1<<PB2)|(1<<PB3)|(1<<PB4);
	// /RE on MAX485 to low => read
	PORTB &= ~(1<<PB0);
	// read jumpers for baud setting
	jmprs |= (PINB>>2)&7;
	// main loop to react on RS-232 write
	while (1)
	{
		if (!(PINB&(1<<PB1)))
		{
			PORTB |= (1<<PB0);
			// delay until a character is sent - with some correction
			if 		(jmprs==7)	_delay_us(86);		// 115k2 baud
			else if (jmprs==6)	_delay_us(173-1);	// 57k6
			else if (jmprs==5)	_delay_us(260-2);	// 38k4
			else if (jmprs==4)	_delay_us(520-2);	// 19k2
			else if (jmprs==3)	_delay_us(1041-3);	// 9k6
			else if (jmprs==2)	_delay_us(2083-3);	// 4k8
			else if (jmprs==1)	_delay_us(4166-4);	// 2k4
			else 				_delay_us(8333-5);	// 1k2
			PORTB &= ~(1<<PB0);
		}
	}
	// und tschüß...
	return (0);
}
