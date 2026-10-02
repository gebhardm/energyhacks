// Read BME280 temperature/pressure/humidity via Matter ESP32-C6
// SDA/SCL: 23/22 --> 20/19
#include <Arduino.h>
#include <Wire.h>
#include <Matter.h>           // uses Espressif esp32 board manager 4.0.0-rc1
#include <Adafruit_BME280.h>  // GYBMPE board with Bosch barometric sensor

MatterButton button;  // use boot button for decommissioning
MatterTemperatureSensor bme280temp;
MatterPressureSensor bme280pres;
MatterHumiditySensor bme280hum;

Adafruit_BME280 bme;  // the sensor reading library, thanks to Lady Ada

// Product Information
static const char *kVendorName = "Bosch";
static const char *kProductName = "BME280";
static const char *kDeviceName = "Environment Sensor";
static const char *kSerialNumber = "00000001";
static const char *kHardwareVersionString = "2026";
static const uint16_t kHardwareVersion = 1;
static const uint16_t kSetupDiscriminator = 0xF00;
static const uint32_t kSetupPasscode = 20202021;

void setup() {
  button.begin(BOOT_PIN);
  Serial.begin(115200);
  // start BME280
  Wire.begin(20, 19);
  if (!bme.begin(0x76, &Wire)) {
    log_e("No BME280 Sensor %i", bme.sensorID());
    while (1) {};
  }
  // start the sensors
  bme280temp.begin();
  bme280pres.begin();
  bme280hum.begin();
  // set Matter commissioning data
  Matter.setVendorName(kVendorName);
  Matter.setProductName(kProductName);
  Matter.setDeviceName(kDeviceName);
  Matter.setSerialNumber(kSerialNumber);
  Matter.setHardwareVersion(kHardwareVersion);
  Matter.setHardwareVersionString(kHardwareVersionString);
  Matter.setSetupDiscriminator(kSetupDiscriminator);
  Matter.setSetupPasscode(kSetupPasscode);
  // start the Matter stack
  Matter.begin();
  if (!Matter.isDeviceCommissioned()) {
    Serial.println("\nDevice not commissioned. Use the code/QR below:");
    Serial.printf("  Manual pairing code : %s\n", Matter.getManualPairingCode().c_str());
    Serial.printf("  QR code URL         : %s\n", Matter.getOnboardingQRCodeUrl().c_str());
    uint32_t t = 0;
    while (!Matter.isDeviceCommissioned()) {
      delay(100);
      if (++t % 50 == 0) Serial.println("  Waiting for commissioning…");
    }
  }
  Serial.println("Device commissioned. Ready.");
  // check all up and running
  matterWaitUntilReady();
}

void loop() {
  matterRestartIfNoFabric();
  // set sensor readings
  double temp = bme.readTemperature();
  double pres = bme.readPressure() / 100.00F;
  double hum = bme.readHumidity();
  static uint32_t lastSensorUpd = 0;
  // update sensor data all 10 sec only
  if (millis() - lastSensorUpd >= 10000UL) {
    lastSensorUpd = millis();
    bme280temp.setTemperature(temp);
    bme280pres.setPressure(pres);
    bme280hum.setHumidity(hum);
  }
  log_i("Temperature %.02f °C", temp);
  log_i("Pressure %.02f hPa", pres);
  log_i("Relative Humidity %.02f %%", hum);
  // check for decommissioning
  matterButtonEvent_t ev;
  while ((ev = button.poll()) != MATTER_BUTTON_NONE) {
    if (ev == MATTER_BUTTON_LONG_HOLD) {
      Serial.println("Decommissioning BME280 Sensor Matter Accessory. It shall be commissioned again.");
      Matter.decommission();
    }
  }
  // ... and wait
  delay(500);
}
