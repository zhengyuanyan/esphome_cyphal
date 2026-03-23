import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_UNIT_OF_MEASUREMENT, CONF_ACCURACY_DECIMALS

AUTO_LOAD = ["sensor"]

from .. import cyphal_ns, CyphalComponent, CONF_CYPHAL_ID, CONF_DSDL_TYPE, CONF_SUBJECT_ID

sensor_ns = cyphal_ns.namespace("sensor")

CyphalSensorPublisher = sensor_ns.class_("CyphalSensorPublisher", cg.Component, sensor.Sensor)
CyphalSensorSubscriber = sensor_ns.class_("CyphalSensorSubscriber", cg.Component, sensor.Sensor)

CONF_PUBLISH_INTERVAL = "publish_interval"
CONF_EXTENT = "extent"
CONF_SOURCE_SENSOR = "source_sensor"
CONF_TYPE = "type"

# 平台Schema - 支持 platform: cyphal 语法
PLATFORM_SCHEMA = cv.Any(
    sensor.sensor_schema(CyphalSensorSubscriber).extend({
        cv.Required(CONF_CYPHAL_ID): cv.use_id(CyphalComponent),
        cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
        cv.Optional(CONF_DSDL_TYPE, default="uavcan.primitive.scalar.Real32"): cv.string,
        cv.Optional(CONF_EXTENT, default=256): cv.positive_int,
        cv.Optional(CONF_TYPE, default="subscribe"): cv.one_of("subscribe", lower=True),
    }).extend(cv.COMPONENT_SCHEMA),
    sensor.sensor_schema(CyphalSensorPublisher).extend({
        cv.Required(CONF_CYPHAL_ID): cv.use_id(CyphalComponent),
        cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
        cv.Optional(CONF_DSDL_TYPE, default="uavcan.primitive.scalar.Real32"): cv.string,
        cv.Optional(CONF_PUBLISH_INTERVAL, default="1s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_SOURCE_SENSOR): cv.use_id(sensor.Sensor),
        cv.Required(CONF_TYPE): cv.one_of("publish", lower=True),
    }).extend(cv.COMPONENT_SCHEMA),
)

# 某些环境下可能需要 CONFIG_SCHEMA
CONFIG_SCHEMA = PLATFORM_SCHEMA

async def to_code(config):
    if config[CONF_TYPE] == "publish":
        var = cg.new_Pvariable(config[CONF_ID])
    else:  # subscribe
        var = cg.new_Pvariable(config[CONF_ID])
    
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    
    cg.add(var.set_subject_id(config[CONF_SUBJECT_ID]))
    cg.add(var.set_dsdl_type(config[CONF_DSDL_TYPE]))
    
    cyphal = await cg.get_variable(config[CONF_CYPHAL_ID])
    cg.add(var.set_cyphal_parent(cyphal))

    if config[CONF_TYPE] == "publish":
        cg.add(var.set_publish_interval(config[CONF_PUBLISH_INTERVAL].total_milliseconds))
        if CONF_SOURCE_SENSOR in config:
            src = await cg.get_variable(config[CONF_SOURCE_SENSOR])
            cg.add(var.set_source_sensor(src))
    else:  # subscribe
        cg.add(var.set_extent(config[CONF_EXTENT]))


