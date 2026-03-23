import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID, CONF_NAME

AUTO_LOAD = ["switch"]

from .. import cyphal_ns, CyphalComponent, CONF_CYPHAL_ID, CONF_DSDL_TYPE, CONF_SUBJECT_ID

switch_ns = cyphal_ns.namespace("switch_")

CyphalSwitchPublisher = switch_ns.class_("CyphalSwitchPublisher", cg.Component, switch.Switch)
CyphalSwitchSubscriber = switch_ns.class_("CyphalSwitchSubscriber", cg.Component, switch.Switch)

CONF_EXTENT = "extent"
CONF_SOURCE_SWITCH = "source_switch"
CONF_TYPE = "type"

# 支持的DSDL类型
DSDL_TYPE_BIT = "uavcan.primitive.scalar.Bit"

# 平台Schema - 支持 platform: cyphal 语法
PLATFORM_SCHEMA = cv.Any(
    switch.switch_schema(CyphalSwitchSubscriber).extend({
        cv.Required(CONF_CYPHAL_ID): cv.use_id(CyphalComponent),
        cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
        cv.Optional(CONF_DSDL_TYPE, default=DSDL_TYPE_BIT): cv.string,
        cv.Optional(CONF_EXTENT, default=32): cv.positive_int,
        cv.Optional(CONF_TYPE, default="subscribe"): cv.one_of("subscribe", lower=True),
    }).extend(cv.COMPONENT_SCHEMA),
    switch.switch_schema(CyphalSwitchPublisher).extend({
        cv.Required(CONF_CYPHAL_ID): cv.use_id(CyphalComponent),
        cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
        cv.Optional(CONF_DSDL_TYPE, default=DSDL_TYPE_BIT): cv.string,
        cv.Optional(CONF_SOURCE_SWITCH): cv.use_id(switch.Switch),
        cv.Required(CONF_TYPE): cv.one_of("publish", lower=True),
    }).extend(cv.COMPONENT_SCHEMA),
)

CONFIG_SCHEMA = PLATFORM_SCHEMA

async def to_code(config):
    if config[CONF_TYPE] == "publish":
        var = cg.new_Pvariable(config[CONF_ID])
    else:  # subscribe
        var = cg.new_Pvariable(config[CONF_ID])
    
    await cg.register_component(var, config)
    await switch.register_switch(var, config)
    
    cg.add(var.set_subject_id(config[CONF_SUBJECT_ID]))
    
    cyphal = await cg.get_variable(config[CONF_CYPHAL_ID])
    cg.add(var.set_cyphal_parent(cyphal))

    if config[CONF_TYPE] == "publish":
        if CONF_SOURCE_SWITCH in config:
            src = await cg.get_variable(config[CONF_SOURCE_SWITCH])
            cg.add(var.set_source_switch(src))
    else:  # subscribe
        cg.add(var.set_extent(config[CONF_EXTENT]))


