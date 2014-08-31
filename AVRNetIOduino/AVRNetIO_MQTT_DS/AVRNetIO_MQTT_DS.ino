/* AVRNetIO using MQTT to post sensor readings to the FLM broker */
#include <SPI.h>
#include <UIPEthernet.h>  // https://github.com/ntruchsess/arduino_uip
#include <PubSubClient.h>  // https://github.com/knolleary/pubsubclient
#include <OneWire.h> // http://www.pjrc.com/teensy/td_libs_OneWire.html

// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

OneWire  ds(28);  // on pin 28/ADC1

const char hex[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

#define ledPin 1
#define DEBUG 0

//Global variables for Interrupt computation
unsigned long sum = 0, Hz = 0, last = 0;
byte cnt = 0;
char payload[60];

// The AVRNetIO's MAC as provided by Pollin
byte mac[]    = { 
  0x00, 0x22, 0xF9, 0x01, 0x2E, 0x49 };
// The FLM's IP address
byte flm[] = { 
  192, 168, 0, 50 };

void callback(char* topic, byte* payload, unsigned int length) {
  if (length>0) {
    // do something with the payload - unused here... 
  }
}

EthernetClient ethClient;
PubSubClient client(flm, 1883, callback, ethClient);

void setup()
{
  // initialize serial interface
  if (DEBUG) Serial.begin(9600);
  // initialize ethernet controller by DHCP
  if(Ethernet.begin(mac) == 0) {
    // something went wrong
    while(true)
    { 
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, LOW);
      delay(500);        
    }
  } 
  else {
    IPAddress myip = Ethernet.localIP();
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
  if (dp > 0) {
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
  // handling of temperature sensor(s)
  byte i, present = 0, type_s, data[12], addr[8];
  char addrhex[17];
  char topic[32] = "";
  //
  client.loop();
  if (client.connected()) {
    if(!ds.search(addr)) {
      ds.reset_search();
      delay(250);
    }
    switch (addr[0]) {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      if (DEBUG) Serial.println("Device is not a DS18x20 family device.");
      return;
    }
    // make address string
    for (i=0;i<8;i++) {
      addrhex[i*2] = hex[addr[i] >> 4];
      addrhex[i*2+1] = hex[addr[i]&0xF];
    }
    addrhex[16] = 0x00;
    strcat(topic,"/sensor/");
    strcat(topic,addrhex);
    strcat(topic,"/gauge");
    if (DEBUG) Serial.println(topic);
    //
    ds.reset();
    ds.select(addr);
    ds.write(0x44,1); // start conversion
    delay(1000);
    present = ds.reset();
    ds.select(addr);
    ds.write(0xbe); // read scratchpad
    for( i=0;i<9;i++) data[i] = ds.read();
    // compute read sensor data
    unsigned int raw = (data[1] << 8) | data[0];
    if (type_s) {
      raw = raw << 3; // 9 bit resolution default
      if (data[7] == 0x10) {
        // count remain gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
      }
    } 
    else {
      byte cfg = (data[4] & 0x60);
      if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
      else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
      // default is 12 bit resolution, 750 ms conversion time
    }
    client.publish(topic,createPayload(raw*100/16,"°C",2));
  }
  else {
    client.connect("arduino");
  }
  delay(1000);
}
