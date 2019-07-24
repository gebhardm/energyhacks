// Simple example to read a HP03S pressure and temperature sensor
// and display the readings on an OLED display
// This uses code from
// https://github.com/philippG777/HP03S
// https://github.com/adafruit/Adafruit_SSD1306
// https://github.com/adafruit/Adafruit-GFX-Library
// https://github.com/knolleary/pubsubclient
// This code is adapted to run on a NodeMCU 1.0 (ESP-12E)

// Hardware purchased on https://www.pollin.de/ and
// https://www.az-delivery.de/ - no ad ;-)

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "xxxx" //"<ssid>"
#define wifi_password "yyyy" //"<wlan password>"

#define mqtt_server "zzzz" //"<MQTT broker IP address>" 

//#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#include "HP03S.h"

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
HP03S hp03s(D3, D5); // XCLR, MCLK

void setup()
{
  // set up WiFi and MQTT publisher
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  // set up display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //
  // set up sensor
  hp03s.begin();
  hp03s.printAllValues();
  // do a display clear and init
  display.clearDisplay(); // Clear display buffer
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  //  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.display();
}


void loop()
{
  // deal with the WiFi connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // perform the measurement
  handle_sensor();
  // wait 2 seconds
  delay(2000);
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

void handle_sensor() {
  // do the measurement and publish part
  hp03s.measure(); // You've to call this method every time You want to get new values.
  double temp = hp03s.getTemperature();
  double pres = hp03s.getPressure();
  // output to OLED display
  display.clearDisplay(); // Clear display buffer
  display.setCursor(0, 0);     // Start at top-left corner
  display.print("Temp: ");
  display.print(temp);
  display.println(" C");
  display.print("Pres: ");
  display.print(pres);
  display.print(" hPa");
  display.display();
  // publish to MQTT
  String tempTopic = "/sensor/temperature/gauge";
  String presTopic = "/sensor/pressure/gauge";
  String tempValue = "[" + String(temp) + ", \"°C\"]";
  String presValue = "[" + String(pres) + ", \"hPa\"]";
  const char * tempT = tempTopic.c_str();
  const char * tempV = tempValue.c_str();
  client.publish(tempT,tempV,false);
  const char * presT = presTopic.c_str();
  const char * presV = presValue.c_str();
  client.publish(presT,presV,false);
}
