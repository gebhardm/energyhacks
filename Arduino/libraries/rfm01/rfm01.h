/*
 
 Header file for RFM01(S) receiver utilization
 
 Refer to Pollin Electronic "Funkmodul RFM01-433 Empfangsmodul"
 Refer to HopeRF Electronics specification http://www.hoperf.com/upload/rf/RFM01.pdf
 
 Idea in parts adapted from https://github.com/ArcticSaturn/RFM01
 
 Markus Gebhard, Karlsruhe, (c) 2016
 
 The MIT License (MIT)
 
 Copyright (c) 2016 Markus Gebhard
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify,
 merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 */

#ifndef RFM01_H
#define RFM01_H

#define RFM01_CS 10
#define RFM01_IRQ 3

#ifndef LEDPIN
#define LEDPIN   13  // default Arduino LED pin
#endif

// used SPI interface to attach receiver module
#include <SPI.h>

class RFM01 {
public:
    // default constructor
    RFM01();
    // constructor with SEL pin and nIRQ pin
    RFM01(uint8_t selPin, uint8_t irqPin);
    // initialize
    void begin();
    // receive byte array
    uint8_t receive(uint8_t *rxData, uint16_t *rxStatus, uint8_t msgLen);
private:
    // write RFM01 control commands
    void initDevice();
    void writeCtrlBytes(uint8_t highByte, uint8_t lowByte);
    void writeCtrlWord(uint16_t cmdWord);
};

#endif
