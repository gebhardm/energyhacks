/* AVRNetIO using MQTT to post frequency readings to an MQTT broker */
/* code is for ATmega32 at 16MHz */
/* Frequency measurement adapted from Tynemouth Software "improved Arduino Frequency Counter" */
/* I am not yet satisfied... */

#include <SPI.h>
#include <UIPEthernet.h>  // https://github.com/ntruchsess/arduino_uip
#include <PubSubClient.h>  // https://github.com/knolleary/pubsubclient

#define ledPin 1      // signal LED
#define acPin 10      // AC input via 10k resistor

// frequency detection variables
volatile unsigned long startTime;
volatile unsigned long endTime;
volatile unsigned long count;
float lastFreq = 0;

// the frequency measurement interrupt routines
void trigger();
void start();
void pulse();
float getFrequency(unsigned int);

void trigger()
{
  attachInterrupt(0, start, RISING);
}

void start()
{
  startTime = micros();
  attachInterrupt(0, pulse, RISING);
}

void pulse()
{
  endTime = micros();
  count++;
}

float getFrequency(unsigned int sampleTime)
{
  count = 0;
  attachInterrupt(0, trigger, RISING);
  delay(sampleTime);
  detachInterrupt(0);
  if (count==0) return 0;
  else return float(1000000 * count) / float(endTime - startTime);
}

//Global variables for communication
char payload[60];

// The AVRNetIO's MAC as provided by Pollin
// Please use your own...
byte mac[]    = { 
  0x00, 0x22, 0xF9, 0x01, 0x02, 0x03 };
// Provide here the MQTT broker's IP address the Arduino shall conntect to
uint8_t broker[] = { 
  192, 168, 0, 50 };
uint16_t port = 1883;

IPAddress myip;

// define callback routine
void callback(char* topic, byte* payload, unsigned int length) {
  // do something with the payload - unused here... 
}

// now initialize the network connection and MQTT broker access
EthernetClient ethClient;
PubSubClient client(broker, port, callback, ethClient);

// flash the LED to display a condition
void flashLED( void ) {
  digitalWrite(ledPin, LOW);
  for(int i=0;i<10000;i++) __asm__("nop\n\t");
  digitalWrite(ledPin, HIGH);
}

void toggleLED( void ) {
  digitalWrite(ledPin, !digitalRead(ledPin));
}

void setup()
{
  int i; // loop counter
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  //  initialize ethernet controller by DHCP
  if(!Ethernet.begin(mac)) {
    // something went wrong
    while(true)
    { 
      for(i=0;i<3;i++) flashLED();
      for(i=0;i<60000;i++) __asm__("nop\n\t");
    }
  } 
  else {
    myip = Ethernet.localIP();
  };
}

char* createPayload(long value, char* unit, byte dec)
{
  char val[10];
  int len, dp;
  // build payload [<value>,"<unit>"]
  ltoa(value, val, 10);
  // set decimal point
  len = strlen(val);
  dp  = len - dec;
  if ((dp > 0) && (len > dec)) {
    for(int i=len; i > dp; i--) {
      val[i+1] = val[i];
    }
    if (dp < len)
      val[dp] = '.';
  }
  // now build the payload
  payload[0] = 0x00;
  strcat(payload,"[");
  strcat(payload,val);
  strcat(payload,",\"");
  strcat(payload,unit);
  strcat(payload,"\"]");
  return payload;
}

void loop()
{
  float freq = 0;
  if (lastFreq > 5000 || lastFreq == 0) freq = 1000 * getFrequency(1000);
  else freq = 1000 * getFrequency(3333);
  lastFreq = freq;

  client.loop();
  if (!client.connected()) {
    client.connect("AVRNetIO");
  }
  // publish the detected net frequency
  client.publish("/sensor/netfrq/gauge",createPayload(freq,"Hz",3));
  flashLED();
}
