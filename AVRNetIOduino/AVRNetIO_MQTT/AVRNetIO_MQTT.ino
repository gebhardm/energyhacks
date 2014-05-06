/* AVRNetIO using MQTT to post sensor readings to the FLM broker */
#include <SPI.h>
#include <UIPEthernet.h>  // https://github.com/ntruchsess/arduino_uip
#include <PubSubClient.h>  // https://github.com/knolleary/pubsubclient

#define ledPin 1
#define AVG 50
#define DIVIDER 1

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
  //  initialize ethernet controller by DHCP
  if(Ethernet.begin(mac) == 0) {
    // something went wrong
    while(true)
    { 
      for (int i=0;i<3;i++){
        if (digitalRead(ledPin) == LOW) digitalWrite(ledPin, HIGH);
        else digitalWrite(ledPin, LOW);
      };
      delay(300);
    }
  } 
  else {
    IPAddress myip = Ethernet.localIP();
  };
  // attach INT0 to the net frequency detection routine
  attachInterrupt(0, interrupt, FALLING);
  //GICR |= (1 << INT0);
  //MCUCR |= (1 << ISC11);
  //sei();
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
  client.loop();
  if (client.connected()) {
    client.publish("/sensor/Temperature1/gauge",createPayload((analogRead(A0)*61-17412),"°C",2));
    client.publish("/sensor/Temperature2/gauge",createPayload((analogRead(A1)*61-17412),"°C",2));
    client.publish("/sensor/NetFrequency/gauge",createPayload(Hz,"Hz",3));
  }
  else {
    client.connect("arduino");
  }
  delay(1000);
}

void interrupt( void )
//ISR(INT0_vect)
{
  unsigned long usec = micros(); 
  unsigned long diff = usec - last;
  sum += diff;
  cnt++;
  if (cnt == AVG) {
    sum /= AVG;
    Hz = 1000000000UL / sum;
    sum = 0;
    cnt = 0;
  }
  last = usec;
}
