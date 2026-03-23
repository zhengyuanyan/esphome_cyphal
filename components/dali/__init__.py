import os
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import (
    CONF_ID,
    CONF_PIN,
    CONF_TYPE,
    CONF_NAME,
    CONF_INTERVAL,
    CONF_NUMBER,
)
from esphome import pins

# 定义命名空间
dali_ns = cg.esphome_ns.namespace("dali")
DaliHub = dali_ns.class_("DaliHub", cg.Component)
DaliLight = dali_ns.class_("DaliLight", light.LightOutput, cg.Component)

from esphome.components import cyphal

CONF_DALI_ID = "dali_id"
CONF_TX_PIN = "tx_pin"
CONF_RX_PIN = "rx_pin"
CONF_TIMER_NUM = "timer_num"
CONF_ACTIVE_LOW = "active_low"
CONF_DALI_ADDRESS = "address"
CONF_ADDRESS_TYPE = "address_type"
CONF_CYPHAL_ID = "cyphal_id"
CONF_MAPPINGS = "mappings"
CONF_SUBJECT_ID = "subject_id"
CONF_DSDL_TYPE = "dsdl_type"

MAPPING_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=65535),
        cv.Required(CONF_DALI_ADDRESS): cv.int_range(min=0, max=255),
        cv.Optional(CONF_ADDRESS_TYPE, default="SHORT"): cv.enum(
            {"SHORT": 0, "GROUP": 1, "BROADCAST": 2}, upper=True
        ),
        cv.Optional(CONF_DSDL_TYPE, default="uavcan.primitive.scalar.Natural8"): cv.string,
    }
)

# DALI 核心配置
CONFIG_SCHEMA = cv.ensure_list(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DaliHub),
            cv.Required(CONF_TX_PIN): pins.gpio_output_pin_schema,
            cv.Required(CONF_RX_PIN): pins.gpio_input_pin_schema,
            cv.Optional(CONF_TIMER_NUM, default=0): cv.int_range(min=0, max=1), # 支持 0 或 1
            cv.Optional(CONF_ACTIVE_LOW, default=True): cv.boolean,
            cv.Optional(CONF_CYPHAL_ID): cv.use_id(cyphal.CyphalComponent),
            cv.Optional(CONF_MAPPINGS): cv.ensure_list(MAPPING_SCHEMA),
        }
    ).extend(cv.COMPONENT_SCHEMA)
)

THIS_DIR = os.path.dirname(os.path.abspath(__file__))
ARDUINO_DALI_DIR = os.path.join(THIS_DIR, "lib", "arduino-dali", "src")
TIMER_INTERRUPT_DIR = os.path.join(THIS_DIR, "lib", "TimerInterrupt_Generic", "src")
DSDL_SRC_DIR = os.path.join(os.path.dirname(THIS_DIR), "cyphal", "lib", "dsdl", "src")

async def to_code(config):
    for conf in config:
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        
        # 获取引脚编号
        tx_pin = conf[CONF_TX_PIN][CONF_NUMBER]
        rx_pin = conf[CONF_RX_PIN][CONF_NUMBER]
        
        cg.add(var.set_tx_pin(tx_pin))
        cg.add(var.set_rx_pin(rx_pin))
        cg.add(var.set_timer_num(conf[CONF_TIMER_NUM]))
        cg.add(var.set_active_low(conf[CONF_ACTIVE_LOW]))
        
        if CONF_CYPHAL_ID in conf:
            parent = await cg.get_variable(conf[CONF_CYPHAL_ID])
            cg.add(var.set_cyphal_parent(parent))
            
        if CONF_MAPPINGS in conf:
            for mapping in conf[CONF_MAPPINGS]:
                cg.add(var.add_cyphal_mapping(mapping[CONF_SUBJECT_ID], mapping[CONF_DALI_ADDRESS], mapping[CONF_ADDRESS_TYPE], mapping[CONF_DSDL_TYPE]))
    
    # 添加包含路径 (使用正斜杠以兼容所有平台)
    cg.add_build_flag("-I" + THIS_DIR.replace('\\', '/'))
    cg.add_build_flag("-I" + ARDUINO_DALI_DIR.replace('\\', '/'))
    cg.add_build_flag("-I" + TIMER_INTERRUPT_DIR.replace('\\', '/'))
    cg.add_build_flag("-I" + DSDL_SRC_DIR.replace('\\', '/'))
    
    # 编译标志
    cg.add_build_flag("-DDALI_DONT_EXPORT")
    cg.add_build_flag("-DDALI_TIMER=0")
    
    # 动态添加库文件引用，确保跨盘符和跨平台兼容
    # 我们使用相对于组件根目录的路径，这样无论项目在哪个盘符都能正确引用
    cg.add_global(cg.RawStatement('#include "lib/arduino-dali/src/Dali.cpp"'))
    cg.add_global(cg.RawStatement('#include "lib/arduino-dali/src/DaliBus.cpp"'))
