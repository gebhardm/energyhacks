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
  bool begin(double ugPerM3 = 0.00) {
    return begin(static_cast<int16_t>(ugPerM3));
  }
  // stop processing concentration measurement sensor matter events
  void end();
  // set reported concentration measurement value
  bool setPm25Concentration(double ugPerM3) {
    int16_t rawValue = static_cast<int16_t>(ugPerM3);
    return setRawPm25Concentration(rawValue);
  }
  // return the reported float concentration measurement
  double getPm25Concentration() {
    return (double)rawPm25Concentration;
  }
  // double conversion operator
  void operator=(double ugPerM3) {
    setPm25Concentration(ugPerM3);
  }
  // double conversion operator
  operator double() {
    return (double)getPm25Concentration();
  }

protected:
  void onStackStarted() override {
    esp_matter_attr_val_t val = esp_matter_nullable_float(nullable<float>(rawPm25Concentration));
    lock::ScopedChipStackLock stackLock(portMAX_DELAY);
    if (!updateAttributeVal(Pm25ConcentrationMeasurement::Id, Pm25ConcentrationMeasurement::Attributes::MeasuredValue::Id, &val)) {
      log_e("Failed to apply cached PM2.5 value after Matter.begin().");
    }
  }

private:
  bool started = false;
  int16_t rawPm25Concentration = 0;
  bool setRawPm25Concentration(int16_t _rawPm25Concentration);
  bool begin(int16_t _rawPm25Concentration);
};
#endif /* CONFIG_ESP_MATTER_ENABLE_DATA_MODEL */