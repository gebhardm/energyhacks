/* DHT library 
MIT license
written by Adafruit Industries
*/

#include "DHT.h"

DHT::DHT(uint8_t pin, uint8_t type, uint8_t count) {
	_pin = pin;
	_type = type;
	_count = count;
	firstreading = true;
}

void DHT::begin(void) {
	// set up the pins!
	pinMode(_pin, INPUT);
	digitalWrite(_pin, HIGH);
	_lastreadtime = 0;
}

//boolean S == Scale.  True == Fahrenheit; False == Celsius
float DHT::readTemperature(bool S) {
	float f;

	if (read()) {
		switch (_type) {
		case DHT11:
			f = data[2];
			if(S)
			f = convertCtoF(f);
			
			return f;
		case DHT22:
		case DHT21:
			f = data[2] & 0x7F;
			f *= 256;
			f += data[3];
			f /= 10;
			if (data[2] & 0x80)
			f *= -1;
			if(S)
			f = convertCtoF(f);

			return f;
		}
	}
	return NAN;
}

float DHT::convertCtoF(float c) {
	return c * 9 / 5 + 32;
}

float DHT::convertFtoC(float f) {
	return (f - 32) * 5 / 9; 
}

float DHT::readHumidity(void) {
	float f;
	if (read()) {
		switch (_type) {
		case DHT11:
			f = data[0];
			return f;
		case DHT22:
		case DHT21:
			f = data[0];
			f *= 256;
			f += data[1];
			f /= 10;
			return f;
		}
	}
	return NAN;
}

/*
float DHT::computeHeatIndex(float tempFahrenheit, float percentHumidity) {
	// Adapted from equation at: https://github.com/adafruit/DHT-sensor-library/issues/9 and
	// Wikipedia: http://en.wikipedia.org/wiki/Heat_index
	return -42.379 + 
	2.04901523 * tempFahrenheit + 
	10.14333127 * percentHumidity +
	-0.22475541 * tempFahrenheit*percentHumidity +
	-0.00683783 * pow(tempFahrenheit, 2) +
	-0.05481717 * pow(percentHumidity, 2) + 
	0.00122874 * pow(tempFahrenheit, 2) * percentHumidity + 
	0.00085282 * tempFahrenheit*pow(percentHumidity, 2) +
	-0.00000199 * pow(tempFahrenheit, 2) * pow(percentHumidity, 2);
}
*/
float DHT::computeHeatIndex(float tempC, float percH) {
	// see http://de.wikipedia.org/wiki/Hitzeindex
	return (float) 
	(-8.784695 +
	1.61139411 * tempC +
	2.338549 * percH +
	-0.14611605 * tempC * percH +
	-0.012308094 * pow(tempC, 2) +
	-0.016424828 * pow(percH, 2) +
	0.002211732 * pow(tempC, 2) * percH +
	0.00072546 * tempC * pow(percH, 2) +
	0.000003582 * pow(tempC, 2) * pow(percH,2));
}

boolean DHT::read(void) {
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;
	unsigned long currenttime;

	// Check if sensor was read less than two seconds ago and return early
	// to use last reading.
	currenttime = millis();
	if (currenttime < _lastreadtime) {
		// ie there was a rollover
		_lastreadtime = 0;
	}
	if (!firstreading && ((currenttime - _lastreadtime) < 2000)) {
		return true; // return last correct measurement
	}
	firstreading = false;
	_lastreadtime = millis();

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	// pull the pin high and wait 250 milliseconds
	digitalWrite(_pin, HIGH);
	delay(250);

	// now pull it low for ~20 milliseconds
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, LOW);
	delay(20);
	noInterrupts();
	digitalWrite(_pin, HIGH);
	delayMicroseconds(40);
	pinMode(_pin, INPUT);

	// read in timings
	for ( i=0; i< MAXTIMINGS; i++) {
		counter = 0;
		while (digitalRead(_pin) == laststate) {
			counter++;
			delayMicroseconds(1);
			if (counter == 255) {
				break;
			}
		}
		laststate = digitalRead(_pin);

		if (counter == 255) break;

		// ignore first 3 transitions
		if ((i >= 4) && (i%2 == 0)) {
			// shove each bit into the storage bytes
			data[j/8] <<= 1;
			if (counter > _count)
			data[j/8] |= 1;
			j++;
		}

	}

	interrupts();

	// check we read 40 bits and that the checksum matches
	if ((j >= 40) && 
			(data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
		return true;
	}

	return false;
}
