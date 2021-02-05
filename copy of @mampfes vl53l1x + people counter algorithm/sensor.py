import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import CONF_ID, UNIT_METER, ICON_ARROW_EXPAND_VERTICAL
from esphome.core import TimePeriod

DEPENDENCIES = ['i2c']

vl53l1x_ns = cg.esphome_ns.namespace('vl53l1x')
VL53L1XSensor = vl53l1x_ns.class_('VL53L1XSensor', sensor.Sensor, cg.PollingComponent,
                                  i2c.I2CDevice)
vl53l1x_distance_mode = vl53l1x_ns.enum('vl53l1x_distance_mode')
vl53l1x_distance_modes = {
    'SHORT': vl53l1x_distance_mode.SHORT,
    'MEDIUM': vl53l1x_distance_mode.MEDIUM,
    'LONG': vl53l1x_distance_mode.LONG,
}

CONF_DISTANCE_MODE = "distance_mode"
CONF_TIMING_BUDGET = 'timing_budget'
CONF_RETRY_BUDGET = 'retry_budget'

CONF_OPTICAL_CENTER = 'optical_center'
CONF_X_OPTICAL_CENTER = 'x_coordinate'
CONF_Y_OPTICAL_CENTER = 'y_coordinate'
CONF_ROI_HEIGHT= 'roi_height'
CONF_ROI_WIDTH = 'roi_width'

CONFIG_SCHEMA = sensor.sensor_schema(UNIT_METER, ICON_ARROW_EXPAND_VERTICAL, 2).extend({
    cv.GenerateID(): cv.declare_id(VL53L1XSensor),
    cv.Optional(CONF_DISTANCE_MODE, default="LONG"): cv.enum(vl53l1x_distance_modes, upper=True),
    cv.Optional(CONF_TIMING_BUDGET, default='50ms'):
        cv.All(cv.positive_time_period_microseconds,
               cv.Range(min=TimePeriod(microseconds=20000),
                        max=TimePeriod(microseconds=1100000))),
    cv.Optional(CONF_RETRY_BUDGET, default=5): cv.int_range(min=0, max=255),
   
   
    cv.Optional(CONF_OPTICAL_CENTER): cv.Schema({
        cv.Required(CONF_X_OPTICAL_CENTER, default=159): cv.int_range(min=0, max=255),
        cv.Required(CONF_Y_OPTICAL_CENTER, default=231)): cv.int_range(min=0, max=255),
    }),

    cv.Optional(CONF_ROI_HEIGHT, default=16): cv.int_range(min=0, max=255),
    cv.Optional(CONF_ROI_WIDTH, default=7): cv.int_range(min=0, max=255),
    
}).extend(cv.polling_component_schema('60s')).extend(i2c.i2c_device_schema(0x29))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    cg.add(var.set_distance_mode(config[CONF_DISTANCE_MODE]))
    cg.add(var.set_timing_budget(config[CONF_TIMING_BUDGET]))
    cg.add(var.set_retry_budget(config[CONF_RETRY_BUDGET]))
    
    optical_center_config = config[CONF_OPTICAL_CENTER]
    cg.add(var.set_optical_center(optical_center_config[CONF_X_OPTICAL_CENTER], optical_center_config[CONF_Y_OPTICAL_CENTER]))

    cg.add(var.set_roi_height(config[CONF_ROI_HEIGHT]))
    cg.add(var.set_roi_width(config[CONF_ROI_WIDTH]))

    yield sensor.register_sensor(var, config)
    yield i2c.register_i2c_device(var, config)
