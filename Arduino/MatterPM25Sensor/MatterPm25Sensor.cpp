#include <sdkconfig.h>
#ifdef CONFIG_ESP_MATTER_ENABLE_DATA_MODEL

// Uses matter implementation of ESP32 with at least release 4.0.0-rc1 to allow custom endpoints
#include <Matter.h>
#include "MatterPm25Sensor.h"

using namespace esp_matter;
using namespace esp_matter::endpoint;
using namespace esp_matter::cluster;
using namespace esp_matter::cluster::concentration_measurement::feature;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::detail;

MatterPm25Sensor::MatterPm25Sensor() {}

MatterPm25Sensor::~MatterPm25Sensor() {
  end();
}

bool MatterPm25Sensor::begin(float initPm25Ugm3) {
  // check that the Matter root node has been created  
  ensureMatterNode();
  // check that we start from the root node - have to think about this
  if (getEndPointId() != 0) {
    log_e("Matter Endpoint Id %u is not the root node.", getEndPointId());
    return false;
  }
  // now create the Air Quality Sensor endpoint with particle concentration measurement cluster
  air_quality_sensor::config_t aqConfig;

  pm25_concentration_measurement::config_t pm25Config;
  pm25Config.measurement_medium = chip::to_underlying(MeasurementMediumEnum::kAir);
  pm25Config.feature_flags = concentration_measurement::feature::numeric_measurement::get_id();
  pm25Config.features.numeric_measurement.measured_value = nullable<float>(initPm25Ugm3);
  pm25Config.features.numeric_measurement.min_measured_value = nullable<float>(initPm25Ugm3);
  pm25Config.features.numeric_measurement.max_measured_value = nullable<float>(initPm25Ugm3);
  pm25Config.features.numeric_measurement.measurement_unit = chip::to_underlying(MeasurementUnitEnum::kUgm3);

  // endpoint handles can be used to add/modify clusters
  endpoint_t *endpoint = air_quality_sensor::create(node::get(), &aqConfig, ENDPOINT_FLAG_NONE, (void *)this);
  if (endpoint == nullptr) {
    log_e("Failed to create Air Quality Sensor endpoint.");
    return false;
  }
  // now define a particle concentration cluster at the endpoint
  cluster_t *pm25Cluster = pm25_concentration_measurement::create(endpoint, &pm25Config, CLUSTER_FLAG_SERVER);
  if (pm25Cluster == nullptr) {
    log_e("Failed to add PM2.5 cluster — halting.");
    return false;
  }
  log_i("PM2.5 Concentration Measurement cluster added");

  // register the endpoint to the Matter environment
  if (!registerCreatedEndpoint(endpoint)) {
    log_e("Error in endpoint creation", endpoint);
    return false;
  }

  pm25Ugm3 = initPm25Ugm3;

  log_i("PM2.5 Concentration Measurement Sensor created with endpoint_id %u", getEndPointId());

  started = true;
  return true;
}

void MatterPm25Sensor::end() {
  started = false;
}

bool MatterPm25Sensor::setPm25Concentration(float _pm25Ugm3) {
  if (!started) {
    log_e("Matter PM2.5 Concentration Measurement Sensor device has not begun.");
    return false;
  }

  // avoid processing if there was no change
  if (pm25Ugm3 == _pm25Ugm3) {
    return true;
  }

  esp_matter_attr_val_t pm25Val = esp_matter_invalid(NULL);

  if (!getAttributeVal(Pm25ConcentrationMeasurement::Id, Pm25ConcentrationMeasurement::Attributes::MeasuredValue::Id, &pm25Val)) {
    log_e("Failed to get PM2.5 Concentration Measurement Sensor Attribute.");
    return false;
  }
  if (pm25Val.val.f != _pm25Ugm3) {
    pm25Val.val.f = _pm25Ugm3;
    if (!updateAttributeVal(Pm25ConcentrationMeasurement::Id, Pm25ConcentrationMeasurement::Attributes::MeasuredValue::Id, &pm25Val)) {
      log_e("Failed to update PM2.5 Concentration Measurement Sensor Measurement Attribute.");
      return false;
    }
    // make value gloab
    pm25Ugm3 = _pm25Ugm3;
  }
  log_v("PM2.5 Concentration Measurement Sensor set to %.02f ugPerM3", (float)_rawPm25Concentration);

  return true;
}

#endif  // CONFIG_ESP_MATTER_ENABLE_DATA_MODEL