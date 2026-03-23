import os
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import canbus
from esphome.const import CONF_ID

# 自动加载依赖组件
AUTO_LOAD = ["canbus"]

# 自动查找子模块
CODEOWNERS = ["@cyphal"]

cyphal_ns = cg.esphome_ns.namespace("cyphal")
CyphalComponent = cyphal_ns.class_("CyphalComponent", cg.Component)

# 核心配置常量
CONF_CYPHAL_ID = "cyphal_id"
CONF_CANBUS_ID = "canbus_id"
CONF_NODE_ID = "node_id"
CONF_MTU = "mtu"
CONF_TX_QUEUE_SIZE = "tx_queue_size"
CONF_MODE = "mode"
CONF_HEARTBEAT = "heartbeat"
CONF_SUBSCRIPTIONS = "subscriptions"
CONF_PUBLISHERS = "publishers"
CONF_SUBJECT_ID = "subject_id"
CONF_EXTENT = "extent"
CONF_TIMEOUT_US = "timeout_us"
CONF_DSDL_TYPE = "dsdl_type"
CONF_PRIORITY = "priority"
CONF_INTERVAL = "interval"
CONF_ON_RECEIVE = "on_receive"
CONF_PAYLOAD = "payload"
CONF_SENSOR = "sensor"
CONF_SWITCH = "switch"
CONF_LIGHT = "light"
CONF_BINARY_SENSOR = "binary_sensor"
CONF_GATEWAY = "gateway"

# 节点模式
NODE_MODE_NODE = "node"
NODE_MODE_GATEWAY = "gateway"

# 心跳配置
HEARTBEAT_SCHEMA = cv.Schema({
    cv.Optional(CONF_SUBJECT_ID, default=7509): cv.int_range(min=0, max=8191),
    cv.Optional("interval", default="1s"): cv.positive_time_period_milliseconds,
})

# 订阅配置
SUBSCRIPTION_SCHEMA = cv.Schema({
    cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
    cv.Required(CONF_EXTENT): cv.int_range(min=1),
    cv.Optional(CONF_TIMEOUT_US, default=2000000): cv.uint32_t,
    cv.Required(CONF_DSDL_TYPE): cv.string,
    cv.Required(CONF_ON_RECEIVE): cv.lambda_,
})

# 发布配置
PUBLISHER_SCHEMA = cv.Schema({
    cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
    cv.Optional(CONF_PRIORITY, default=4): cv.int_range(min=0, max=7),
    cv.Required(CONF_DSDL_TYPE): cv.string,
    cv.Required(CONF_INTERVAL): cv.positive_time_period_milliseconds,
    cv.Required(CONF_PAYLOAD): cv.lambda_,
})

# 主配置Schema
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(CyphalComponent),
    cv.Required(CONF_CANBUS_ID): cv.use_id(canbus.CanbusComponent),
    cv.Required(CONF_NODE_ID): cv.int_range(min=0, max=127),
    cv.Optional(CONF_MODE, default=NODE_MODE_NODE): cv.one_of(NODE_MODE_NODE, NODE_MODE_GATEWAY, lower=True),
    cv.Optional(CONF_MTU, default=8): cv.one_of(8, 64, int=True),
    cv.Optional(CONF_TX_QUEUE_SIZE, default=32): cv.positive_int,
    cv.Optional(CONF_HEARTBEAT): HEARTBEAT_SCHEMA,
    cv.Optional(CONF_SUBSCRIPTIONS, default=[]): cv.ensure_list(SUBSCRIPTION_SCHEMA),
    cv.Optional(CONF_PUBLISHERS, default=[]): cv.ensure_list(PUBLISHER_SCHEMA),
}).extend(cv.COMPONENT_SCHEMA)

