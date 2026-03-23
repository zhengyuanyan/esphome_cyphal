import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_ADDRESS,
    CONF_TYPE,
    CONF_OUTPUT_ID,
)
from . import dali_ns, DaliHub, CONF_DALI_ID

AUTO_LOAD = ["light"]

DaliLight = dali_ns.class_("DaliLight", light.LightOutput, cg.Component)

CONF_ADDRESS_TYPE = "address_type"

ADDR_TYPES = {
    "SHORT": 0,
    "GROUP": 1,
    "BROADCAST": 2,
}

PLATFORM_SCHEMA = light.light_schema(DaliLight, light.LightType.BRIGHTNESS_ONLY).extend(
    {
        cv.Required(CONF_DALI_ID): cv.use_id(DaliHub),
        cv.Required(CONF_ADDRESS): cv.int_range(min=0, max=255),
        cv.Optional(CONF_ADDRESS_TYPE, default="SHORT"): cv.enum(ADDR_TYPES, upper=True),
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = PLATFORM_SCHEMA

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await cg.register_component(var, config)
    await light.register_light(var, config)

    parent = await cg.get_variable(config[CONF_DALI_ID])
    cg.add(var.set_dali_parent(parent))
    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(var.set_address_type(config[CONF_ADDRESS_TYPE]))
