import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

AUTO_LOAD = ["api"]

cyphal_ns = cg.esphome_ns.namespace("cyphal")
gateway_ns = cyphal_ns.namespace("gateway")

CyphalGateway = gateway_ns.class_("CyphalGateway", cg.Component)

CONF_CYPHAL_ID = "cyphal_id"
CONF_GATEWAY_NODE_ID = "gateway_node_id"
CONF_HA_TO_CYPHAL = "ha_to_cyphal"
CONF_CYPHAL_TO_HA = "cyphal_to_ha"
CONF_ENTITY_ID = "entity_id"
CONF_SUBJECT_ID = "subject_id"
CONF_BIDIRECTIONAL = "bidirectional"
CONF_EXTENT = "extent"

# 单个映射配置
MAPPING_SCHEMA = cv.Schema({
    cv.Required(CONF_ENTITY_ID): cv.string,
    cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
    cv.Optional(CONF_BIDIRECTIONAL, default=False): cv.boolean,
    cv.Optional(CONF_EXTENT, default=256): cv.positive_int,
})

# 订阅配置
SUBSCRIPTION_SCHEMA = cv.Schema({
    cv.Required(CONF_SUBJECT_ID): cv.int_range(min=0, max=8191),
    cv.Optional(CONF_EXTENT, default=256): cv.positive_int,
})

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(CyphalGateway),
    cv.Required(CONF_CYPHAL_ID): cv.use_id(cg.MockObj),
    cv.Optional(CONF_GATEWAY_NODE_ID): cv.int_range(min=0, max=127),
    cv.Optional(CONF_HA_TO_CYPHAL, default=[]): cv.ensure_list(MAPPING_SCHEMA),
    cv.Optional(CONF_CYPHAL_TO_HA, default=[]): cv.ensure_list(MAPPING_SCHEMA),
})


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    cyphal = await cg.get_variable(config[CONF_CYPHAL_ID])
    cg.add(var.set_cyphal_parent(cyphal))
    
    if CONF_GATEWAY_NODE_ID in config:
        cg.add(var.set_gateway_node_id(config[CONF_GATEWAY_NODE_ID]))
    
    # 处理HA到Cyphal的映射
    for mapping in config.get(CONF_HA_TO_CYPHAL, []):
        cg.add(var.add_ha_to_cyphal_mapping(
            mapping[CONF_ENTITY_ID],
            mapping[CONF_SUBJECT_ID]
        ))
        if mapping[CONF_BIDIRECTIONAL]:
            cg.add(var.add_cyphal_to_ha_mapping(
                mapping[CONF_SUBJECT_ID],
                mapping[CONF_ENTITY_ID]
            ))
            cg.add(var.subscribe_cyphal_subject(
                mapping[CONF_SUBJECT_ID],
                mapping[CONF_EXTENT]
            ))
    
    # 处理Cyphal到HA的映射
    for mapping in config.get(CONF_CYPHAL_TO_HA, []):
        cg.add(var.add_cyphal_to_ha_mapping(
            mapping[CONF_SUBJECT_ID],
            mapping[CONF_ENTITY_ID]
        ))
        cg.add(var.subscribe_cyphal_subject(
            mapping[CONF_SUBJECT_ID],
            mapping[CONF_EXTENT]
        ))
