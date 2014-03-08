/* AVRNetIO using MQTT to post sensor readings to the FLM broker */
#include <SPI.h>
#include <UIPEthernet.h>  // https://github.com/ntruchsess/arduino_uip
#include <PubSubClient.h>  // https://github.com/knolleary/pubsubclient

#define ledPin 1

// The AVRNetIO's MAC as provided by Pollin
byte mac[]    = { 
  0x00, 0x22, 0xF9, 0x01, 0x2E, 0x49 };
// The FLM's IP address
byte flm[] = { 
  192, 168, 0, 50 };

void callback(char* topic, byte* payload, unsigned int length) {
  if (length>0) {
    // do something with the payload
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
        digitalWrite(ledPin, LOW);
        delay(100);
        digitalWrite(ledPin, HIGH);
        delay(100);
      };
      delay(800);
    }
  } 
  else {
    IPAddress myip = Ethernet.localIP();
  };
}

char* createPayload(int value, char* unit)
{
  char val[10], payload[60];
  // build payload [<value>,"<unit>"]
  itoa(value, val, 10);
  strcpy(payload,"");
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
    client.publish("/sensor/ADC1/gauge",createPayload(analogRead(A1),"DEC"));
    client.publish("/sensor/ADC2/gauge",createPayload(analogRead(A2),"DEC"));
    client.publish("/sensor/ADC3/gauge",createPayload(analogRead(A3),"DEC"));
    client.publish("/sensor/ADC4/gauge",createPayload(analogRead(A4),"DEC"));
  }
  else {
    client.connect("arduino");
  }
  digitalWrite(ledPin, HIGH);
  delay(250);
  digitalWrite(ledPin, LOW);
  delay(750);
}
