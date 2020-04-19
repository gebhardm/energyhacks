// code adapted from https://github.com/tobyoxborrow/gameoflife-arduino
// now with torus plane
// in memory of John Horton Conway I had to also program this Game of Life
// Rest in Peace

// uses a simple 8x8 LED matrix with MAX7219 controller
// LedControl courtesy of https://github.com/wayoda/LedControl

#include "LedControl.h"

// LED matrix pins
#define DIN_PIN 12
#define CLK_PIN 11
#define CS_PIN 10
// grid dimensions corresponding to LED matrix
#define MAX_Y 8
#define MAX_X 8
// delay time between generations
#define GEN_DELAY 250
// how many generations per game before starting a new game
#define GENS_MAX 60
// how many generations to wait on no changes before starting a new game
#define NO_CHANGES_RESET 5

byte GENS = 0;       // counter for generations
byte NO_CHANGES = 0; // counter for generations without changes

// game state. 0 is dead cell, 1 is live cell
boolean grid[MAX_Y][MAX_X] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
};

boolean new_grid[MAX_Y][MAX_X] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
};

LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, 1);

void setup() {
  // seed random number generator from unused analog pin
  randomSeed(analogRead(A0));

  // initialise the LED matrix
  lc.shutdown(0, false);
  lc.setIntensity(0, 0);
  lc.clearDisplay(0);

  reset_grid();
  display_grid();
}

void loop() {
  play_gol();

  GENS++;

  display_grid();
  delay(GEN_DELAY);
  // reset the grid if the loop has been running a long time or no changes occur
  if ((GENS == GENS_MAX) || (NO_CHANGES == NO_CHANGES_RESET)) reset_grid();
}

// play game of life
void play_gol() {
  for (byte y = 0; y < MAX_Y; y++) {
    for (byte x = 0; x < MAX_X; x++) {
      byte neighbours = count_neighbours(y, x);
      // a new cell is dead by default
      new_grid[y][x] = 0;
      //
      //    1. Any live cell with fewer than two neighbours dies, as if by loneliness.
      //    2. Any live cell with more than three neighbours dies, as if by overcrowding.
      //    3. Any live cell with two or three neighbours lives, unchanged, to the next generation.
      if ((grid[y][x] == 1) && ((neighbours == 2) || (neighbours == 3)))
        new_grid[y][x] = 1;
      //
      //    4. Any dead cell with exactly three neighbours comes to life.
      if ((grid[y][x] == 0) && (neighbours == 3))
        new_grid[y][x] = 1;
    }
  }

  // update grid and count no changes occurring
  if (update_grid() == 0) {
    NO_CHANGES++;
  }
}

// count the number of neighbour live cells for a given cell
byte count_neighbours(byte y, byte x) {
  byte count = 0;
  byte xl = 0; // left of x
  byte xr = 0; // right of x
  byte yd = 0; // down from y
  byte yu = 0; // up from y
  // we deal with a torus: MAX_X <-> 0; MAX_Y <-> 0
  if ((x + 1) == MAX_X) xr = 0; else xr = x + 1;
  if (x == 0) xl = MAX_X - 1; else xl = x - 1;
  if ((y + 1) == MAX_Y) yu = 0; else yu = y + 1;
  if (y == 0) yd = MAX_Y - 1; else yd = y - 1;
  // the neighbourhood and its state
  count += grid[yd][xl];
  count += grid[y][xl];
  count += grid[yu][xl];

  count += grid[yd][x];
  count += grid[yu][x];

  count += grid[yd][xr];
  count += grid[y][xr];
  count += grid[yu][xr];

  return count;
}

// reset the grid
// we could set it all to zero then flip some bits on
// but that leads to some predictable games I see quite frequently
// instead, keep previous game state and flip some bits on
void reset_grid() {
  NO_CHANGES = 0;
  GENS = 0;
  for (int y = 0; y < MAX_Y; y++) {
    for (int x = 0; x < MAX_X; x++) {
      if (random(0, MAX_X) <= 1) grid[y][x] = 1;
    }
  }
}

byte update_grid() {
  // update the current grid from the new grid and count how many changes occured
  byte changes = 0;
  for (byte y = 0; y < MAX_Y; y++) {
    for (byte x = 0; x < MAX_X; x++) {
      if (new_grid[y][x] != grid[y][x]) changes++;
      grid[y][x] = new_grid[y][x];
    }
  }
  return changes;
}

// display the current grid to the LED matrix
void display_grid() {
  for (int y = 0; y < MAX_Y; y++) {
    for (int x = 0; x < MAX_X; x++) {
      lc.setLed(0, y, x, grid[y][x]);
    }
  }
}
