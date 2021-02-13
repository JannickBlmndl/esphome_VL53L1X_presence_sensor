#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace vl53l1x_people_counter {

class VL53L1X;

enum DistanceMode { SHORT, MEDIUM, LONG };

class VL53L1XSensor : public sensor::Sensor, public PollingComponent, public i2c::I2CDevice {
 public:
  VL53L1XSensor();

  void setup() override;

  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void update() override;

  void loop() override;

  void set_distance_mode(DistanceMode mode) { distance_mode_ = mode; }
  void set_timing_budget(uint32_t budget) { timing_budget_ = budget; }
  void set_retry_budget(uint8_t budget) { retry_budget_ = budget; }
  void set_optical_center(std::array<uint8_t, 2> centerpoints)) {}
  void set_roi_height(uint8_t height) { roi_height_ = height; }
  void set_roi_width(uint8_t width) { roi_width_ = width; };
  void set_distance_threshold(std::array<uint16_t, 2> thresholds)) {}
  void set_i2c_parent(i2c::I2CComponent* parent);
  void set_i2c_address(uint8_t address);

protected:
  VL53L1X* vl53l1x_{nullptr};
  DistanceMode distance_mode_{DistanceMode::LONG};
  uint32_t timing_budget_{50000};
  uint8_t retry_budget_{5};
  uint8_t retry_count_{0};
  uint8_t roi_height_{5};
  uint8_t roi_width_{5};

  struct MeasuringCycle
  {
    static const unsigned SIZE = 2;

    uint8_t get_index() { return index_; }
    
    float get_result(unsigned i) const { return result_[i]; }
    void set_result(float r) { result_[index_] = r; }

    struct Roi
    {
      uint8_t top_left_x;
      uint8_t top_left_y;
      uint8_t bot_right_x;
      uint8_t bot_right_y;
    };

    const Roi& get_roi() const { return roi_[index_]; }

    void start()
    {
      index_ = 0;
      for (float & i : result_)
      {
        i = 0.0f;
      }
    }


    bool next() // returns false if cycle has been completed
    {
      index_ = (index_ + 1) % SIZE;

      return index_ == 0;
    }

    protected:
    uint8_t index_{0};
    float result_[SIZE] = {};
    Roi roi_[SIZE] = {{0,0,15,7}, {0,8,15,15}};
    
  } measuring_cycle_;

  void initiate_reading_();
  void store_reading_(float r);
};

}  // namespace vl53l1x_people_counter
}  // namespace esphome
