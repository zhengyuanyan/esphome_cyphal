import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_ID, CONF_OUTPUT_ID

AUTO_LOAD = ["light"]

from .. import cyphal_ns, CyphalComponent, CONF_CYPHAL_ID, CONF_DSDL_TYPE, CONF_SUBJECT_ID

light_ns = cyphal_ns.namespace("light")

CyphalLightPublisher = light_ns.class_("CyphalLightPublisher", cg.Component, light.LightOutput)
CyphalLightSubscriber = light_ns.class_("CyphalLightSubscriber", cg.Component, light.LightOutput)

CONF_EXTENT = "extent"
CONF_TYPE = "type"

# 平台Schema - 支持 platform: cyphal 语法
PLATFORM_SCHEMA = cv.Any(
    light.light_schema(CyphalLightSubscriber, light.LightType.BRIGHTNESS_ONLY).extend({
        cv.Required(CONF_CYPHAL_ID): cv.use_id(CyphalComponent),
        cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
        cv.Optional(CONF_DSDL_TYPE, default="uavcan.primitive.scalar.Natural8"): cv.string,
        cv.Optional(CONF_EXTENT, default=64): cv.positive_int,
        cv.Optional(CONF_TYPE, default="subscribe"): cv.one_of("subscribe", lower=True),
    }).extend(cv.COMPONENT_SCHEMA),
    light.light_schema(CyphalLightPublisher, light.LightType.BRIGHTNESS_ONLY).extend({
        cv.Required(CONF_CYPHAL_ID): cv.use_id(CyphalComponent),
        cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
        cv.Optional(CONF_DSDL_TYPE, default="uavcan.primitive.scalar.Natural8"): cv.string,
        cv.Required(CONF_TYPE): cv.one_of("publish", lower=True),
    }).extend(cv.COMPONENT_SCHEMA),
)

CONFIG_SCHEMA = PLATFORM_SCHEMA

async def to_code(config):
    if config[CONF_TYPE] == "publish":
        var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    else:  # subscribe
        var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    
    await cg.register_component(var, config)
    await light.register_light(var, config)
    
    cg.add(var.set_subject_id(config[CONF_SUBJECT_ID]))
    cg.add(var.set_dsdl_type(config[CONF_DSDL_TYPE]))
    
    cyphal = await cg.get_variable(config[CONF_CYPHAL_ID])
    cg.add(var.set_cyphal_parent(cyphal))

    if config[CONF_TYPE] == "subscribe":
        cg.add(var.set_extent(config[CONF_EXTENT]))


