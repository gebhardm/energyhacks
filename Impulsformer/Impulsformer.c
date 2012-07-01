/**************************************************************************
Impulsformer - device to transform a voltage delivered from a current clamp
to an S0-output with 1000 pulses per kilo-Watt-hour

Uses an Atmel ATtiny13(A)) - Copyright 2012 Markus Gebhard

Changed oscillator setting 4,8MHz with prescaling by 8 to 600kHz

One clamp variant:
Use ADC3/PB3 as analog input,
Use PB2 as pulse output with optocoupler active low;
Three clamp variant:
Use ADC1,2,3/PB2,4,3 as analog inputs,
Use PB1 as pulse output with optocoupler active low;
Note: current clamp has reaction time of 300ms according to vendor info

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
#define F_CPU 600000	// 600kHz = 4,8MHz/8
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

// global variables
uint32_t meterconst, pulseconst, energy;
#if THREE_CLAMPS
uint16_t lastADC[3];	// values of last measurements for integration
uint8_t activeADC;		// the actually active ADC port for multiplexing
#else
uint16_t lastADC;		// value of last measurement for integration
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
	// prescaler clk/4 => 150kHz sampling frequency
	ADCSRA = (1<<ADPS1);
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
	// set to prescaling / 1024=> 1 cycl = 1,706667msec
	TCCR0B = (1<<CS02) | (1<<CS00);
	// set clear time on compare match (CTC) mode
	TCCR0A = (1<<WGM01);
	// set top of CTC 195 * 1,706667msec = 333ms
	OCR0A = 195;
	// set timer interrupt
	TIMSK0 = (1<<OCIE0A);
}

void send_pulse( void )
{
	// active on low
	PULSEPORT &= ~(1<<PULSE_1);
	// use delay compliant to S0-specification
	_delay_ms(30);
	// deactivate again
	PULSEPORT |= (1<<PULSE_1);
}

// Interrupt on timer compare match to measure and integrate power
ISR(TIM0_COMPA_vect)
{
	uint16_t actADC;				// actually measured value
	while (ADCSRA & (1<<ADSC));
	actADC = ADC;
	// discrete integration via Trapezoid Sums I(f)a|b ~ ((b-a)/2)*(f(a)+f(b))
#if THREE_CLAMPS
	energy += meterconst * (lastADC[activeADC-1] + actADC);
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
	energy += meterconst * (lastADC + actADC);
	lastADC = actADC;
#endif
	if (energy >= pulseconst)
	{
		send_pulse();
		energy -= pulseconst;
	}
	// start next conversion
	ADCSRA |= (1<<ADSC);
}

int main( void )
{
	init_ADC();
	init_Timer();
	init_Output();
	// calculate metering constant for measuring intervalls
	// (T/2) * (GRID_V * CLAMP_A / 1024); T = 333ms
	meterconst = 333U;
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
	// main loop
	while (1)
	{
	}
	return (0);
}
