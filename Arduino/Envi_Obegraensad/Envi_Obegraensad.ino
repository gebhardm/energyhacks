// IKEA OBEGRÄSAD LED matrix lamp using  SCT 2024 shift registers
// showing environment data: temperature, relative humidity and pressure

#include <SPI.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// get character set
#include "numchars.h"

// LED matrix pins
#define CLA_PIN 9
#define EN_PIN 8
// grid dimensions corresponding to LED matrix
#define MAX_Y 16
#define MAX_X 16
// number of shift registers
#define SR 16

// grid for display
uint8_t grid[MAX_Y * MAX_X];

// environment sensor
Adafruit_BME280 bme;

void setup() {
  // set the output pins
  pinMode(CLA_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  // route the grid directly to the LEDs - OE/EN is internally pulled high
  digitalWrite(EN_PIN, LOW);
  // no data, nothing to latch yet - CLA is internally pulled low
  digitalWrite(CLA_PIN, LOW);
  // initialize SPI
  SPI.begin();
  // set up BMP280 sensor
  bme.begin(0x76);
}

void loop() {
  uint8_t tens, ones;
  // get environment data
  int temp = (int)bme.readTemperature();
  int pres = (int)(bme.readPressure() / 100.0F);
  int hum = (int)bme.readHumidity();
  // output temperature on Obegränsad
  memset(grid, 0, MAX_Y * MAX_X);
  ones = temp % 10;
  tens = (temp - ones) / 10;
  if (tens != 0) draw_char(0, 0, tens);
  draw_char(8, 0, ones);
  draw_char(0, 8, 10);  // °
  draw_char(8, 8, 11);  // C
  display_grid(0);
  delay(2000);
  // output realtive humidity on Obegränsad
  memset(grid, 0, MAX_Y * MAX_X);
  ones = hum % 10;
  tens = (hum - ones) / 10;
  if (tens != 0) draw_char(0, 0, tens);
  draw_char(8, 0, ones);
  draw_char(0, 8, 12);  // %
  draw_char(8, 8, 13);  // H
  display_grid(0);
  delay(2000);
  // display pressure
  memset(grid, 0, MAX_Y * MAX_X);
  // display condition icon
  // draw_num(0, 12, pres);
  // output rain
  if (pres < 990) {
    draw_char(0, 0, 14);
    draw_char(8, 0, 15);
    draw_char(0, 8, 16);
    draw_char(8, 8, 17);
  }
  // output cloud
  else if ((pres >= 990) && (pres < 1010)) {
    draw_char(0, 0, 14);
    draw_char(8, 0, 15);
  }
  // output cloud with sun
  else if ((pres >= 1010) && (pres < 1030)) {
    draw_char(0, 0, 22);
    draw_char(8, 0, 23);
    draw_char(0, 8, 24);
    draw_char(8, 8, 25);
  }
  // output sun
  else if (pres >= 1030) {
    draw_char(0, 0, 18);
    draw_char(8, 0, 19);
    draw_char(0, 8, 20);
    draw_char(8, 8, 21);
  }
  display_grid(0);
  delay(2000);
}

// display the current grid to the LED matrix
void display_grid(uint8_t bit) {
  uint8_t bx, by, s;
  uint8_t buffer[SR * 2];
  memset(buffer, 0, SR * 2);
  for (uint8_t y = 0; y < MAX_Y; y++) {
    for (uint8_t x = 0; x < MAX_X; x++) {
      // map to the weird LED assignment
      s = y & 3;
      bx = x;
      by = y;
      if (s == 0) {
        if (x & 8) {
          bx = 0x0f - (x & 7);
          by += 1;
        } else {
          bx = x + 0x08;
        }
      } else if (s == 1) {
        if (x & 8) {
          bx = 0x0f - x;
        } else {
          by -= 1;
        }
      } else if (s == 2) {
        if (x & 8) {
          bx = 0x0f - x;
          by += 1;
        }
      } else if (s == 3) {
        if (x & 8) {
          bx = 0x0f - (x & 7);
        } else {
          bx += 0x08;
          by -= 1;
        }
      }
      // map to the shift registers' bits
      buffer[y * 2 + ((x & 8) >> 3)] |= ((grid[by * MAX_X + bx] & (1 << bit)) << (x % 8));
    }
  }
  // transfer to shift registers
  SPI.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE0));
  SPI.transfer(buffer, SR * 2);
  SPI.endTransaction();
  // transfer the cells to the LED output
  digitalWrite(CLA_PIN, HIGH);
  digitalWrite(CLA_PIN, LOW);
}

void draw_char(uint8_t x0, uint8_t y0, uint8_t ch) {
  uint8_t c[8] = {};
  int i = 0;
  for (uint8_t j = 0; j < 8; j++) c[j] = pgm_read_byte_near(chars + ch * 8 + j);
  for (uint8_t y = y0; y < y0 + 8; y++) {
    for (uint8_t x = x0; x < x0 + 8; x++) {
      i = x0 + 0x07 - x;
      grid[y * MAX_X + x] = ((c[y - y0] & (1 << i)) >> i);
    }
  }
}

void draw_num(uint8_t x0, uint8_t y0, int num) {
  uint8_t n[4] = {};
  int div = 1000;
  int number = num;
  int i = 0;
  // show digits
  for (uint8_t d = 0; d < 4; d++) {
    for (uint8_t j = 0; j < 4; j++) n[j] = pgm_read_byte_near(nums + (number / div) * 4 + j);
    for (uint8_t y = y0; y < y0 + 4; y++) {
      for (uint8_t x = x0; x < x0 + 4; x++) {
        i = x0 + 0x03 - x;
        grid[y * MAX_X + x + d * 4] = ((n[y - y0] & (1 << i)) >> i);
      }
    }
    number -= (number / div) * div;
    div /= 10;
  }
}