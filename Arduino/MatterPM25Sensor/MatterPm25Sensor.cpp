#include <sdkconfig.h>
#ifdef CONFIG_ESP_MATTER_ENABLE_DATA_MODEL

#include <Matter.h>
#include "MatterPm25Sensor.h"

using namespace esp_matter;
using namespace esp_matter::endpoint;
using namespace esp_matter::cluster;
//using namespace chip::app::Clusters;

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

bool MatterPm25Sensor::attributeChangeCB(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val) {
  bool ret = true;
  if (!started) {
    log_e("Matter PM2.5 Concentration Measurement Sensor device has not begun.");
    return false;
  }

  log_d(
    "PM2.5 Concentration Measurement Sensor Attr update callback: endpoint: %u, cluster: %" PRIu32 ", attribute: %" PRIu32 ", val: %" PRIu32, endpoint_id, cluster_id, attribute_id,
    val->val.u32);
  return ret;
}

MatterPm25Sensor::MatterPm25Sensor() {}

MatterPm25Sensor::~MatterPm25Sensor() {
  end();
}

bool MatterPm25Sensor::begin(int16_t _rawPm25Concentration) {
  //ArduinoMatter::_init();
  node::config_t nodeConfig;
  node_t *node = node::create(&nodeConfig, app_attribute_update_cb, app_identification_cb);
  if (node == nullptr) {
    log_e("Failed to create Matter node");
    return false;
  }

  //ArduinoMatter::_init() would set the NodeCreated lifecycle indication
  //sLifecycle = MatterLifecycle::NodeCreated;

  if (getEndPointId() != 0) {
    log_e("Matter PM2.5 Concentration Sensor with Endpoint Id %u device has already been created.", getEndPointId());
    return false;
  }

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

  // endpoint handles can be used to add/modify clusters
  air_quality_sensor::config_t aqConfig;
  endpoint_t *endpoint = air_quality_sensor::create(node::get(), &aqConfig, ENDPOINT_FLAG_NONE, nullptr);
  if (endpoint == nullptr) {
    log_e("Failed to create Air Quality Sensor endpoint — halting.");
    return false;
  }

  cluster_t *pm25Cluster =
    pm25_concentration_measurement::create(endpoint, &pm25Config, CLUSTER_FLAG_SERVER);
  if (pm25Cluster == nullptr) {
    log_e("Failed to add PM2.5 cluster — halting.");
    return false;
  }
  log_i("PM2.5 Concentration Measurement cluster added");

  rawPm25Concentration = _rawPm25Concentration;
  setEndPointId(endpoint::get_id(endpoint));

  log_i("PM2.5 Concentration Measurement Sensor created with endpoint_id %u", getEndPointId());

  started = true;
  return true;
}

void MatterPm25Sensor::end() {
  started = false;
}

bool MatterPm25Sensor::setRawPm25Concentration(int16_t _rawPm25Concentration) {
  if (!started) {
    log_e("Matter PM2.5 Concentration Measurement Sensor device has not begun.");
    return false;
  }

  // avoid processing if there was no change
  if (rawPm25Concentration == _rawPm25Concentration) {
    return true;
  }

  esp_matter_attr_val_t pm25Val = esp_matter_invalid(NULL);

  if (!getAttributeVal(Pm25ConcentrationMeasurement::Id, Pm25ConcentrationMeasurement::Attributes::MeasuredValue::Id, &pm25Val)) {
    log_e("Failed to get PM2.5 Concentration Measurement Sensor Attribute.");
    return false;
  }
  if (pm25Val.val.i16 != _rawPm25Concentration) {
    pm25Val.val.i16 = _rawPm25Concentration;
    bool ret;
    ret = updateAttributeVal(Pm25ConcentrationMeasurement::Id, Pm25ConcentrationMeasurement::Attributes::MeasuredValue::Id, &pm25Val);
    if (!ret) {
      log_e("Failed to update PM2.5 Concentration Measurement Sensor Measurement Attribute.");
      return false;
    }
    rawPm25Concentration = _rawPm25Concentration;
  }
  log_v("PM2.5 Concentration Measurement Sensor set to %.02f ugPerM3", (float)_rawPm25Concentration);

  return true;
}

#endif  // CONFIG_ESP_MATTER_ENABLE_DATA_MODEL