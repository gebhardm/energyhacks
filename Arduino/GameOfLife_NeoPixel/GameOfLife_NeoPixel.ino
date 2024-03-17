// code adapted from https://github.com/tobyoxborrow/gameoflife-arduino
// now with torus surface leading "over the edges"
// in memory of John Horton Conway I had to also program this Game of Life
// Rest in Peace

// uses a WS2812B board with 64 pixels
// Adafruit_NeoPixel library https://github.com/adafruit/Adafruit_NeoPixel

// This version shows new, living and dying cells by color, inspired by 
// Brian White, Elektor 3/2024, S.56 ff.

#include <Adafruit_NeoPixel.h>

#define DEBUG 0

// NeoPixel pin and number of pixels to address
#define NEO_PIN 12
#define NUM_PXL 64
// grid dimensions corresponding to pixel matrix
#define MAX_Y 8
#define MAX_X 8
// delay time between generations
#define GEN_DELAY 500
// number of generations per game before starting a new game
#define GENS_MAX 60
// number of generations to wait on no changes before starting a new game
#define NO_CHANGES_RESET 10

uint8_t gens = 0;       // counter for generations
uint8_t noChanges = 0;  // counter for generations without changes

// game state. 0 is dead cell, 1 is live cell
uint8_t grid[MAX_Y][MAX_X];

Adafruit_NeoPixel pixels(NUM_PXL, NEO_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // seed random number generator from unused analog pin
  randomSeed(analogRead(A0));

  // initialise the NeoPixel matrix
  pixels.begin();
  pixels.clear();

  reset_grid();

  if (DEBUG) Serial.begin(19200);
}

void loop() {
  play_gol();

  gens++;

  display_grid();
  delay(GEN_DELAY);
  // reset the grid if the loop has been running a long time or no changes occur
  if ((gens == GENS_MAX) || (noChanges == NO_CHANGES_RESET)) reset_grid();
}

// play game of life
void play_gol() {
  for (uint8_t y = 0; y < MAX_Y; y++) {
    for (uint8_t x = 0; x < MAX_X; x++) {
      uint8_t neighbours = count_neighbours(y, x);
      // a new cell is dead by default
      grid[y][x] &= 0xfe;
      //
      //    1. Any live cell with fewer than two neighbours dies, as if by loneliness.
      //    2. Any live cell with more than three neighbours dies, as if by overcrowding.
      //    3. Any live cell with two or three neighbours lives, unchanged, to the next generation.
      if ((((grid[y][x] & 2) >> 1) == 1) && ((neighbours == 2) || (neighbours == 3)))
        grid[y][x] |= 1;
      //
      //    4. Any dead cell with exactly three neighbours comes to life.
      if ((((grid[y][x] & 2) >> 1) == 0) && (neighbours == 3))
        grid[y][x] |= 1;
    }
  }

  // update grid and count no changes occurring
  if (update_grid() == 0) {
    noChanges++;
  }
}

// count the number of neighbour live cells for a given cell
uint8_t count_neighbours(int y, int x) {
  uint8_t count = 0;
  // torus surface: MAX_X <-> 0; MAX_Y <-> 0
  uint8_t xl = mod(x - 1, MAX_X);  // left of x
  uint8_t xr = mod(x + 1, MAX_X);  // right of x
  uint8_t yd = mod(y - 1, MAX_Y);  // down from y
  uint8_t yu = mod(y + 1, MAX_Y);  // up from y
  // the neighbourhood and its state
  count += ((grid[yd][xl] & 2) >> 1);
  count += ((grid[y][xl] & 2) >> 1);
  count += ((grid[yu][xl] & 2) >> 1);
  count += ((grid[yd][x] & 2) >> 1);
  count += ((grid[yu][x] & 2) >> 1);
  count += ((grid[yd][xr] & 2) >> 1);
  count += ((grid[y][xr] & 2) >> 1);
  count += ((grid[yu][xr] & 2) >> 1);
  return count;
}

// reset the grid - use ramdomizer to get different generation zeros
void reset_grid() {
  noChanges = 0;
  gens = 0;
  for (int y = 0; y < MAX_Y; y++) {
    for (int x = 0; x < MAX_X; x++) {
      grid[y][x] = 0;
      if (random(0, MAX_X) < 2) grid[y][x] |= 2;
    }
  }
  display_grid();
  delay(GEN_DELAY);
}

uint8_t update_grid() {
  // update the current grid from the new grid and count how many changes occured
  uint8_t changes = 0;
  for (uint8_t y = 0; y < MAX_Y; y++) {
    for (uint8_t x = 0; x < MAX_X; x++) {
      if ((grid[y][x] & 1) != ((grid[y][x] & 2) >> 1)) changes++;
      grid[y][x] <<= 1;
    }
  }
  return changes;
}

// display the current grid
void display_grid() {
  uint8_t cell;
  for (int px = 0; px < NUM_PXL; px++) {
    cell = grid[mod(px, MAX_Y)][(uint8_t)(px / MAX_X)];
    // newly born cell
    if ((((cell & 2) >> 1) == 1) && (((cell & 4) >> 2) == 0))
      pixels.setPixelColor(px, pixels.Color(0, 8, 0));
    // living cell
    else if ((((cell & 2) >> 1) == 1) && (((cell & 4) >> 2) == 1))
      pixels.setPixelColor(px, pixels.Color(0, 0, 8));
    // died cell
    else if ((((cell & 2) >> 1) == 0) && (((cell & 4) >> 2) == 1))
      pixels.setPixelColor(px, pixels.Color(2, 0, 2));
    else pixels.setPixelColor(px, pixels.Color(0, 0, 0));
  }
  pixels.show();
  if (DEBUG) {
    for (uint8_t y = 0; y < MAX_Y; y++) {
      for (uint8_t x = 0; x < MAX_X; x++) {
        Serial.print((grid[y][x] & 2) >> 1, BIN);
      }
      Serial.println();
    }
    Serial.println();
  }
}

// modulo function allowing also -1 to map to b-1
int mod(int a, int b) {
  return a < 0 ? (a + b) % b : a % b;
}