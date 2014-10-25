/* AVRNetIO using MQTT to post sensor readings to the FLM broker */
#define AVRNETIO 1
#define DEBUG 0

// use the SPI capability to connect the Ethernet shield
#include <SPI.h>
// load the correct Ethernet library for your Arduino/derivate
#if AVRNETIO
#include <UIPEthernet.h> // https://github.com/ntruchsess/arduino_uip - use with AVRNetIO
#else
// /!\ the stupid preprocessor seems to not compile conditional #if on #include !!!
//#include <Ethernet.h> // use with Arduino Ethernet or Ethernet Shield
#endif
// the MQTT publish/subscribe library
#include <PubSubClient.h>   // https://github.com/knolleary/pubsubclient
// the Dallas/Maxim one-wire library
#include <OneWire.h>        // http://www.pjrc.com/teensy/td_libs_OneWire.html
// use the DHT library from Adafruit
#include <DHT.h>            // http://github.com/adafruit/DHT-sensor-library
#define DHTTYPE DHT11
#if AVRNETIO
#define DHTPIN 29           // on pin 28/ADC1 on AVRNetIO
#define WIREPIN 28
#else
#define DHTPIN 29
#define WIREPIN 8           // on pin 8 of Arduino Ethernet
#endif
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

// for translating HEX to printout
const byte hex[16] = {
  '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

// define the debug LED port
#if AVRNETIO
#define ledPin 1  // LED is connected at J11 cathode to ATmega, 10k replaced by 1k to +5V
#else
#define ledPin 9 // LED on pin 9 of Arduino Ethernet
#endif

char payload[60];
/*************************************************************************/
// The AVRNetIO's MAC as provided by Pollin - use your own!
#if AVRNETIO
byte mac[] = { 
  0x00, 0x22, 0xF9, 0x01, 0x2E, 0x49 };
#else
// the Arduino Ethernat MAC is written on the PCB backside - use your own!
byte mac[] = {
  0x90, 0xa2, 0xda, 0x0e, 0x50, 0x1b};  
#endif  
OneWire  ds(WIREPIN);  
DHT dht(DHTPIN, DHTTYPE, 2);
// The FLM's IP address - replace with your broker's IP address
byte flm[] = { 
  192, 168, 0, 50 };
/*************************************************************************/

void callback(char* topic, byte* payload, unsigned int length) {
  if (length>0) {
    // do something with the payload - unused here... 
  }
}

EthernetClient ethClient;
PubSubClient client(flm, 1883, callback, ethClient);

// flash the LED to display a condition
void flashLED( void ) {
#ifdef AVRNETIO  
  digitalWrite(ledPin, LOW);
  delay(25);
  digitalWrite(ledPin, HIGH);
#else
  digitalWrite(ledPin, HIGH);
  delay(25);
  digitalWrite(ledPin, LOW);
#endif
}

// the setup routine to initialize everything needed
void setup()
{
  // initialize serial interface if required for debugging purposes
#if DEBUG
  Serial.begin(9600);
#endif
  // enable LED
  pinMode(ledPin, OUTPUT);
#if AVRNETIO 
  digitalWrite(ledPin, HIGH); // LED is active low on AVRNetIO
#else
  digitalWrite(ledPin, LOW);
#endif 
  // initialize ethernet controller by DHCP
  if(Ethernet.begin(mac) == 0) {
    // something went wrong
    while(true)
    { 
      for (int i=0;i<3;i++) {
        flashLED();        
      }
      delay(1000);
    }
  } 
  else {
    IPAddress myip = Ethernet.localIP();
  };
  // initialize the humidity sensor
  dht.begin();
}

// format the payload to be published
char* createPayload(long value, char* unit, byte dec)
// <val>, <unit>, <decimal point position>
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
#if DEBUG
  Serial.println(payload);
#endif
  return payload;
}

// handling of temperature sensor(s)
void handle_ds18x20( void ) {
  byte i, present = 0, type_s, data[12], addr[8];
  char addrhex[17];
  char topic[32] = "";
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
#if DEBUG
    Serial.println("Device is not a DS18x20 family device.");
#endif
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
#if DEBUG
  Serial.println(topic);
#endif
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

void handle_DHT( void ) {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
#if DEBUG
    Serial.println("Failed to read from DHT sensor!");
#endif
    return;
  }
  // Compute heat index
  // Must send in temp in Fahrenheit!
  float hi = dht.computeHeatIndex(f, h);
#if DEBUG
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hi);
  Serial.println(" *F");
#endif
  long hum = (long) (h * 100);
  char t_hum[32] = "/sensor/DHT_RH/gauge";
  client.publish(t_hum,createPayload(hum,"%RH",2));
  long tem = (long) (t * 100);
  char t_t[32] = "/sensor/DHT_T/gauge";
  client.publish(t_t,createPayload(tem,"°C",2));
  long hin = (long) (hi * 100);
  char t_hi[32] = "/sensor/DHT_HI/gauge";
  client.publish(t_hi,createPayload(hin,"°F",2));
}

// main loop
void loop()
{
  client.loop();
  if (client.connected()) {
    handle_ds18x20();
    handle_DHT();
  }
  else {
    client.connect("arduino");
  }
  flashLED();
  delay(2500);
}





