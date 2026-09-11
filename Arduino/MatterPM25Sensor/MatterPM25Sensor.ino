// Copyright 2025 Espressif Systems (Shanghai) PTE LTD
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
 * Hardware: ESP32 with large app partition scheme.
 *
 * Commissioning:
 *   - Manual pairing code.
 *   - Hold BOOT button for 5 s to factory-reset / decommission.
 *
 * readPM25Sensor() takes data from an IKEA Vindrektning PM1006 particle sensor.
 * 
 * Programmed with some help from Claude Code (yes, everybody has to do "something with AI")
 */

#include <Arduino.h>
#include <Matter.h>  // pulls in esp_matter.h → esp_matter_endpoint.h + esp_matter_cluster.h

#if !CONFIG_ENABLE_CHIPOBLE
#include <WiFiManager.h>
#endif

// All required namespaces come from esp_matter.h (included via Matter.h).
using namespace esp_matter;
using namespace esp_matter::endpoint;
using namespace esp_matter::cluster;

constexpr auto k_timeout_seconds = 300;

// Matter cluster / attribute IDs for PM2.5 Concentration Measurement (cluster 0x042A).
// These come from the CHIP data model via <app-common/zap-generated/ids/…>.
static const uint32_t kPM25ClusterId =
  chip::app::Clusters::Pm25ConcentrationMeasurement::Id;
static const uint32_t kMeasuredValueAttrId =
  chip::app::Clusters::Pm25ConcentrationMeasurement::Attributes::MeasuredValue::Id;

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
static uint16_t g_endpointId = 0;
static float g_lastPM25 = -1.0f;
uint8_t cnt = 0;
int pm1006[20];

// ---------------------------------------------------------------------------
// Decommission button (BOOT button)
// ---------------------------------------------------------------------------
static const uint8_t kButtonPin = BOOT_PIN;
static const uint32_t kDecommissionTimeout = 5000;
static bool g_buttonPressed = false;
static uint32_t g_buttonPressedAt = 0;

// ---------------------------------------------------------------------------
// Minimal callbacks required by node::create().
// ArduinoMatter::_init() is protected (friend-class only), so we call
// node::create() directly and own the node pointer ourselves.
// ---------------------------------------------------------------------------
static esp_err_t app_attribute_update_cb(
  attribute::callback_type_t type, uint16_t endpoint_id,
  uint32_t cluster_id, uint32_t attribute_id,
  esp_matter_attr_val_t *val, void *priv_data) {
  return ESP_OK;
}

static esp_err_t app_identification_cb(
  identification::callback_type_t type, uint16_t endpoint_id,
  uint8_t effect_id, uint8_t effect_variant, void *priv_data) {
  return ESP_OK;
}

// This callback is invoked for all Matter events. The application can handle the events as required.
static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg) {
  switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
      log_d(
        "Interface %s Address changed", event->InterfaceIpAddressChanged.Type == chip::DeviceLayer::InterfaceIpChangeType::kIpV4_Assigned ? "IPv4" : "IPV6");
      break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete: log_d("Commissioning complete"); break;
    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired: log_d("Commissioning failed, fail safe timer expired"); break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted: log_d("Commissioning session started"); break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped: log_d("Commissioning session stopped"); break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened: log_d("Commissioning window opened"); break;
    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed: log_d("Commissioning window closed"); break;
    case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
      {
        log_d("Fabric removed successfully");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
          log_d("No fabric left, opening commissioning window");
          chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
          constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
          if (!commissionMgr.IsCommissioningWindowOpen()) {
            // After removing last fabric, it does not remove the Wi-Fi credentials and still has IP connectivity so, only advertising on DNS-SD.
            CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, chip::CommissioningWindowAdvertisement::kDnssdOnly);
            if (err != CHIP_NO_ERROR) {
              log_e("Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
            }
          }
        }
        break;
      }
    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved: log_d("Fabric will be removed"); break;
    case chip::DeviceLayer::DeviceEventType::kFabricUpdated: log_d("Fabric is updated"); break;
    case chip::DeviceLayer::DeviceEventType::kFabricCommitted: log_d("Fabric is committed"); break;
    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized: log_d("BLE deinitialized and memory reclaimed"); break;
    default: break;
  }
}

// ---------------------------------------------------------------------------
// Sensor data taken from what was read via serial connection from PM1006
// ---------------------------------------------------------------------------
static float readPM25Sensor() {
  uint8_t checksum = 0;
  static float value = 0.0f;
  for (uint8_t i = 0; i < 20; i++) { checksum += pm1006[i]; }
  if (checksum == 0) value = (float)((pm1006[5] << 8) | pm1006[6]);
  else log_e("PM25: Sensor checksum invalid.");
  return value;
}

