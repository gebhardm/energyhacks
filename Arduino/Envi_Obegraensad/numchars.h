#ifndef numchars_h
#define numchars_h

const uint8_t chars[] PROGMEM = { 0b00000000,  // 0: zero
                                  0b00111000,
                                  0b01000100,
                                  0b01001100,
                                  0b01010100,
                                  0b01100100,
                                  0b01000100,
                                  0b00111000,

                                  0b00000000,  // 1: one
                                  0b00010000,
                                  0b00110000,
                                  0b00010000,
                                  0b00010000,
                                  0b00010000,
                                  0b00010000,
                                  0b00111000,

                                  0b00000000,  // 2: two
                                  0b00111000,
                                  0b01000100,
                                  0b00000100,
                                  0b00111000,
                                  0b01000000,
                                  0b01000000,
                                  0b01111100,

                                  0b00000000,  // 3: three
                                  0b00111000,
                                  0b01000100,
                                  0b00000100,
                                  0b00011000,
                                  0b00000100,
                                  0b01000100,
                                  0b00111000,

                                  0b00000000,  // 4: four
                                  0b00001000,
                                  0b00011000,
                                  0b00101000,
                                  0b01001000,
                                  0b01111100,
                                  0b00001000,
                                  0b00001000,

                                  0b00000000,  // 5: five
                                  0b01111100,
                                  0b01000000,
                                  0b01111000,
                                  0b00000100,
                                  0b00000100,
                                  0b01000100,
                                  0b00111000,

                                  0b00000000,  // 6: six
                                  0b00011000,
                                  0b00100000,
                                  0b01000000,
                                  0b01111000,
                                  0b01000100,
                                  0b01000100,
                                  0b00111000,

                                  0b00000000,  // 7: seven
                                  0b01111100,
                                  0b00000100,
                                  0b00001000,
                                  0b00010000,
                                  0b00100000,
                                  0b01000000,
                                  0b01000000,

                                  0b00000000,  // 8: eight
                                  0b00111000,
                                  0b01000100,
                                  0b01000100,
                                  0b00111000,
                                  0b01000100,
                                  0b01000100,
                                  0b00111000,

                                  0b00000000,  // 9: nine
                                  0b00111000,
                                  0b01000100,
                                  0b01000100,
                                  0b00111100,
                                  0b00000100,
                                  0b00001000,
                                  0b00110000,

                                  0b00000000,  // 10: °
                                  0b00001000,
                                  0b00010100,
                                  0b00001000,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,

                                  0b00000000,  // 11: C
                                  0b00111000,
                                  0b01000100,
                                  0b01000000,
                                  0b01000000,
                                  0b01000000,
                                  0b01000100,
                                  0b00111000,

                                  0b00000000,  // 12: %
                                  0b01100000,
                                  0b01100100,
                                  0b00001000,
                                  0b00010000,
                                  0b00100000,
                                  0b01001100,
                                  0b00001100,

                                  0b00000000,  // 13: H
                                  0b01000100,
                                  0b01000100,
                                  0b01000100,
                                  0b01111100,
                                  0b01000100,
                                  0b01000100,
                                  0b01000100,

                                  0b00000001,  // 14: cloud upper left
                                  0b00011110,
                                  0b00100100,
                                  0b00100000,
                                  0b01110000,
                                  0b10000000,
                                  0b10000000,
                                  0b01111111,

                                  0b11000000,  // 15: cloud upper right
                                  0b00111000,
                                  0b00100110,
                                  0b00000001,
                                  0b00000010,
                                  0b00000001,
                                  0b00000001,
                                  0b11111110,

                                  0b00010001,  // 16: rain lower left
                                  0b00000010,
                                  0b01000000,
                                  0b10001001,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,

                                  0b00100100,  // 17: rain lower right
                                  0b01000000,
                                  0b00010000,
                                  0b00100000,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,

                                  0b00100000,  // 18: sun upper left
                                  0b00010000,
                                  0b00001000,
                                  0b00000101,
                                  0b00000010,
                                  0b11111100,
                                  0b00000100,
                                  0b00000010,

                                  0b10000100,  // 19: sun upper right
                                  0b10001000,
                                  0b10010000,
                                  0b10100000,
                                  0b01000000,
                                  0b00100000,
                                  0b00111111,
                                  0b01000000,

                                  0b00000101,  // 20: sun lower left
                                  0b00001001,
                                  0b00010001,
                                  0b00100001,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,

                                  0b10100000,  // 21: sun lower right
                                  0b00010000,
                                  0b00001000,
                                  0b00000100,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,

                                  0b00100000,  // 22: cloud with sun upper left
                                  0b00010000,
                                  0b00001000,
                                  0b00000101,
                                  0b00000010,
                                  0b00011110,
                                  0b00100101,
                                  0b00100000,

                                  0b10000100,  // 23: cloud with sun upper right
                                  0b10001000,
                                  0b10010000,
                                  0b10100000,
                                  0b01000000,
                                  0b01111111,
                                  0b10110000,
                                  0b00001100,

                                  0b01110000,  // 24: cloud with sun lower left
                                  0b10000000,
                                  0b10000000,
                                  0b01111111,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,

                                  0b00000010,  // 25: cloud with sun lower right
                                  0b00000001,
                                  0b00000001,
                                  0b11111110,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000,
                                  0b00000000 };

const uint8_t nums[] PROGMEM{ 0b00000000,
                              0b00000000,
                              0b00000000,
                              0b00000000,

                              0b00000000,
                              0b00000100,
                              0b00000000,
                              0b00000000,

                              0b00000010,
                              0b00000000,
                              0b00001000,
                              0b00000000,

                              0b00000010,
                              0b00000100,
                              0b00001000,
                              0b00000000,

                              0b00001010,
                              0b00000000,
                              0b00001010,
                              0b00000000,

                              0b00001010,
                              0b00000100,
                              0b00001010,
                              0b00000000,

                              0b00001110,
                              0b00000000,
                              0b00001110,
                              0b00000000,

                              0b00001110,
                              0b00000100,
                              0b00001110,
                              0b00000000,

                              0b00001110,
                              0b00001010,
                              0b00001110,
                              0b00000000,

                              0b00001110,
                              0b00001110,
                              0b00001110,
                              0b00000000 };

// logarithmic LED fading
const uint8_t logled[] PROGMEM = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
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
#endif