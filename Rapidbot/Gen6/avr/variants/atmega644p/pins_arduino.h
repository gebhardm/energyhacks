/* 
This version of the pins_arduino.h file is for use
with Gen6 3D-printer driver board as supplied by
Make Mendel; uses bootload like Arduino Nano
Adapted May 2015 by Markus Gebhard, Karlsruhe
*/

/*
pins_arduino.h - Pin definition functions for Arduino
Part of Arduino - http://www.arduino.cc/

Copyright (c) 2007 David A. Mellis

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General
Public License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA  02111-1307  USA

$Id: wiring.h 249 2007-02-03 16:52:51Z mellis $

Changelog
-----------
11/25/11  - ryan@ryanmsutton.com - Add pins for Sanguino 644P and 1284P
07/15/12  - ryan@ryanmsutton.com - Updated for arduino0101
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <avr/pgmspace.h>

// ATMEL ATMEGA644P
//
// Open pins
// PA0(31), PB0(0), PB1(1), PC5(21)
//
// Built-in LED on pin 8
//
// SDA PC1(17)
// SCL PC0(16)
// 
// TX PD1(9)
// RX PD0(8)
//
// RS485
// R RX1 PD2(10), RE PD5(13)
// D TX1 PD3(11), DE PD4(12)
//
// Opto
// X PC4(20), Y PA(6)(25), Z PA1(30), Ext n/a
//
// Heat PD6(14), Therm PA5(26/AI5)
//
// Thermocoupler is 100k on 4k7 pull-up
//
// Motor control on DRV8811
//
//        X        Y        Z        Ext
// EN    PC3(19)  PA7(24)  PA2(29)  PB3(3)
// STEP  PD7(15)  PC7(23)  PA4(27)  PB4(4)
// DIR   PC2(18)  PC6(22)  PA3(28)  PB2(2)

#define NOT_A_PIN 0
#define NOT_A_PORT 0

#define NOT_ON_TIMER 0
#define TIMER0A 1
#define TIMER0B 2
#define TIMER1A 3
#define TIMER1B 4
#define TIMER2A 5
#define TIMER2B 6

const static uint8_t SS   = 4;
const static uint8_t MOSI = 5;
const static uint8_t MISO = 6;
const static uint8_t SCK  = 7;

static const uint8_t SDA = 17;
static const uint8_t SCL = 16;
static const uint8_t LED_BUILTIN = 8;

static const uint8_t A0 = 24;
static const uint8_t A1 = 25;
static const uint8_t A2 = 26;
static const uint8_t A3 = 27;
static const uint8_t A4 = 28;
static const uint8_t A5 = 29;
static const uint8_t A6 = 30;
static const uint8_t A7 = 31;

#define NUM_DIGITAL_PINS            32
#define NUM_ANALOG_INPUTS           8
#define analogInputToDigitalPin(p)  ((p < 8) ? (p) + 24 : -1)

#define digitalPinHasPWM(p)         ((p) == 3 || (p) == 4 || (p) == 12 || (p) == 13 || (p) == 14 || (p) == 15 )

#define PA 1
#define PB 2
#define PC 3
#define PD 4

#ifdef ARDUINO_MAIN
// these arrays map port names (e.g. port B) to the
// appropriate addresses for various functions (e.g. reading
// and writing)
const uint16_t PROGMEM port_to_mode_PGM[] =
{
	NOT_A_PORT,
	(uint16_t) &DDRA,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
};

const uint16_t PROGMEM port_to_output_PGM[] =
{
	NOT_A_PORT,
	(uint16_t) &PORTA,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
};
const uint16_t PROGMEM port_to_input_PGM[] =
{
	NOT_A_PORT,
	(uint16_t) &PINA,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
};
const uint8_t PROGMEM digital_pin_to_port_PGM[] =
{
	PB, /* 0 */
	PB,
	PB,
	PB,
	PB,
	PB,
	PB,
	PB,
	PD, /* 8 */
	PD,
	PD,
	PD,
	PD,
	PD,
	PD,
	PD,
	PC, /* 16 */
	PC,
	PC,
	PC,
	PC,
	PC,
	PC,
	PC,
	PA, /* 24 */
	PA,
	PA,
	PA,
	PA,
	PA,
	PA,
	PA  /* 31 */
};
const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] =
{
	_BV(0), /* 0, port B */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0), /* 8, port D */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0), /* 16, port C */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0), /* 24, port A */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7)
};
const uint8_t PROGMEM digital_pin_to_timer_PGM[] =
{
	NOT_ON_TIMER,   /* 0  - PB0 */
	NOT_ON_TIMER,   /* 1  - PB1 */
	NOT_ON_TIMER,   /* 2  - PB2 */
	TIMER0A,        /* 3  - PB3 */
	TIMER0B,        /* 4  - PB4 */
	NOT_ON_TIMER,   /* 5  - PB5 */
	NOT_ON_TIMER,   /* 6  - PB6 */
	NOT_ON_TIMER,   /* 7  - PB7 */
	NOT_ON_TIMER,   /* 8  - PD0 */
	NOT_ON_TIMER,   /* 9  - PD1 */
	NOT_ON_TIMER,   /* 10 - PD2 */
	NOT_ON_TIMER,   /* 11 - PD3 */
	TIMER1B,        /* 12 - PD4 */
	TIMER1A,        /* 13 - PD5 */
	TIMER2B,        /* 14 - PD6 */
	TIMER2A,        /* 15 - PD7 */
	NOT_ON_TIMER,   /* 16 - PC0 */
	NOT_ON_TIMER,   /* 17 - PC1 */
	NOT_ON_TIMER,   /* 18 - PC2 */
	NOT_ON_TIMER,   /* 19 - PC3 */
	NOT_ON_TIMER,   /* 20 - PC4 */
	NOT_ON_TIMER,   /* 21 - PC5 */
	NOT_ON_TIMER,   /* 22 - PC6 */
	NOT_ON_TIMER,   /* 23 - PC7 */
	NOT_ON_TIMER,   /* 24 - PA0 */
	NOT_ON_TIMER,   /* 25 - PA1 */
	NOT_ON_TIMER,   /* 26 - PA2 */
	NOT_ON_TIMER,   /* 27 - PA3 */
	NOT_ON_TIMER,   /* 28 - PA4 */
	NOT_ON_TIMER,   /* 29 - PA5 */
	NOT_ON_TIMER,   /* 30 - PA6 */
	NOT_ON_TIMER    /* 31 - PA7 */
};
#endif
#endif
