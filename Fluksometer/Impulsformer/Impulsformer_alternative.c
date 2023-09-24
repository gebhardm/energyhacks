/**************************************************************************
Impulsformer - device to transform a voltage delivered from a current clamp
to an S0-output with 1000 pulses per kilo-Watt-hour

Alternative version with energy calculation in timer interupt and
continuous ADC measurement...

Uses an Atmel ATtiny13(A)) - Copyright 2012 Markus Gebhard

Changed oscillator setting 4,8MHz with prescaling by 8 to 600kHz

One clamp variant:
Use ADC3/PB3 as analog input,
Use PB2 as pulse output with optocoupler active low;
Three clamp variant:
Use ADC1,2,3/PB2,4,3 as analog inputs,
Use PB1 as pulse output with optocoupler active low;

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
**************************************************************************/

#ifndef F_CPU
#define F_CPU 1200000	// 1200kHz = 9,6MHz/8
#endif

#define THREE_CLAMPS 0	// if 1 then adapt to the three clamp variant

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define GRID_V	230		// Standard voltage of power grid in Germany
#define CLAMP_A	50		// Maximum current that the current clamp transforms
#define N_KWH   1000	// Number of pulses per kWh

#define PULSEPORT PORTB	// the port on which the pulses are given
#define PULSEDDR  DDRB	// direction of pulse port

#if THREE_CLAMPS
#define PULSE_1   PB1	// pulse output port in three clamp variant
#else
#define PULSE_1   PB2	// pulse output port in one clamp variant
#endif

// global variables for use in interupt routine
// factors for energy calculation
volatile uint32_t meterconst, pulseconst;
// the actual energy
volatile uint32_t energy;
// current ADC reading
volatile uint16_t actADC;
// ADC readings
#if THREE_CLAMPS
volatile uint16_t lastADC[3];	// values of last measurements for integration
volatile uint8_t activeADC;		// the actually active ADC port for multiplexing
#else
volatile uint16_t lastADC;	// value of last measurement for integration
#endif

// initialize the Pulse output port
void init_Output( void )
{
	// set output port as output
	PULSEDDR |= (1<<PULSE_1);
	// set level on output port high
	PULSEPORT |= (1<<PULSE_1);
}

// initialize the analog-digital-converter
void init_ADC( void )
{
	// Disable unused digital input buffers for noise reduction
	DIDR0  = (1<<ADC0D) | (1<<ADC1D) | (1<<ADC2D) | (1<<ADC3D);
	// prescaler clk/8 => 150kHz sampling frequency
	ADCSRA = (1<<ADPS1) | (1<<ADPS0);
	// Vref = Vbg (1.1V), right adjust (default), select ADC3 as input
	ADMUX  = (1<<REFS0) | (1<<MUX0) | (1<<MUX1);
	// enable ADC and start first ADC conversion (25 cycl)
	ADCSRA |= (1<<ADEN) | (1<<ADSC);
	_delay_ms(1);
	// start second conversion to initialize lastADC for proper "integration"
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC));
#if THREE_CLAMPS
	activeADC = 3;
	lastADC[activeADC-1] = ADC;
#else
	lastADC = ADC;
#endif
}

// initialize timer for continuous AD conversions and defined time base
void init_Timer( void )
{
	// set to prescaling / 1024 => 1 cycl = 0,85333333msec
	TCCR0B = (1<<CS02) | (1<<CS00);
	// set clear timer on compare match (CTC) mode
	TCCR0A = (1<<WGM01);
	// set top of CTC 47 * 0,853333msec = 40ms
	OCR0A = 47;
	// set timer interrupt
	TIMSK0 = (1<<OCIE0A);
}

// Interrupt on timer compare match to integrate power over defined time interval
ISR(TIM0_COMPA_vect)
{
#if THREE_CLAMPS
	energy += meterconst * (lastADC[activeADC-1] + actADC);
#else
	energy += meterconst * (lastADC + actADC);
#endif
	if (energy >= pulseconst)
	{
		// active on low
		PULSEPORT &= ~(1<<PULSE_1);
		// use delay compliant to S0-specification
		_delay_ms(30);
		// deactivate again
		PULSEPORT |= (1<<PULSE_1);;
		energy -= pulseconst;
	}
}

int main( void )
{
	init_ADC();
	init_Timer();
	init_Output();
	// calculate metering constant for measuring intervalls
	// (T/2) * (GRID_V * CLAMP_A / 1024); T = 40ms
	meterconst = 40;
#if THREE_CLAMPS
	// in the three clamp variant the ADC channels are measured each 3rd time
	meterconst *= 3;
#endif	
	meterconst *= GRID_V;
	meterconst *= CLAMP_A;
	meterconst /= 2048U;
	// set energy limit to send a pulse kWh -> 1000 W * 60 min * 60 sec * 1000 ms
	pulseconst = 3600000000U; 	// kWh in Wms
	pulseconst /= N_KWH;		// distributed to n impulses per kWh
	// enable interrupts
	sei();
	// main loop - continuous ADC measuring
	while (1)
	{
		while (ADCSRA & (1<<ADSC));
		actADC = ADC;
		// discrete integration via Trapezoid Sums I(f)a|b ~ ((b-a)/2)*(f(a)+f(b))
#if THREE_CLAMPS
		//energy += meterconst * (lastADC[activeADC-1] + actADC);
		lastADC[activeADC-1] = actADC;
		// count through ADCs
		// Start was ADC3,
		// ++ => 4 modulo 3 = 1 => ADC1, 
		// ++ => 2 modulo 3 = 2 => ADC2,
		// ++ => 3 keep for ADC3, and again from the beginning
		activeADC++;
		if (activeADC != 3) activeADC %= 3;
		// measure next ADC input
		ADMUX  = (1<<REFS0) | (activeADC<<MUX0);
#else
		//energy += meterconst * (lastADC + actADC);
		lastADC = actADC;
#endif
		// start next conversion
		ADCSRA |= (1<<ADSC);
	}
	return (0);
}
