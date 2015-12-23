// Ardunio Uno with Pollin ENC28J60 Network module
// receiving NTP packages (http://pollin.de => "Netzwerk-Modul mit ENC28J60")
// In parts this code uses the tutorial code provided on
// https://www.arduino.cc/en/Tutorial/UdpNtpClient
// To use with an Arduino Leonardo you have to alter the
// Slave Select declaration in utility/Enc28J60Network.h - ENC28J60_CONTROL_CS
// SPI_SS may remain unchanged as it is not used... (?!)
// --> https://github.com/ntruchsess/arduino_uip/blob/master/utility/Enc28J60Network.h#L30
#include <SPI.h>
#include <UIPEthernet.h>  // https://github.com/ntruchsess/arduino_uip

#define BAUD 9600

unsigned int localPort = 2390; // local UDP port

// declare a valid NTP server, e.g. by sending a ping to time.nist.gov and
// watch how the address is resolved
IPAddress timeServer(24, 56, 178, 140); // time.nist.gov NTP => ntp1.glb.nist.gov

// NTP timestamp in first 48 message bytes
#define NTP_PACKET_SIZE 48
byte packetBuffer[ NTP_PACKET_SIZE ];

byte mac[] = {
  0x00, 0x22, 0xF9, 0x01, 0x02, 0x03  // please use you own MAC :-)
};
// declare the UDP package handler
EthernetUDP Udp;

// set up serial interface and Ethernet connection
void setup() {
  Serial.begin(BAUD);
  while (!Serial) {}
  while (!Ethernet.begin(mac)) {}
  //
  IPAddress myip = Ethernet.localIP(); // just for the records...
  Serial.println(myip);
  //
  Udp.begin(localPort);
}

void loop() {
  sendNTPpacket(timeServer);
  delay(1000); // you may use a longer delay to prevent server flooding
  if (Udp.parsePacket())
  {
    Udp.read(packetBuffer, NTP_PACKET_SIZE);
    // refer to http://www.ntp.org/ntpfaq/NTP-s-algo.htm
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secSince1900 - seventyYears;
    Serial.print("UTC time : ");
    byte hours = (epoch % 86400L) / 3600;
    byte minutes = (epoch % 3600) / 60;
    byte seconds = (epoch % 60);
    if (hours<10) Serial.print("0");
    Serial.print(hours);
    Serial.print(":");
    if (minutes<10) Serial.print("0");
    Serial.print(minutes);
    Serial.print(":");
    if (seconds<10) Serial.print("0");
    Serial.println(seconds);
  }
}

// send the NTP time request; the server answers with the same package
void sendNTPpacket(IPAddress& address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
