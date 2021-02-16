#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "vl53l1_error_codes.h"

// imported from vl531lx_class.h
#define VL53L1X_IMPLEMENTATION_VER_MAJOR       1
#define VL53L1X_IMPLEMENTATION_VER_MINOR       0
#define VL53L1X_IMPLEMENTATION_VER_SUB         1
#define VL53L1X_IMPLEMENTATION_VER_REVISION  0000

typedef int8_t VL53L1X_ERROR;

#define SOFT_RESET											0x0000
#define VL53L1_I2C_SLAVE__DEVICE_ADDRESS					0x0001
#define VL53L1_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND        0x0008
#define ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS 		0x0016
#define ALGO__CROSSTALK_COMPENSATION_X_PLANE_GRADIENT_KCPS 	0x0018
#define ALGO__CROSSTALK_COMPENSATION_Y_PLANE_GRADIENT_KCPS 	0x001A
#define ALGO__PART_TO_PART_RANGE_OFFSET_MM					0x001E
#define MM_CONFIG__INNER_OFFSET_MM							0x0020
#define MM_CONFIG__OUTER_OFFSET_MM 							0x0022
#define GPIO_HV_MUX__CTRL									0x0030
#define GPIO__TIO_HV_STATUS       							0x0031
#define SYSTEM__INTERRUPT_CONFIG_GPIO 						0x0046
#define PHASECAL_CONFIG__TIMEOUT_MACROP     				0x004B
#define RANGE_CONFIG__TIMEOUT_MACROP_A_HI   				0x005E
#define RANGE_CONFIG__VCSEL_PERIOD_A        				0x0060
#define RANGE_CONFIG__VCSEL_PERIOD_B						0x0063
#define RANGE_CONFIG__TIMEOUT_MACROP_B_HI  					0x0061
#define RANGE_CONFIG__TIMEOUT_MACROP_B_LO  					0x0062
#define RANGE_CONFIG__SIGMA_THRESH 							0x0064
#define RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS			0x0066
#define RANGE_CONFIG__VALID_PHASE_HIGH      				0x0069
#define VL53L1_SYSTEM__INTERMEASUREMENT_PERIOD				0x006C
#define SYSTEM__THRESH_HIGH 								0x0072
#define SYSTEM__THRESH_LOW 									0x0074
#define SD_CONFIG__WOI_SD0                  				0x0078
#define SD_CONFIG__INITIAL_PHASE_SD0        				0x007A
#define ROI_CONFIG__USER_ROI_CENTRE_SPAD					0x007F
#define ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE		0x0080
#define SYSTEM__SEQUENCE_CONFIG								0x0081
#define VL53L1_SYSTEM__GROUPED_PARAMETER_HOLD 				0x0082
#define SYSTEM__INTERRUPT_CLEAR       						0x0086
#define SYSTEM__MODE_START                 					0x0087
#define VL53L1_RESULT__RANGE_STATUS							0x0089
#define VL53L1_RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0		0x008C
#define RESULT__AMBIENT_COUNT_RATE_MCPS_SD					0x0090
#define VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0				0x0096
#define VL53L1_RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0 	0x0098
#define VL53L1_RESULT__OSC_CALIBRATE_VAL					0x00DE
#define VL53L1_FIRMWARE__SYSTEM_STATUS                      0x00E5
#define VL53L1_IDENTIFICATION__MODEL_ID                     0x010F
#define VL53L1_ROI_CONFIG__MODE_ROI_CENTRE_SPAD				0x013E


#define VL53L1X_DEFAULT_DEVICE_ADDRESS						0x52
//

namespace esphome {
namespace vl53l1x {

enum DistanceMode { SHORT, MEDIUM, LONG };

class VL53L1XSensor : public sensor::Sensor, public PollingComponent, public i2c::I2CDevice {
    public:
    void setup() override;

    void dump_config() override;
    float get_setup_priority() const override { return setup_priority::DATA; }
    void update() override;

    void loop() override;

    // Esphome config SETTERS
    void set_timing_budget(uint32_t budget) { timing_budget_ = budget; }
    void set_distance_mode(DistanceMode mode) { distance_mode_ = mode; }

    void set_retry_budget(uint8_t budget) { retry_budget_ = budget; }

    // VL53L1X booleans and function    
    bool VL53L1X_begin(); //Initialization of sensor
   	bool VL53L1X_check_id(); //Check the ID of the sensor, returns true if ID is correct
    
    int8_t VL53L1X_set_distance_mode_long();
    int8_t VL53L1X_set_distance_mode_short();
    int8_t VL53L1X_set_distance_mode(uint16_t DM);
   
    void VL53L1X_start_ranging(); //Begins taking measurements
   	void VL53L1X_stop_ranging(); //Stops taking measurements
   	bool VL53L1X_check_for_data_ready(); //Checks the to see if data is ready
   	void VL53L1X_set_timing_budget_in_ms(uint16_t timingBudget); //Set the timing budget for a measurement
   	uint16_t VL53L1X_get_timing_budget_in_ms(); //Get the timing budget for a measurement
    void VL53L1X_set_distance_mode(DistanceMode mode);
   	void VL53L1X_set_distance_mode_long(); //Set to 4M range
   	void VL53L1X_set_distance_mode_short(); //Set to 1.3M range
  	
    uint16_t VL53L1X_get_distance(); //Returns distance

    // ROI
    void VL53L1X_set_ROI(uint8_t X, uint8_t Y, uint8_t opticalCenter); //Set the height and width of the ROI(region of interest) in SPADs, lowest possible option is 4. Set optical center based on above table
   	uint16_t VL53L1X_get_ROIX(); //Returns the width of the ROI in SPADs
    uint16_t VL53L1X_get_ROIY();//Returns the height of the ROI in SPADs
   
 protected:
  DistanceMode distance_mode_{DistanceMode::LONG};
  uint32_t timing_budget_{50};
  
/*   TO DO: retries */
//   uint8_t retry_budget_{5};
//   uint8_t retry_count_{0};
};

}  // namespace vl53l1x
}  // namespace esphome
