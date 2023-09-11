// code adapted from https://github.com/tobyoxborrow/gameoflife-arduino
// now with torus surface leading "over the edges"
// in memory of John Horton Conway I had to also program this Game of Life
// Rest in Peace

// adapted to an IKEA OBEGRÄSAD LED matrix lamp using  SCT 2024 shift registers

// LED matrix pins
#define CLA_PIN 12
#define CLK_PIN 11
#define DO_PIN 10
#define EN_PIN 9
// grid dimensions corresponding to LED matrix
#define MAX_Y 16
#define MAX_X 16
// delay time between generations
#define GEN_DELAY 250
// how many generations per game before starting a new game
#define GENS_MAX 60
// how many generations to wait on no changes before starting a new game
#define NO_CHANGES_RESET 5

uint8_t gens = 0;       // counter for generations
uint8_t no_changes = 0; // counter for generations without changes

// game state. 0 is dead cell, 1 is live cell
bool grid[MAX_Y * MAX_X];
bool new_grid[MAX_Y * MAX_X];

// map the weird geometry of the shift registers (taken from https://github.com/ph1p/ikea-led-obegraensad)
const uint8_t ledmap[MAX_Y * MAX_X] = {
      0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
      0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
      0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
      0x4f, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a, 0x49, 0x48, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
      0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x40, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
      0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61, 0x60, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
      0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x69, 0x68, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
      0x8f, 0x8e, 0x8d, 0x8c, 0x8b, 0x8a, 0x89, 0x88, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
      0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x80, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
      0xa7, 0xa6, 0xa5, 0xa4, 0xa3, 0xa2, 0xa1, 0xa0, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
      0xaf, 0xae, 0xad, 0xac, 0xab, 0xaa, 0xa9, 0xa8, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
      0xcf, 0xce, 0xcd, 0xcc, 0xcb, 0xca, 0xc9, 0xc8, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
      0xc7, 0xc6, 0xc5, 0xc4, 0xc3, 0xc2, 0xc1, 0xc0, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
      0xe7, 0xe6, 0xe5, 0xe4, 0xe3, 0xe2, 0xe1, 0xe0, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
      0xef, 0xee, 0xed, 0xec, 0xeb, 0xea, 0xe9, 0xe8, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

void setup() {
  // seed random number generator from unused analog pin
  randomSeed(analogRead(A0));
  // set the output pins
  pinMode(CLA_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DO_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  // set the LED output
  //analogWrite(EN_PIN,200); // PWM to switch LED brightness
  // route the grid directly to the LEDs
  digitalWrite(EN_PIN, LOW);
  // no data, nothing to latch
  digitalWrite(CLA_PIN, HIGH);
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
      //
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
uint8_t count_neighbours(int y, int x) {
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
  for (int y = 0; y < MAX_Y; y++) {
    for (int x = 0; x < MAX_X; x++) {
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
  for (int y = 0; y < MAX_Y; y++) {
    for (int x = 0; x < MAX_X; x++) {
      // pass the cells to the LED matrix
      // note: the LEDs are on or off, as such <>0 or 0, a digitalWrite suffices
      // there may be a smarter implementation to allow grayscale using the full
      // byte of a cell in the grid
      digitalWrite(DO_PIN, grid[ledmap[y * MAX_Y + x]]);
      // toggle clock for next bit
      digitalWrite(CLK_PIN, HIGH);
      digitalWrite(CLK_PIN, LOW);
    }
  }
  // latch the LED grid
  digitalWrite(CLA_PIN, HIGH);
  digitalWrite(CLA_PIN, LOW);
}

// modulo function allowing also -1
int mod(int a, int b) {
  return a < 0 ? (a + b) % b : a % b;
}