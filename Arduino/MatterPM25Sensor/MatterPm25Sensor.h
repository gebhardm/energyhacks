#pragma once
#include <sdkconfig.h>
#ifdef CONFIG_ESP_MATTER_ENABLE_DATA_MODEL

#include <Matter.h>
#include <MatterEndPoint.h>

class MatterPm25Sensor : public MatterEndPoint {
public:
  MatterPm25Sensor();
  ~MatterPm25Sensor();
  // this function is called by Matter internal event processor. It could be overwritten by the application, if necessary.
  bool attributeChangeCB(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val) override {
    if (!started) {
      log_e("Air Quality PM2.5 sensor has not begun.");
      return false;
    }
    log_d(
      "Attr update: endpoint %u, cluster 0x%08" PRIX32 ", attribute 0x%08" PRIX32, endpoint_id, cluster_id, attribute_id);
    return true;
  }
  // begin Matter PM2.5 Concentration Measurement Sensor endpoint with initial concentration
  bool begin(float initPm25Ugm3 = 0.0f);
  // stop processing concentration measurement sensor matter events
  void end();
  // set reported concentration measurement value
  bool setPm25Concentration(float);
  // return the reported float concentration measurement
  float getPm25Concentration() const {
    return pm25Ugm3;
  }

protected:
  void onStackStarted() override {
    esp_matter_attr_val_t val = esp_matter_nullable_float(nullable<float>(pm25Ugm3));
    lock::ScopedChipStackLock stackLock(portMAX_DELAY);
    if (!updateAttributeVal(Pm25ConcentrationMeasurement::Id, Pm25ConcentrationMeasurement::Attributes::MeasuredValue::Id, &val)) {
      log_e("Failed to apply cached PM2.5 value after Matter.begin().");
    }
  }

private:
  bool started = false;
  float pm25Ugm3 = 0.0f;
};
#endif /* CONFIG_ESP_MATTER_ENABLE_DATA_MODEL */