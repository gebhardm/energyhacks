#include "rfm01.h"

uint8_t _pinNIRQ;
uint8_t _pinSel;

volatile int rxFlag = LOW;  // interrupt indicator

// interrupt handler
void nIRQISR();

// constructor
RFM01::RFM01()
{
    RFM01(RFM01_CS, RFM01_IRQ);
}

// parameterized constructor
RFM01::RFM01(uint8_t pin_CS, uint8_t pin_nIRQ)
{
    _pinNIRQ = pin_nIRQ;
    _pinSel = pin_CS;
}

// the public initialization routine
void RFM01::begin()
{
    digitalWrite(_pinSel, HIGH);
    pinMode(_pinSel, OUTPUT);
    pinMode(_pinNIRQ, INPUT_PULLUP);
    
    initDevice();
    
    attachInterrupt(_pinNIRQ, nIRQISR, FALLING);
    
    // blink once to indicate init complete
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, HIGH);
    delay(10);
    digitalWrite(LEDPIN, LOW);
}

// the receive routine
uint8_t RFM01::receive(uint8_t *rxData, uint16_t *rxStatus, uint8_t msgLen)
{
    uint8_t rec, hStatus, lStatus;
    if (rxFlag) {
        digitalWrite(_pinSel, LOW);
        hStatus = SPI.transfer(0x00); // send status read command
        lStatus = SPI.transfer(0x00);
        rec = SPI.transfer(0x00);
        rxFlag = LOW;
    }
}

// the receive interrupt handler
void nIRQISR()
{
    rxFlag = HIGH;
}

// initialize the RFM01(S) module
void RFM01::initDevice()
{
    // control sequence as presented in the Pollin datasheet for the RFM01 receiver
    writeCtrlWord(0x0000);
    writeCtrlWord(0x893A); // configuration setting 433MHz, bandwidth 134kHz
    writeCtrlWord(0xA640); // frequency setting 434MHz
    writeCtrlWord(0xC847); // data rate 4.8kbps
    writeCtrlWord(0xC69B); // AFC
    writeCtrlWord(0xC42A); // data filter: digital filter, data quality threshold 4
    writeCtrlWord(0xC240); // low batt/clock divider 1.66MHz, 2.25V threshold
    writeCtrlWord(0xC080); // receiver setting clock recovery lock, -103dBm RSSI detect
    writeCtrlWord(0xCE88); // FiFo ??
    writeCtrlWord(0xCE8B); // FiFo ??
    writeCtrlWord(0xC081); // receiver setting: enable
}

// write a control word as bytes
void RFM01::writeCtrlBytes(uint8_t highByte, uint8_t lowByte)
{
    digitalWrite(_pinSel, LOW);
    SPI.transfer(highByte);
    SPI.transfer(lowByte);
    digitalWrite(_pinSel, HIGH);
}

// write control word
void RFM01::writeCtrlWord(uint16_t cmdWord)
{
    uint8_t h = cmdWord >> 8;
    uint8_t l = cmdWord;
    digitalWrite(_pinSel, LOW);
    SPI.transfer(h);
    SPI.transfer(l);
    digitalWrite(_pinSel, HIGH);
}