// ---------------------------------------------------------------------------
// Push a new PM2.5 reading into the Matter attribute store
// ---------------------------------------------------------------------------
static bool updatePM25(float ugPerM3) {
  if (g_endpointId == 0) return false;
  if (ugPerM3 == g_lastPM25) return true;

  // esp_matter::attribute::update() is the correct SDK call for reporting
  // attribute changes to subscribed Matter controllers.
  esp_matter_attr_val_t val = esp_matter_nullable_float(ugPerM3);
  esp_err_t err = attribute::update(g_endpointId, kPM25ClusterId, kMeasuredValueAttrId, &val);
  if (err != ESP_OK) {
    log_e("PM25: attribute::update failed (%s)", esp_err_to_name(err));
    return false;
  }

  g_lastPM25 = ugPerM3;
  log_i("PM25: MeasuredValue = %.2f µg/m³", ugPerM3);
  return true;
}

// ---------------------------------------------------------------------------
// Arduino setup
// ---------------------------------------------------------------------------
void setup() {
  pinMode(kButtonPin, INPUT_PULLUP);
  Serial.begin(115200);
  // PM1006 Reading --> RX/TX = 16/17 (actually UART2)
  Serial1.setPins(16, 17);
  Serial1.begin(9600);
  // configure WiFi connection
#if !CONFIG_ENABLE_CHIPOBLE
  WiFiManager wm;
  while (wm.autoConnect("MatterDevice", "password") == false) {
    Serial.print('.');
    delay(500);
  };
  log_i("connected.");
#endif

  // -------------------------------------------------------------------------
  // Build the Matter data model.
  //
  // Node
  // └── Air Quality Sensor endpoint  (device type 0x002C)
  //     ├── Descriptor cluster           (added by air_quality_sensor::create)
  //     ├── Identify cluster             (added by air_quality_sensor::create)
  //     ├── Air Quality cluster          (added by air_quality_sensor::create)
  //     └── PM2.5 Concentration Measurement cluster  (added below)
  // -------------------------------------------------------------------------

  // 1. Create the Matter node (root endpoint 0).
  //    ArduinoMatter::_init() is protected (friend-class only), so we call
  //    node::create() directly. Matter.begin() will call esp_matter::start()
  //    on the same node created here.
  node::config_t nodeConfig;
  node_t *node = node::create(&nodeConfig, app_attribute_update_cb, app_identification_cb);
  if (node == nullptr) {
    log_e("Failed to create Matter node — halting.");
    while (true) delay(1000);
  }

  // 1. Create the Air Quality Sensor endpoint.
  //    air_quality_sensor::config_t contains an air_quality::config_t member;
  //    air_quality = 0 means "Unknown" per the Matter spec.
  air_quality_sensor::config_t aqConfig;
  endpoint_t *ep = air_quality_sensor::create(node, &aqConfig, ENDPOINT_FLAG_NONE, nullptr);
  if (ep == nullptr) {
    log_e("Failed to create Air Quality Sensor endpoint — halting.");
    while (true) delay(1000);
  }
  g_endpointId = endpoint::get_id(ep);
  log_i("Air Quality Sensor endpoint id=%u", g_endpointId);

  // 2. Add the PM2.5 Concentration Measurement cluster.
  //    pm25_concentration_measurement::config_t is a typedef for
  //    concentration_measurement::config_t (see esp_matter_cluster_impl.h).
  //
  //    Enable the NumericMeasurement feature (bit 0) so that the
  //    floating-point MeasuredValue attribute is present and subscribable.
  pm25_concentration_measurement::config_t pm25Config;
  pm25Config.measurement_medium = 0;  // 0 = Air

  // feature_flags: bit 0 = NumericMeasurement (MEA)
  pm25Config.feature_flags =
    concentration_measurement::feature::numeric_measurement::get_id();

  // Initial MeasuredValue: 0.0 µg/m³
  pm25Config.features.numeric_measurement.measured_value = nullable<float>(0.0f);
  pm25Config.features.numeric_measurement.min_measured_value = nullable<float>();
  pm25Config.features.numeric_measurement.max_measured_value = nullable<float>();
  pm25Config.features.numeric_measurement.measurement_unit = 1;  // 1 = µg/m³

  cluster_t *pm25Cluster =
    pm25_concentration_measurement::create(ep, &pm25Config, CLUSTER_FLAG_SERVER);
  if (pm25Cluster == nullptr) {
    log_e("Failed to add PM2.5 cluster — halting.");
    while (true) delay(1000);
  }
  log_i("PM2.5 Concentration Measurement cluster added");

  // 3. Start the Matter stack (must be called after all endpoints are set up).
  // Matter.begin(); // does not work as we have no ArduinoMatter here
  esp_err_t startErr = esp_matter::start(app_event_cb);
  if (startErr != ESP_OK) {
    log_e("esp_matter::start failed (%s) - halting.", esp_err_to_name(startErr));
    while (true) delay(1000);
  }

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
  // read from the particle measurement sensor
  if (Serial1.available() > 0) {
    pm1006[cnt] = Serial1.read();
    //Serial.print(pm1006[cnt], HEX);
    //Serial.print(" ");
    cnt++;
    if (cnt == 20) cnt = 0;
  }
  static uint32_t lastSensorRead = 0;
  if (millis() - lastSensorRead >= 10000UL) {
    lastSensorRead = millis();
    float pm25 = readPM25Sensor();
    //Serial.printf("Sensor reading: %.2f µg/m³\n", pm25);
    log_i("Sensor reading: %.2f µg/m³\n", pm25);
    updatePM25(pm25);
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
