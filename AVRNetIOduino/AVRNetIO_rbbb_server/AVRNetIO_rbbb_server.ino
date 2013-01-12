// Real Bare Bone Board webserver on Pollin AVR-NetIO board 3.3V
// Adaptation of jcw's code done in January 2013 by Markus Gebhard, KA
// This is a demo of the RBBB running as webserver with the Ether Card
// 2010-05-28 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

#include <EtherCard.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 
  0x00,0x22,0xf9,0x01,0x2e,0x4a }; // Pollin AVR-NetIO
//static byte myip[] = { 192,168,1,203 };

byte Ethernet::buffer[900];
BufferFiller bfill;

void setup () {
  Serial.begin(57600);
  Serial.println("AVRNetIO Test\r\n");
  if (!ether.begin(sizeof Ethernet::buffer, mymac, SS))
    Serial.println( "Failed to access Ethernet controller");
  //  ether.staticSetup(myip);
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");
  ether.printIp("Assigned IP address ", ether.myip);
  // define direction of pins
  // ADC input
  // digital input
  pinMode(24, INPUT); 
  pinMode(25, INPUT); 
  pinMode(26, INPUT); 
  pinMode(27, INPUT); 
  // digital output
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(20, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
}

static word homePage() {
  bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
  "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<title>Simple AVRNetIO webserver</title>" 
    "<form method=get>"
    "<input type=checkbox name=o1 value=1 $S>Output 1</input><br/>"
    "<input type=checkbox name=o2 value=1 $S>Output 2</input><br/>"
    "<input type=checkbox name=o3 value=1 $S>Output 3</input><br/>"
    "<input type=checkbox name=o4 value=1 $S>Output 4</input><br/>"
    "<input type=checkbox name=o5 value=1 $S>Output 5</input><br/>"
    "<input type=checkbox name=o6 value=1 $S>Output 6</input><br/>"
    "<input type=checkbox name=o7 value=1 $S>Output 7</input><br/>"
    "<input type=checkbox name=o8 value=1 $S>Output 8</input><br/>"
    "<input type=submit></input>"
    "</form>"
    "Input 1 $D<br/>"
    "Input 2 $D<br/>"
    "Input 3 $D<br/>"
    "Input 4 $D<br/>"
    "ADC1 $D<br/>"
    "ADC2 $D<br/>"
    "ADC3 $D<br/>"
    "ADC4 $D<br/>"),
  (digitalRead(16)?"checked":""),
  (digitalRead(17)?"checked":""),
  (digitalRead(18)?"checked":""),
  (digitalRead(19)?"checked":""),
  (digitalRead(20)?"checked":""),
  (digitalRead(21)?"checked":""),
  (digitalRead(22)?"checked":""),
  (digitalRead(23)?"checked":""),
  digitalRead(24),
  digitalRead(25),
  digitalRead(26),
  digitalRead(27),
  analogRead(4),
  analogRead(5),
  analogRead(6),
  analogRead(7));
  return bfill.position();
}

static int getIntArg(const char* data, const char* key, int value =-1) {
  char temp[10];
  if (ether.findKeyVal(data + 6, temp, sizeof temp, key) > 0)
    value = atoi(temp);
  return value;
}

void loop () {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  int i;

  if (pos)  // check if valid tcp data is received
  {
    bfill = ether.tcpOffset();
    char* received = (char *) Ethernet::buffer + pos;
    Serial.println(received);
    getIntArg(received, "o1", 0)?digitalWrite(16, HIGH):digitalWrite(16, LOW);
    getIntArg(received, "o2", 0)?digitalWrite(17, HIGH):digitalWrite(17, LOW);
    getIntArg(received, "o3", 0)?digitalWrite(18, HIGH):digitalWrite(18, LOW);
    getIntArg(received, "o4", 0)?digitalWrite(19, HIGH):digitalWrite(19, LOW);
    getIntArg(received, "o5", 0)?digitalWrite(20, HIGH):digitalWrite(20, LOW);
    getIntArg(received, "o6", 0)?digitalWrite(21, HIGH):digitalWrite(21, LOW);
    getIntArg(received, "o7", 0)?digitalWrite(22, HIGH):digitalWrite(22, LOW);
    getIntArg(received, "o8", 0)?digitalWrite(23, HIGH):digitalWrite(23, LOW);
    ether.httpServerReply(homePage()); // send web page data
  }
}





