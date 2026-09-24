// Copyright 2025-2026 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
 * Matter PM2.5 Concentration Sensor
 *
 * Creates a Matter Air Quality Sensor endpoint (device type 0x002C) with a
 * PM2.5 Concentration Measurement cluster (Matter spec cluster 0x042A).
 * The NumericMeasurement feature is enabled so controllers receive a float
 * MeasuredValue in µg/m³.
 *
 * How this works with arduino-esp32:
 *   <Matter.h> transitively includes <esp_matter.h>, which pulls in
 *   esp_matter_endpoint.h and esp_matter_cluster.h — giving access to
 *   the esp_matter::endpoint::air_quality_sensor and
 *   esp_matter::cluster::pm25_concentration_measurement namespaces
 *   without any separate generated headers.
 *
 * This code came to life following https://github.com/espressif/arduino-esp32/issues/12920
 * Many thanks for the quick support!!
 *
 * Hardware: ESP32 with large app partition scheme.
 *
 * Commissioning:
 *   - Manual pairing code.
 *   - Hold BOOT button for 5 s to factory-reset / decommission.
 *
 * readPM25Sensor() takes data from an IKEA Vindriktning PM1006 particle sensor via serial 
 * connection pin IO16
 * 
 * Programmed with some help from Claude Code (yes, everybody has to do "something with AI" these days)
 * and revised after _init() issues with the help of the Matter Library maintainers
 *
 * Used hardware:
 * IKEA Vindriktning Particle Measurement Sensor
 * ESP32-C3 Mini board
 * Voltage level shifter from Vindriktning 5V to the ESP's 3V3, a simple BS170 with two 10k resistors
 *
 */

#include <Arduino.h>
#include <Matter.h>
#include "MatterPm25Sensor.h"

#if !CONFIG_ENABLE_CHIPOBLE
#include <WiFiManager.h>
#endif

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
static float g_lastPM25 = -1.0f;
uint8_t cnt = 0;
int pm1006[20];

MatterPm25Sensor Pm25Sensor;

// Product Information
static const char *kVendorName = "IKEA of Sweden";
static const char *kProductName = "Vindriktning";
static const char *kDeviceName = "Air Sensor";
static const char *kSerialNumber = "2111 50050";
static const char *kHardwareVersionString = "E2014";
static const uint16_t kHardwareVersion = 1;
static const uint16_t kSetupDiscriminator = 0xF01;
static const uint32_t kSetupPasscode = 20202024;

// ---------------------------------------------------------------------------
// Decommission button (BOOT button)
// ---------------------------------------------------------------------------
static const uint8_t kButtonPin = BOOT_PIN;
static const uint32_t kDecommissionTimeout = 5000;
static bool g_buttonPressed = false;
static uint32_t g_buttonPressedAt = 0;

// ---------------------------------------------------------------------------
// Sensor data taken from what was read via serial connection from PM1006
// ---------------------------------------------------------------------------
static float readPM25Sensor() {
  uint8_t checksum = 0;
  static float value = 0.0f;
  for (uint8_t i = 0; i < 20; i++) { checksum += pm1006[i]; }
  if (checksum == 0) value = (float)((pm1006[5] << 8) | pm1006[6]);
  else {
    log_e("PM25: Sensor checksum invalid.");
    cnt = 0;  // reset read counter as something must went wrong
  }
  return value;
}

// ---------------------------------------------------------------------------
// Arduino setup
// ---------------------------------------------------------------------------
void setup() {
  pinMode(kButtonPin, INPUT_PULLUP);
  Serial.begin(115200);
  // PM1006 Reading --> RX/TX = 16/17 (actually UART2, TX unused)
  Serial1.setPins(16, 17);
  Serial1.begin(9600);
  // configure WiFi connection via WiFi manager frontend UI
  // search WLAN for "MatterDevice" and log in with "password"
#if !CONFIG_ENABLE_CHIPOBLE
  WiFiManager wm;
  while (wm.autoConnect("MatterDevice", "password") == false) {
    Serial.print('.');
    delay(500);
  };
  log_i("connected.");
#endif
  // Setup PM2.5 Concentration Measurement
  Pm25Sensor.begin();

  // Set Product Information
  Matter.setVendorName(kVendorName);
  Matter.setProductName(kProductName);
  Matter.setDeviceName(kDeviceName);
  Matter.setSerialNumber(kSerialNumber);
  Matter.setHardwareVersion(kHardwareVersion);
  Matter.setHardwareVersionString(kHardwareVersionString);
  // Not the Arduino test pair 0xF00 / 20202021. Use the generated pairing codes below.
  Matter.setSetupDiscriminator(kSetupDiscriminator);
  Matter.setSetupPasscode(kSetupPasscode);
  // declare a distinct endpoint name per the product name
  matterSetExampleIdentity(kProductName);

  // 3. Start the Matter stack (must be called after all endpoints are set up).
  Matter.begin();

  // 4. Wait for commissioning. - this is what all MatterEndPoint examples do
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
}

// ---------------------------------------------------------------------------
// Arduino loop
// ---------------------------------------------------------------------------
void loop() {
  // restart if the Matter configuration is not complete
  matterRestartIfNoFabric();
  // read from the particle measurement sensor
  if (Serial1.available() > 0) {
    pm1006[cnt] = Serial1.read();
    cnt++;
    if (cnt == 20) cnt = 0;  // the sensor reading hold 20 bytes
  }
  static uint32_t lastSensorRead = 0;
  if (millis() - lastSensorRead >= 10000UL) {
    lastSensorRead = millis();
    float pm25 = readPM25Sensor();
    log_i("Sensor reading: %.2f µg/m³\n", pm25);
    Pm25Sensor.setPm25Concentration(pm25);
  }

  // Decommission on 5-second BOOT-button press.
  bool pressed = (digitalRead(kButtonPin) == LOW);
  if (pressed && !g_buttonPressed) {
    g_buttonPressedAt = millis();
    g_buttonPressed = true;
  }
  if (!pressed) g_buttonPressed = false;
  if (g_buttonPressed && (millis() - g_buttonPressedAt > kDecommissionTimeout)) {
    Serial.println("Decommissioning device...");
    Matter.decommission();
    g_buttonPressedAt = millis();
  }

  delay(100);
}
