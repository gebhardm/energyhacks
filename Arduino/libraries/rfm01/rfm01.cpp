#include "rfm01.h"

uint8_t _pinNIRQ;
uint8_t _pinSel;

volatile int rxFlag = LOW;  // interrupt indicator
int cntRec = 0; // counter of received bytes

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
    uint16_t status;
    uint8_t  rec;
    uint8_t msgOK = 0; // keep 0 until message length is reached
    if (rxFlag) {
        // the receive sequence
        digitalWrite(_pinSel, LOW);
        status = SPI.transfer16(0x0000); // send status read command
        rec    = SPI.transfer(0x00);     // now read from the FiFo buffer
        digitalWrite(_pinSel, HIGH);
        // store the received byte
        rxData[cntRec] = rec;
        // store the status
        rxStatus[cntRec] = status;
        cntRec++;
        // wait for next received message
        rxFlag = LOW;
    }
    // transfer the received message
    if(cntRec==msgLen){
        writeCtrlWord(0xCE88); // FiFo ??
        writeCtrlWord(0xCE8B); // FiFo ??
        msgOK = 1;
        cntRec = 0;
    }
    return msgOK;
}

// the receive interrupt handler
void nIRQISR()
{
    rxFlag = HIGH;
}

// initialize the RFM01(S) module
void RFM01::initDevice()
{
    uint16_t status;
    // control sequence as presented in the Pollin datasheet for the RFM01 receiver
    digitalWrite(_pinSel, LOW);
    status = SPI.transfer16(0x0000);  // send read status and receive this
    digitalWrite(_pinSel, HIGH);
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

// write control word
void RFM01::writeCtrlWord(uint16_t cmdWord)
{
    digitalWrite(_pinSel, LOW);
    SPI.transfer16(cmdWord);
    digitalWrite(_pinSel, HIGH);
}
