// IKEA OBEGRÄSAD LED matrix lamp using  SCT 2024 shift registers
// showing environment data: temperature, relative humidity and pressure
// Runs on an Ardunio Mini with ATMega168

#include <SPI.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// get character set
#include "numchars.h"

// debugging
#define DEBUG 0
// LED matrix pins
#define CLA_PIN 8
#define EN_PIN 9
// define button pins
#define BUT_I 2
#define BUT_O 3
// grid dimensions corresponding to LED matrix
#define MAX_Y 16
#define MAX_X 16
// number of shift registers
#define SR 16
// delay time between generations
#define GEN_DELAY 500
// number of generations per game before starting a new game
#define GENS_MAX 60
// number of generations to wait on no changes before starting a new game
#define NO_CHANGES_RESET 10

// for Game of Life
uint8_t gens = 0;       // counter for generations
uint8_t noChanges = 0;  // counter for generations without changes

// grid for display
uint8_t grid[MAX_Y * MAX_X];
volatile uint8_t button_state = LOW;
// environment sensor
Adafruit_BME280 bme;

void setup() {
  // seed random number generator from unused analog pin
  randomSeed(analogRead(A0));
  // set the output pins
  pinMode(CLA_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  // button handling
  pinMode(BUT_O, OUTPUT);
  pinMode(BUT_I, INPUT_PULLUP);
  digitalWrite(BUT_O, LOW);
  attachInterrupt(digitalPinToInterrupt(BUT_I), handle_button, FALLING);
  // switch LEDs off - OE/EN is internally pulled high
  digitalWrite(EN_PIN, HIGH);
  // no data, nothing to latch yet - CLA is internally pulled low
  digitalWrite(CLA_PIN, LOW);
  // initialize SPI
  SPI.begin();
  // set up BMP280 sensor
  bme.begin(0x76);
  // debugging
  if (DEBUG) Serial.begin(9600);
}

void loop() {
  if (button_state) {
    digitalWrite(EN_PIN, LOW);
    play_gol();
    gens++;
    display_grid(1);
    delay(GEN_DELAY);
    // reset the grid if the loop has been running a long time or no changes occur
    if ((gens == GENS_MAX) || (noChanges == NO_CHANGES_RESET)) reset_grid();
  }
  if (!button_state) show_temperature();
  if (!button_state) show_humidity();
  if (!button_state) show_pressure();
}

// button interrupt handler
void handle_button(void) {
  button_state = !button_state;
  if (DEBUG) Serial.println(button_state);
}

// play game of life
void play_gol() {
  for (uint8_t y = 0; y < MAX_Y; y++) {
    for (uint8_t x = 0; x < MAX_X; x++) {
      uint8_t neighbours = count_neighbours(x, y);
      // a new cell is dead by default
      grid[y * MAX_X + x] &= 0xfe;
      //    1. Any live cell with fewer than two neighbours dies, as if by loneliness.
      //    2. Any live cell with more than three neighbours dies, as if by overcrowding.
      //    3. Any live cell with two or three neighbours lives, unchanged, to the next generation.
      if ((((grid[y * MAX_X + x] & 2) >> 1) == 1) && ((neighbours == 2) || (neighbours == 3)))
        grid[y * MAX_X + x] |= 1;
      //    4. Any dead cell with exactly three neighbours comes to life.
      if ((((grid[y * MAX_X + x] & 2) >> 1) == 0) && (neighbours == 3))
        grid[y * MAX_X + x] |= 1;
    }
  }
  // update grid and count no changes occurring
  if (update_grid() == 0) {
    noChanges++;
  }
}

// count the number of neighbour live cells for a given cell
uint8_t count_neighbours(int x, int y) {
  uint8_t count = 0;
  // torus surface: MAX_X <-> 0; MAX_Y <-> 0
  // the neighbourhood and its state
  for (short dy = -1; dy <= 1; dy++)
    for (short dx = -1; dx <= 1; dx++)
      if ((dx != 0) || (dy != 0))
        count += (grid[mod(y + dy, MAX_Y) * MAX_X + mod(x + dx, MAX_X)] & 2) >> 1;
  return count;
}

// reset the grid - use ramdomizer to get different generation zeros
void reset_grid() {
  noChanges = 0;
  gens = 0;
  for (int i = 0; i < MAX_Y * MAX_X; i++) {
    if (random(0, MAX_X) < 2) grid[i] = 2;
    else grid[i] = 0;
  }
  display_grid(1);
  delay(GEN_DELAY);
}

uint8_t update_grid() {
  // update the current grid from the new grid and count how many changes occured
  uint8_t changes = 0;
  for (int i = 0; i < MAX_Y * MAX_X; i++) {
    if ((grid[i] & 1) != ((grid[i] & 2) >> 1)) changes++;
    grid[i] <<= 1;
  }
  return changes;
}

// modulo function allowing also -1 to map to b-1
int mod(int a, int b) {
  return a < 0 ? (a + b) % b : a % b;
}

// show temperature
void show_temperature(void) {
  int temp = (int)bme.readTemperature();
  memset(grid, 0, MAX_Y * MAX_X);
  uint8_t ones = temp % 10;
  uint8_t tens = (temp - ones) / 10;
  if (tens != 0) draw_char(0, 0, tens);
  draw_char(8, 0, ones);
  draw_char(0, 8, 10);  // °
  draw_char(8, 8, 11);  // C
  display_grid(0);
  fade_grid(1);
}

// output relative humidity
void show_humidity(void) {
  int hum = (int)bme.readHumidity();
  memset(grid, 0, MAX_Y * MAX_X);
  uint8_t ones = hum % 10;
  uint8_t tens = (hum - ones) / 10;
  if (tens != 0) draw_char(0, 0, tens);
  draw_char(8, 0, ones);
  draw_char(0, 8, 12);  // %
  draw_char(8, 8, 13);  // H
  display_grid(0);
  fade_grid(1);
}

// display pressure
void show_pressure(void) {
  int pres = (int)(bme.readPressure() / 100.0F);
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
  fade_grid(1);
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
      switch (s) {
        case 0:
          if (x & 8) {
            bx = 0x0f - (x & 7);
            by += 1;
          } else {
            bx = x + 0x08;
          }
          break;
        case 1:
          if (x & 8) {
            bx = 0x0f - x;
          } else {
            by -= 1;
          }
          break;
        case 2:
          if (x & 8) {
            bx = 0x0f - x;
            by += 1;
          }
          break;
        case 3:
          if (x & 8) {
            bx = 0x0f - (x & 7);
          } else {
            bx += 0x08;
            by -= 1;
          }
          break;
      }
      // map to the shift registers' bits
      buffer[y * 2 + ((x & 8) >> 3)] |= (((grid[by * MAX_X + bx] & (1 << bit)) >> bit) << (x % 8));
    }
  }
  // transfer to shift registers
  SPI.beginTransaction(SPISettings(800000, MSBFIRST, SPI_MODE0));
  SPI.transfer(buffer, SR * 2);
  SPI.endTransaction();
  // transfer the cells to the LED output
  digitalWrite(CLA_PIN, HIGH);
  digitalWrite(CLA_PIN, LOW);
}

// fade the grid on the LEDs using PWM on the enable pin
void fade_grid(uint8_t mode) {
  switch (mode) {
    case 1:
      for (int i = 0xff; i >= 0; i--) {
        analogWrite(EN_PIN, pgm_read_byte_near(logled + i));
        delay(15);
      }
      for (int i = 0; i <= 0xff; i++) {
        analogWrite(EN_PIN, pgm_read_byte_near(logled + i));
        delay(15);
      }
      break;
    case 2:
      for (int i = 0; i <= 0xff; i++) {
        analogWrite(EN_PIN, 0xff - pgm_read_byte_near(logled + i));
        delay(5);
      }
      delay(1000);
      for (int i = 0xff; i >= 0; i--) {
        analogWrite(EN_PIN, 0xff - pgm_read_byte_near(logled + i));
        delay(5);
      }
      break;
    default:
      digitalWrite(EN_PIN, LOW);
      delay(2000);
      digitalWrite(EN_PIN, HIGH);
      break;
  }
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