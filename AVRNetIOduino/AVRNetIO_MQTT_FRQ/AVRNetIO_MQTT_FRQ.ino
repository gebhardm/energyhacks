/* AVRNetIO using MQTT to post frequency readings to an MQTT broker */
/* code is for ATmega32 at 16MHz */
#include <SPI.h>
#include <UIPEthernet.h>  // https://github.com/ntruchsess/arduino_uip
#include <PubSubClient.h>  // https://github.com/knolleary/pubsubclient

#define ledPin 1      // signal LED
#define acPin 10      // AC input via 10k resistor
#define sparePin 11   // a pin set to low to shield the AC signal
#define NET 50        // mains frequency
#define DIVIDER 8     // Prescaler for Timer
#define NETCNT (unsigned int) (F_CPU / DIVIDER / NET) // @16MHz: 40.000 ticks/period
#define CALIBRATE 280

// the actual capture routines
void setInterrupt0();
void setTimer1();
void startCapture();
unsigned long getCapture();

// frequency detection variables
volatile unsigned long sum = 0;
volatile unsigned char cnt = 0;
volatile byte idle = 1;

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
  // attach interrupt for AC detection
  setInterrupt0();
  // set timer for phase counting
  setTimer1();
}

void setInterrupt0() {
  //pinMode(acPin, INPUT_PULLUP);
  pinMode(acPin, INPUT);
  pinMode(sparePin, OUTPUT);
  digitalWrite(sparePin, LOW); // shield AC detection
  // attach INT0 to the net frequency detection routine
  GICR |= (1 << INT0);    // External interrupt request 0 enable
  //MCUCR |= (1 << ISC01);  // Interrupt sense control 0 on falling edge
  MCUCR |= (1 << ISC01) | (1 << ISC00);  // Interrupt sense control 0 on rising edge
}

void setTimer1() {
  // normal timer mode: count increasing, TOP = 0xffff
  TCCR1A = 0;
  // prescale timer1 counts
  switch (DIVIDER) {
  case 1: 
    TCCR1B = (1 << CS10); 
    break;
  case 8: 
    TCCR1B = (1 << CS11); 
    break;
  case 64: 
    TCCR1B = (1 << CS10) | (1 << CS11); 
    break;
  }
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
  unsigned long Hz;
  client.loop();
  if (!client.connected()) {
    client.connect("AVRNetIO");
  }
  startCapture();
  while (!idle); // wait for frequency measurement to end
  Hz = (unsigned long) F_CPU;
  Hz /= (unsigned long) DIVIDER;
  Hz *= 1000UL;
  Hz /= getCapture();
  //Hz = NETCNT - getCapture();
  // publish the detected net frequency
  client.publish("/sensor/frq/gauge",createPayload(Hz,"Hz",3));
  flashLED();
}

void startCapture() {
  cnt = 0;
  sum = 0;
  idle = 0;
  sei();
  TCNT1 = CALIBRATE;
}

unsigned long getCapture() {
  return (unsigned long) (sum / NET);
}

ISR(INT0_vect)
{
  sum += TCNT1;
  TCNT1 = CALIBRATE;
  cnt++;
  if (cnt ==  NET) { 
    idle = 1; 
    cli(); 
  }
}


