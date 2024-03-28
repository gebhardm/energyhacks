// IKEA OBEGRÄSAD LED matrix lamp using  SCT 2024 shift registers
// showing environment data: temperature, relative humidity and pressure

#include <SPI.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// get character set
#include "numchars.h"

// logarithmic LED fading
const uint8_t logled[256] PROGMEM = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                      0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                      0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
                                      0x1, 0x1, 0x1, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                                      0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x4, 0x4, 0x4, 0x4,
                                      0x4, 0x4, 0x4, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x5, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6,
                                      0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x8, 0x8, 0x8, 0x8, 0x9, 0x9, 0x9, 0x9, 0x9, 0xa,
                                      0xa, 0xa, 0xa, 0xb, 0xb, 0xb, 0xc, 0xc, 0xc, 0xc, 0xd, 0xd, 0xd, 0xe, 0xe, 0xe,
                                      0xf, 0xf, 0xf, 0x10, 0x10, 0x11, 0x11, 0x11, 0x12, 0x12, 0x13, 0x13, 0x13, 0x14, 0x14, 0x15,
                                      0x15, 0x16, 0x16, 0x17, 0x17, 0x18, 0x19, 0x19, 0x1a, 0x1a, 0x1b, 0x1c, 0x1c, 0x1d, 0x1e, 0x1e,
                                      0x1f, 0x20, 0x20, 0x21, 0x22, 0x23, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2a, 0x2b,
                                      0x2c, 0x2d, 0x2e, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x38, 0x39, 0x3a, 0x3b, 0x3d, 0x3e,
                                      0x40, 0x41, 0x42, 0x44, 0x45, 0x47, 0x49, 0x4a, 0x4c, 0x4e, 0x4f, 0x51, 0x53, 0x55, 0x57, 0x59,
                                      0x5b, 0x5d, 0x5f, 0x61, 0x63, 0x65, 0x67, 0x6a, 0x6c, 0x6f, 0x71, 0x74, 0x76, 0x79, 0x7b, 0x7e,
                                      0x81, 0x84, 0x87, 0x8a, 0x8d, 0x90, 0x93, 0x96, 0x9a, 0x9d, 0xa1, 0xa4, 0xa8, 0xac, 0xaf, 0xb3,
                                      0xb7, 0xbb, 0xbf, 0xc4, 0xc8, 0xcc, 0xd1, 0xd6, 0xda, 0xdf, 0xe4, 0xe9, 0xee, 0xf4, 0xf9, 0xff };

// LED matrix pins
#define CLA_PIN 8
#define EN_PIN 9
// grid dimensions corresponding to LED matrix
#define MAX_Y 16
#define MAX_X 16
// number of shift registers
#define SR 16
// switch PWM fading
#define PWM 1

// grid for display
uint8_t grid[MAX_Y * MAX_X];

// environment sensor
Adafruit_BME280 bme;

void setup() {
  // set the output pins
  pinMode(CLA_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  // switch LEDs off - OE/EN is internally pulled high
  digitalWrite(EN_PIN, HIGH);
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
  //delay(2000);
  fade_grid();
  // output relative humidity on Obegränsad
  memset(grid, 0, MAX_Y * MAX_X);
  ones = hum % 10;
  tens = (hum - ones) / 10;
  if (tens != 0) draw_char(0, 0, tens);
  draw_char(8, 0, ones);
  draw_char(0, 8, 12);  // %
  draw_char(8, 8, 13);  // H
  display_grid(0);
  //delay(2000);
  fade_grid();
  // display pressure
  memset(grid, 0, MAX_Y * MAX_X);
  // display condition icon
  // output rain
  if (pres < 990) {
    draw_char(0, 0, 14);
    draw_char(8, 0, 15);
    draw_char(0, 8, 16);
    draw_char(8, 8, 17);
  }
  // output cloud
  else if ((pres >= 990) && (pres < 1010)) {
    draw_char(0, 2, 14);
    draw_char(8, 2, 15);
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
  // output the iconized pressure value
  draw_num(0, 12, pres);
  // and display everything
  display_grid(0);
  //delay(2000);
  fade_grid();
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

// fade the grid on the LEDs using PWM on the enable pin
void fade_grid(void) {
#ifdef PWM
  for (int i = 0xff; i >= 0; i--) {
    analogWrite(EN_PIN, pgm_read_byte(&logled[i]));
    delay(15);
  }
  for (int i = 0; i < 0xff; i++) {
    analogWrite(EN_PIN, pgm_read_byte(&logled[i]));
    delay(15);
  }
#else
  digitalWrite(EN_PIN, LOW);
  delay(2000);
  digitalWrite(EN_PIN, HIGH);
#endif
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