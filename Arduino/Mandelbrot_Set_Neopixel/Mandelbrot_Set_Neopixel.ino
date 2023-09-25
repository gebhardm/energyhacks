// Mandelbrot Set code taken from https://hackaday.io/project/170499-vga-shield-wing/log/198173-detour-arduino-benchmark-with-mandelbrot
#include <Adafruit_NeoPixel.h>
// NeoPixel pin and number of pixels to address
#define NEO_PIN 12
#define NUM_PXL 64
// calculation grid
#define MAX_Y 8
#define MAX_X 8

void reset_grid(void);
void display_grid(void);
void Mandelbrot(float, float, float, float);

uint8_t grid[MAX_Y * MAX_X];

Adafruit_NeoPixel pixels(NUM_PXL, NEO_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // initialise the NeoPixel matrix
  pixels.begin();
  pixels.clear();
  // set initial, randomly set grid
  reset_grid();
  display_grid();
}

void loop() {
  Mandelbrot(-2, 1, -1.5, 1.5);
  display_grid();
  delay(5000);
  reset_grid();
}

// Draw a Apfelmaennchen
void Mandelbrot(float Xn, float Xp, float Yn, float Yp) {
  float x0, y0, xtemp;
  float x = 0;
  float y = 0;
  int iteration = 0;
  int max_iteration = 256;

  for (uint8_t Py = 0; Py < MAX_Y; Py++) {
    y0 = (Yp - Yn) / MAX_Y * Py + Yn;
    for (uint8_t Px = 0; Px < MAX_X; Px++) {
      x0 = (Xp - Xn) / MAX_X * Px + Xn;
      x = 0;
      y = 0;
      iteration = 0;
      while ((x * x + y * y <= 2 * 2) && (iteration < max_iteration)) {
        xtemp = x * x - y * y + x0;
        y = 2 * x * y + y0;
        x = xtemp;
        iteration++;
      }
      grid[Py * MAX_Y + Px] = (uint8_t) iteration;
    }
  }
}

// reset the grid - use ramdomizer to get different generation zeros
void reset_grid() {
  for (int y = 0; y < MAX_Y; y++) {
    for (int x = 0; x < MAX_X; x++) {
      grid[y * MAX_Y + x] = 0;
    }
  }
}

// display the current grid to the LED matrix
void display_grid() {
  for (uint8_t y = 0; y < MAX_Y; y++) {
    for (uint8_t x = 0; x < MAX_X; x++) {
      // pass the grid to the LED matrix
      uint8_t r = grid[y * MAX_Y + x] & 0b11111000;
      uint8_t g = grid[y * MAX_Y + x] & 0b01111100;
      uint8_t b = grid[y * MAX_Y + x] & 0b00011111;
      pixels.setPixelColor(y * MAX_Y + x, pixels.Color(r, g, b));
    }
  }
  // transfer the cells to the LED output
  pixels.show();
}