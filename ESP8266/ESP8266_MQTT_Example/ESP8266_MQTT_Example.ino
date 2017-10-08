/* This code adpated from
    https://home-assistant.io/blog/2015/10/11/measure-temperature-with-esp8266-and-report-to-mqtt/

A simple MQTT publisher of DS18S20 readings with an ESP-01 board
    
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>

#define wifi_ssid "<ssid>"
#define wifi_password "<wlan password>"

#define mqtt_server "<MQTT broker IP address>"  // the broker is the Fluksometer 

OneWire ds(2); // DS18S20 on GPIO2; use pullup resistor to Vcc

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

long lastMsg = 0;  

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
// publish a topic all 5 sec  
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    handle_ds18x20();
  }
}

// handling of temperature topic(s)
void handle_ds18x20( void ) {
  byte i, present = 0, type_s, data[12], addr[8];
  String topic;
  float value;
  if(!ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
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
    Serial.println("Device is not a DS18x20 family device.");
    return;
  }
  // format the topic string to /sensor/<DS id>/gauge
  topic = String("/sensor/");
  for(i=0;i<8;i++) topic = topic + String(addr[i], HEX);
  topic = topic + String("/gauge");
  // now get the DS readings
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion
  delay(1000);
  present = ds.reset();
  ds.select(addr);
  ds.write(0xbe); // read scratchpad
  for( i=0;i<9;i++) data[i] = ds.read();
  // compute read topic data
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
  value = raw / 16.0;
  const char * topicChar = topic.c_str(); // computer science loves casting :-/
  // format the payload to publish value and unit
  String payload;
  payload = String("[");
  payload += String(value);
  payload += String(", \"°C\"]");
  const char * payloadChar = payload.c_str();
  // and finally publish - hurray!
  client.publish(topicChar,payloadChar,false);
}

