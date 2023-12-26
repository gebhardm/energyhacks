// code adapted from https://github.com/tobyoxborrow/gameoflife-arduino
// now with torus surface leading "over the edges"
// in memory of John Horton Conway I had to also program this Game of Life
// Rest in Peace

// adapted to an IKEA FREKVENS LED matrix lamp using SCT 2024 shift registers

// LED matrix pins
#define EN_PIN 12
#define DA_PIN 11
#define CLK_PIN 10
#define LAK_PIN 9
// grid dimensions corresponding to LED matrix
#define MAX_Y 16
#define MAX_X 16
// delay time between generations
#define GEN_DELAY 250
// how many generations per game before starting a new game
#define GENS_MAX 60
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
  pinMode(LAK_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DA_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  // route the grid directly to the LEDs - OE/EN is internally pulled high
  digitalWrite(EN_PIN, LOW);
  // no data, nothing to latch yet - CLA is internally pulled low
  digitalWrite(LAK_PIN, LOW);
  // set the clock and data pins low
  digitalWrite(CLK_PIN, LOW);
  digitalWrite(DA_PIN, LOW);
  // set initial, randomly set grid
  reset_grid();
  display_grid();
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
  if ((gens == GENS_MAX) || (no_changes == NO_CHANGES_RESET)) reset_grid();
}

// play game of life
void play_gol() {
  for (uint8_t y = 0; y < MAX_Y; y++) {
    for (uint8_t x = 0; x < MAX_X; x++) {
      uint8_t neighbours = count_neighbours(y, x);
      // a new cell is dead by default
      new_grid[y * MAX_Y + x] = 0;
      //    1. Any live cell with fewer than two neighbours dies, as if by loneliness.
      //    2. Any live cell with more than three neighbours dies, as if by overcrowding.
      //    3. Any live cell with two or three neighbours lives, unchanged, to the next generation.
      if ((grid[y * MAX_Y + x] == 1) && ((neighbours == 2) || (neighbours == 3)))
        new_grid[y * MAX_Y + x] = 1;
      //
      //    4. Any dead cell with exactly three neighbours comes to life.
      if ((grid[y * MAX_Y + x] == 0) && (neighbours == 3))
        new_grid[y * MAX_Y + x] = 1;
    }
  }
  // update grid and count no changes occurring
  if (update_grid() == 0) {
    no_changes++;
  }
}

// count the number of neighbour live cells for a given cell
uint8_t count_neighbours(uint8_t y, uint8_t x) {
  uint8_t count = 0;
  // torus surface: MAX_X <-> 0; MAX_Y <-> 0
  // the neighbourhood and its state
  for (short dy = -1; dy <= 1; dy++)
    for (short dx = -1; dx <= 1; dx++)
      if ((dx != 0) || (dy != 0))
        count += grid[mod(y + dy, MAX_Y) * MAX_Y + mod(x + dx, MAX_X)];
  return count;
}

// reset the grid - use ramdomizer to get different generation zeros
void reset_grid() {
  no_changes = 0;
  gens = 0;
  for (uint8_t y = 0; y < MAX_Y; y++) {
    for (uint8_t x = 0; x < MAX_X; x++) {
      if (random(0, MAX_X) < 2) grid[y * MAX_Y + x] = 1;
    }
  }
}

uint8_t update_grid() {
  // update the current grid from the new grid and count how many changes occured
  uint8_t changes = 0;
  for (uint8_t y = 0; y < MAX_Y; y++) {
    for (uint8_t x = 0; x < MAX_X; x++) {
      if (new_grid[y * MAX_Y + x] != grid[y * MAX_Y + x]) changes++;
      grid[y * MAX_Y + x] = new_grid[y * MAX_Y + x];
    }
  }
  return changes;
}

// display the current grid to the LED matrix
void display_grid() {
  uint8_t bx, by, idx;
  for (uint8_t y = 0; y < MAX_Y; y++) {
    for (uint8_t x = 0; x < MAX_X; x++) {
      // pass the cells to the LED matrix
      // note: the LEDs are on or off, as such <>0 or 0, a digitalWrite suffices
      // there may be a smarter implementation to allow grayscale using the full
      // byte of a cell in the grid
      // map to the weird LED assignment
      bx = (x & 7);
      by = y * 2;
      if (x & 8) by += 1;
      if (y & 8) {
        by -= 16;
        bx += 8;
      }
      idx = by * MAX_X + bx;
      // and send to the shift register
      digitalWrite(DA_PIN, grid[idx]);
      // toggle clock for next bit
      digitalWrite(CLK_PIN, HIGH);
      digitalWrite(CLK_PIN, LOW);
    }
  }
  // transfer the cells to the LED output
  digitalWrite(LAK_PIN, HIGH);
  digitalWrite(LAK_PIN, LOW);
}

// modulo function allowing also -1
int mod(int a, int b) {
  return a < 0 ? (a + b) % b : a % b;
}