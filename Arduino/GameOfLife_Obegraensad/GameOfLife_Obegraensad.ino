// code adapted from https://github.com/tobyoxborrow/gameoflife-arduino
// now with torus surface leading "over the edges"
// in memory of John Horton Conway I had to also program this Game of Life
// Rest in Peace

// adapted to an IKEA OBEGRÄSAD LED matrix lamp using  SCT 2024 shift registers

#include <SPI.h>

// LED matrix pins - CLK to SPI CLK(13) and DO to SPI COPI(11)
#define CLA_PIN 9
#define EN_PIN 8
// number of shift registers
#define SR 16
// grid dimensions corresponding to LED matrix
#define MAX_Y 16
#define MAX_X 16
// delay time between generations
#define GEN_DELAY 250
// how many generations per game before starting a new game
#define GENS_MAX 100
// how many generations to wait on no changes before starting a new game
#define NO_CHANGES_RESET 5

uint8_t gens = 0;        // counter for generations
uint8_t no_changes = 0;  // counter for generations without changes

// game state. 0 is dead cell, 1 is live cell
bool grid[MAX_Y * MAX_X];
bool new_grid[MAX_Y * MAX_X];

void setup() {
  // seed random number generator from unused analog pin
  randomSeed(analogRead(A0));
  // set the output pins
  pinMode(CLA_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  // route the grid directly to the LEDs - OE/EN is internally pulled high
  digitalWrite(EN_PIN, LOW);
  // no data, nothing to latch yet - CLA is internally pulled low
  digitalWrite(CLA_PIN, LOW);
  // set data transfer
  SPI.begin();
  // set initial, randomly set grid
  reset_grid();
}

void loop() {
  // play the game of life for this generation
  play_gol();
  // display this generation
  display_grid();
  // go to the next generation
  gens++;
  delay(GEN_DELAY);
  // reset the grid if the loop has been running a long time or no changes occur
  if ((gens == GENS_MAX) || (no_changes == NO_CHANGES_RESET))
    reset_grid();
}

// play game of life
void play_gol() {
  for (uint8_t y = 0; y < MAX_Y; y++) {
    for (uint8_t x = 0; x < MAX_X; x++) {
      uint8_t neighbours = count_neighbours(x, y);
      // a new cell is dead by default
      new_grid[y * MAX_X + x] = 0;
      // 1. Any live cell with fewer than two neighbours dies, as if by loneliness.
      // 2. Any live cell with more than three neighbours dies, as if by overcrowding.
      // 3. Any live cell with two or three neighbours lives, unchanged, to the next generation.
      if ((grid[y * MAX_X + x] == 1) && ((neighbours == 2) || (neighbours == 3)))
        new_grid[y * MAX_X + x] = 1;
      // 4. Any dead cell with exactly three neighbours comes to life.
      if ((grid[y * MAX_X + x] == 0) && (neighbours == 3))
        new_grid[y * MAX_X + x] = 1;
    }
  }
  // update grid and count no changes occurring
  if (update_grid() == 0)
    no_changes++;
}

// count the number of neighbour live cells for a given cell
uint8_t count_neighbours(uint8_t x, uint8_t y) {
  uint8_t count = 0;
  // torus surface: MAX_X <-> 0; MAX_Y <-> 0
  // the neighbourhood and its state
  for (short dy = -1; dy <= 1; dy++)
    for (short dx = -1; dx <= 1; dx++)
      if ((dx != 0) || (dy != 0))
        count += grid[mod(y + dy, MAX_Y) * MAX_X + mod(x + dx, MAX_X)];
  return count;
}

// reset the grid - use ramdomizer to get different generation zeros
void reset_grid() {
  no_changes = 0;
  gens = 0;
  for (int i = 0; i < MAX_Y * MAX_X; i++)
    if (random(0, MAX_X) < 2) grid[i] = 1;
    else grid[i] = 0;
}

int update_grid() {
  // update the current grid from the new grid and 
  // count how many changes occured
  int changes = 0;
  for (int i = 0; i < MAX_Y * MAX_X; i++) {
    if (new_grid[i] != grid[i]) changes++;
    grid[i] = new_grid[i];
  }
  return changes;
}

// display the current grid to the LED matrix
void display_grid() {
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
      buffer[y * 2 + ((x & 8) >> 3)] |= ((grid[by * MAX_X + bx] & 0x01) << (x % 8));
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

// modulo function allowing also -1
int mod(int a, int b) {
  return a < 0 ? (a + b) % b : a % b;
}