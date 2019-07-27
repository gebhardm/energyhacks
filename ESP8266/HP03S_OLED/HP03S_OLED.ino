// Simple example to read a HP03S pressure and temperature sensor
// and display the readings on an OLED display
// This uses code from
// https://github.com/philippG777/HP03S
// https://github.com/adafruit/Adafruit_SSD1306
// https://github.com/adafruit/Adafruit-GFX-Library
// https://github.com/knolleary/pubsubclient
// https://github.com/adafruit/Adafruit_Sensor
// https://github.com/adafruit/Adafruit_BME280_Library
// This code is adapted to run on a NodeMCU 1.0 (ESP-12E)

// Hardware purchased on https://www.pollin.de/ and
// https://www.az-delivery.de/ - no ad ;-)

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "xxxx" //"<ssid>"
#define wifi_password "yyyy" //"<wlan password>"

#define mqtt_server "zzzz" //"<MQTT broker IP address>" // the default broker is the Fluksometer 

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <HP03S.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define SERIAL_OUT 1
#define OLED_OUT 1

WiFiClient espClient;
PubSubClient client(espClient);
// display and sensors
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
HP03S hp03s(D3, D5); // XCLR, MCLK
Adafruit_BME280 bme;
IPAddress ip;

void setup()
{
  // set up WiFi and MQTT publisher
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  // set up display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //
  // set up HP03S sensor
  hp03s.begin();
#ifdef SERIAL_OUT
  hp03s.printAllValues();
#endif
  // set up BMP280 sensor
  bme.begin();
  // do a display clear and init
  display.clearDisplay(); // Clear display buffer
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  //  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.display();
}

// main loop
void loop()
{
  // deal with the WiFi connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // clear display
  display.clearDisplay(); // Clear display buffer
  display.setCursor(0, 0);     // Start at top-left corner
  // perform the measurement
  handle_HP03S();
  display.println();
  handle_BME280();
  // display output
  display.display();
  // wait 2 seconds
  delay(5000);
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
  ip = WiFi.localIP();
#ifdef SERIAL_OUT
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(ip);
#endif
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

void handle_HP03S() {
  // do the measurement and publish part
  hp03s.measure(); // You've to call this method every time You want to get new values.
  double temp = hp03s.getTemperature();
  double pres = hp03s.getPressure();
  // output to OLED display
#ifdef OLED_OUT
  display.print("Temperature: ");
  display.print(temp);
  display.println(" C");
  display.print("Pressure: ");
  display.print(pres);
  display.println(" hPa");
#endif
#ifdef SERIAL_OUT
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
  Serial.print("Pressure: ");
  Serial.print(pres);
  Serial.println(" hPa");
#endif
  // publish to MQTT
  String tempTopic = "/sensor/hp03s_T/gauge";
  String presTopic = "/sensor/hp03s_P/gauge";
  String tempValue = "[" + String(temp) + ", \"°C\"]";
  String presValue = "[" + String(pres) + ", \"hPa\"]";
  const char * tempT = tempTopic.c_str();
  const char * tempV = tempValue.c_str();
  const char * presT = presTopic.c_str();
  const char * presV = presValue.c_str();
  client.publish(tempT, tempV, false);
  client.publish(presT, presV, false);
}

void handle_BME280() {
  float temp = bme.readTemperature();
  float pres = bme.readPressure() / 100.00F;
  float hum = bme.readHumidity();
  float alt = bme.readAltitude(1004); // norm air pressure at sea level 1013.25 hPa
#ifdef OLED_OUT
  display.print("Temperature: ");
  display.print(temp);
  display.println(" C");
  display.print("Pressure: ");
  display.print(pres);
  display.println(" hPa");
  display.print("Humidity: ");
  display.print(hum);
  display.println(" %RH");
  display.print("Altitude: ");
  display.print(alt);
  display.println(" m");
#endif
#ifdef SERIAL_OUT
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
  Serial.print("Pressure: ");
  Serial.print(pres);
  Serial.println(" hPa");
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %RH");
  Serial.print("Altitude: ");
  Serial.print(alt);
  Serial.println(" m");
#endif
  // publish to MQTT
  String tempTopic = "/sensor/bme280_T/gauge";
  String presTopic = "/sensor/bme280_P/gauge";
  String humTopic = "/sensor/bme280_H/gauge";
  String altTopic = "/sensor/bme280_A/gauge";
  String tempValue = "[" + String(temp) + ", \"°C\"]";
  String presValue = "[" + String(pres) + ", \"hPa\"]";
  String humValue = "[" + String(hum) + ", \"%RH\"]";
  String altValue = "[" + String(alt) + ", \"m\"]";
  const char * tempT = tempTopic.c_str();
  const char * presT = presTopic.c_str();
  const char * humT = humTopic.c_str();
  const char * altT = altTopic.c_str();
  const char * tempV = tempValue.c_str();
  const char * presV = presValue.c_str();
  const char * humV = humValue.c_str();
  const char * altV = altValue.c_str();
  client.publish(tempT, tempV, false);
  client.publish(presT, presV, false);
  client.publish(humT, humV, false);
  client.publish(altT, altV, false);
}
