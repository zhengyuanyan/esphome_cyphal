import os
import shutil
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import canbus
from esphome.const import CONF_ID

AUTO_LOAD = ["canbus"]

cyphal_ns = cg.esphome_ns.namespace("cyphal")
CyphalComponent = cyphal_ns.class_("CyphalComponent", cg.Component)

CONF_CANBUS_ID = "canbus_id"
CONF_NODE_ID = "node_id"
CONF_MTU = "mtu"
CONF_TX_QUEUE_SIZE = "tx_queue_size"
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



SUBSCRIPTION_SCHEMA = cv.Schema({
    cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
    cv.Required(CONF_EXTENT): cv.int_range(min=1),
    cv.Optional(CONF_TIMEOUT_US, default=2000000): cv.uint32_t,
    cv.Required(CONF_DSDL_TYPE): cv.string,
    cv.Required(CONF_ON_RECEIVE): cv.lambda_,
})

PUBLISHER_SCHEMA = cv.Schema({
    cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
    cv.Optional(CONF_PRIORITY, default=4): cv.int_range(min=0, max=7),
    cv.Required(CONF_DSDL_TYPE): cv.string,
    cv.Required(CONF_INTERVAL): cv.positive_time_period_milliseconds,
    cv.Required(CONF_PAYLOAD): cv.lambda_,
})

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(CyphalComponent),
    cv.Required(CONF_CANBUS_ID): cv.use_id(canbus.CanbusComponent),
    cv.Required(CONF_NODE_ID): cv.int_range(min=0, max=127),
    cv.Optional(CONF_MTU, default=8): cv.one_of(8, 64, int=True),
    cv.Optional(CONF_TX_QUEUE_SIZE, default=32): cv.positive_int,
    cv.Optional(CONF_SUBSCRIPTIONS, default=[]): cv.ensure_list(SUBSCRIPTION_SCHEMA),
    cv.Optional(CONF_PUBLISHERS, default=[]): cv.ensure_list(PUBLISHER_SCHEMA),
}).extend(cv.COMPONENT_SCHEMA)

THIS_DIR = os.path.dirname(__file__)
GENERATED_DIR = os.path.join(THIS_DIR, "generated")
LIB_DIR = os.path.join(THIS_DIR, "lib")



            
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    
    can = await cg.get_variable(config[CONF_CANBUS_ID])
    cg.add(var.set_canbus(can))
    cg.add(var.set_node_id(config[CONF_NODE_ID]))
    cg.add(var.set_mtu(config[CONF_MTU]))
    cg.add(var.set_tx_queue_size(config[CONF_TX_QUEUE_SIZE]))
    
    # 添加生成的头文件路径
    cg.add_build_flag(f'-I"{GENERATED_DIR}"')

    # 添加 libcanard 和 o1heap 头文件路径
    cg.add_build_flag(f'-I"{os.path.join(LIB_DIR, "libcanard", "libcanard")}"')
    cg.add_build_flag(f'-I"{os.path.join(LIB_DIR, "libcanard", "lib", "cavl2")}"')
    cg.add_build_flag(f'-I"{os.path.join(LIB_DIR, "o1heap", "o1heap")}"')
    
    # 收集所有 DSDL 类型，生成 include
    dsdl_types = set()
    for sub in config.get(CONF_SUBSCRIPTIONS, []):
        dsdl_types.add(sub[CONF_DSDL_TYPE])
    for pub in config.get(CONF_PUBLISHERS, []):
        dsdl_types.add(pub[CONF_DSDL_TYPE])
    for dt in dsdl_types:
        include_name = dt.replace('.', '_') + ".hpp"
        cg.add_global(cg.RawStatement(f'#include <{include_name}>'))

    # 处理订阅
    for sub in config.get(CONF_SUBSCRIPTIONS, []):
        subject_id = sub[CONF_SUBJECT_ID]
        extent = sub[CONF_EXTENT]
        timeout_us = sub[CONF_TIMEOUT_US]
        dsdl_type = sub[CONF_DSDL_TYPE]
        lambda_str = sub[CONF_ON_RECEIVE]

        func_name = f"handle_cyphal_subject_{subject_id}"
        class_name = dsdl_type.replace('.', '_')

        cg.add(cg.RawStatement(f"""
        static void {func_name}(const void* payload, size_t size, uint8_t remote_node_id) {{
            {class_name} msg;
            if (msg.deserialize(payload, size) == 0) {{
                auto lambda = {lambda_str};
                lambda(msg, remote_node_id);
            }} else {{
                ESP_LOGW("cyphal", "Failed to deserialize subject {subject_id}");
            }}
        }}
        """))

        cg.add(var.add_receive_handler(subject_id, extent, timeout_us, func_name))

    # 处理发布器
    for pub in config.get(CONF_PUBLISHERS, []):
        subject_id = pub[CONF_SUBJECT_ID]
        priority = pub[CONF_PRIORITY]
        interval_ms = pub[CONF_INTERVAL].total_milliseconds
        dsdl_type = pub[CONF_DSDL_TYPE]
        lambda_str = pub[CONF_PAYLOAD]

        class_name = dsdl_type.replace('.', '_')
        func_name = f"publish_cyphal_subject_{subject_id}"
        cg.add(cg.RawStatement(f"""
        static void {func_name}(CyphalComponent* comp) {{
            auto lambda = {lambda_str};
            {class_name} msg = lambda();
            uint8_t buffer[{class_name}::SIZE];
            size_t size = msg.serialize(buffer);
            comp->publish_message({subject_id}, buffer, size, static_cast<CanardPriority>({priority}));
        }}
        """))
        cg.add(var.add_timed_publisher(subject_id, interval_ms, func_name))