THIS_DIR = os.path.dirname(__file__)
LIB_CANARD_DIR = os.path.join(THIS_DIR, "lib", "libcanard")
CANARD_INC_DIR = os.path.join(LIB_CANARD_DIR, "libcanard")
CAVL2_INC_DIR = os.path.join(LIB_CANARD_DIR, "lib", "cavl2")
O1HEAP_DIR = os.path.join(THIS_DIR, "lib", "o1heap", "o1heap")
DSDL_SRC_DIR = os.path.join(THIS_DIR, "lib", "dsdl", "src")
NUNavUT_SUPPORT_DIR = os.path.join(DSDL_SRC_DIR, "nunavut", "support")
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    can = await cg.get_variable(config[CONF_CANBUS_ID])
    cg.add(var.set_canbus(can))
    cg.add(var.set_node_id(config[CONF_NODE_ID]))
    cg.add(var.set_mtu(config[CONF_MTU]))
    cg.add(var.set_tx_queue_size(config[CONF_TX_QUEUE_SIZE]))

    # 添加头文件路径
    cg.add_build_flag(f'-I"{THIS_DIR}"')
    cg.add_build_flag(f'-I"{CANARD_INC_DIR}"')
    cg.add_build_flag(f'-I"{CAVL2_INC_DIR}"')
    cg.add_build_flag(f'-I"{O1HEAP_DIR}"')
    
    # 强制 C 编译器使用 C99 标准
    cg.add_build_flag("-std=c99")

    if os.path.isdir(DSDL_SRC_DIR):
        cg.add_build_flag(f'-I"{DSDL_SRC_DIR}"')
    if os.path.isdir(NUNavUT_SUPPORT_DIR):
        cg.add_build_flag(f'-I"{NUNavUT_SUPPORT_DIR}"')
        cg.add_global(cg.RawStatement('#include <nunavut/support/serialization.hpp>'))

    cg.add_global(cg.RawStatement('#include <canard.h>'))
    cg.add_global(cg.RawStatement('#include <o1heap.h>'))
    cg.add_global(cg.RawStatement('#include <cstring>'))
    cg.add_global(cg.RawStatement('#include "DSDL_Types.h"'))

    # --- 处理订阅 ---
    for sub in config.get(CONF_SUBSCRIPTIONS, []):
        subject_id = sub[CONF_SUBJECT_ID]
        extent = sub[CONF_EXTENT]
        timeout_us = sub[CONF_TIMEOUT_US]
        dsdl_type = sub[CONF_DSDL_TYPE]
        lambda_str = sub[CONF_ON_RECEIVE]

        func_name = f"handle_cyphal_subject_{subject_id}"

        cg.add_global(cg.RawStatement(f"""
        static void {func_name}(const void* payload, size_t size, uint8_t remote_node_id) {{
            {dsdl_type} msg;
            nunavut::support::const_bitspan span((const uint8_t*)payload, size * 8);
            if (deserialize(msg, span) >= 0) {{
                {lambda_str}
            }}
        }}
        """))

        cg.add(cg.RawStatement(f"{var}->add_receive_handler({subject_id}, {extent}, {timeout_us}, {func_name});"))

    # --- 处理发布器 ---
    for pub in config.get(CONF_PUBLISHERS, []):
        subject_id = pub[CONF_SUBJECT_ID]
        priority = pub[CONF_PRIORITY]
        interval_ms = pub[CONF_INTERVAL].total_milliseconds
        dsdl_type = pub[CONF_DSDL_TYPE]
        lambda_str = pub[CONF_PAYLOAD]

        func_name = f"publish_cyphal_subject_{subject_id}"

        # 提取命名空间以辅助ADL或显式调用
        ns_parts = dsdl_type.split("::")
        type_ns = "::".join(ns_parts[:-1]) if len(ns_parts) > 1 else ""

        cg.add_global(cg.RawStatement(f"""
        static void {func_name}(esphome::cyphal::CyphalComponent* comp) {{
            {dsdl_type} msg;
            memset(&msg, 0, sizeof(msg));
            {{
                {lambda_str}
            }}
            uint8_t buffer[128]; 
            nunavut::support::bitspan span(buffer, sizeof(buffer) * 8);
            auto result = {type_ns}::serialize(msg, span);
            if (result >= 0) {{
               comp->publish_message({subject_id}, buffer, static_cast<size_t>(result.value()), static_cast<CanardPriority>({priority}));
            }}
        }}
        """))
        # 注册定时发布器
        cg.add(cg.RawStatement(f"{var}->add_timed_publisher({subject_id}, {interval_ms}, {func_name});"))