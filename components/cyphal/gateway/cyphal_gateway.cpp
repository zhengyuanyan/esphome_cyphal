#include "cyphal_gateway.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace cyphal {
namespace gateway {

static const char* const TAG = "cyphal.gateway";

CyphalGateway::CyphalGateway() = default;

void CyphalGateway::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Cyphal Gateway, node_id=%u", gateway_node_id_);
  
  // 注册所有Cyphal订阅
  for (auto& [subject_id, entry] : subscriptions_) {
    if (parent_) {
      parent_->add_receive_handler(
          subject_id,
          entry.extent,
          2000000,
          [](const void* payload, size_t size, uint8_t source_node_id, void* user_data) {
            auto* gw = static_cast<CyphalGateway*>(user_data);
            gw->on_cyphal_message(
                gw->subscriptions_.begin()->first,  // subject_id
                static_cast<const uint8_t*>(payload),
                size,
                source_node_id
            );
          },
          this
      );
    }
  }
  
  ESP_LOGCONFIG(TAG, "Gateway setup complete: %zu HA->Cyphal mappings, %zu Cyphal->HA mappings, %zu subscriptions",
                ha_to_cyphal_map_.size(), cyphal_to_ha_map_.size(), subscriptions_.size());
}

void CyphalGateway::loop() {
  // 定期检查连接状态等
}

void CyphalGateway::dump_config() {
  ESP_LOGCONFIG(TAG, "Cyphal Gateway:");
  ESP_LOGCONFIG(TAG, "  Node ID: %u", gateway_node_id_);
  ESP_LOGCONFIG(TAG, "  HA->Cyphal mappings: %zu", ha_to_cyphal_map_.size());
  for (const auto& [entity_id, subject_id] : ha_to_cyphal_map_) {
    ESP_LOGCONFIG(TAG, "    %s -> %u", entity_id.c_str(), subject_id);
  }
  ESP_LOGCONFIG(TAG, "  Cyphal->HA mappings: %zu", cyphal_to_ha_map_.size());
  for (const auto& [subject_id, entity_id] : cyphal_to_ha_map_) {
    ESP_LOGCONFIG(TAG, "    %u -> %s", subject_id, entity_id.c_str());
  }
}

void CyphalGateway::add_ha_to_cyphal_mapping(const std::string& entity_id, uint16_t subject_id) {
  ha_to_cyphal_map_[entity_id] = subject_id;
  ESP_LOGD(TAG, "Added HA->Cyphal mapping: %s -> %u", entity_id.c_str(), subject_id);
}

void CyphalGateway::add_cyphal_to_ha_mapping(uint16_t subject_id, const std::string& entity_id) {
  cyphal_to_ha_map_[subject_id] = entity_id;
  ESP_LOGD(TAG, "Added Cyphal->HA mapping: %u -> %s", subject_id, entity_id.c_str());
}

void CyphalGateway::subscribe_cyphal_subject(uint16_t subject_id, size_t extent) {
  SubscriptionEntry entry;
  entry.extent = extent;
  subscriptions_[subject_id] = entry;
  ESP_LOGD(TAG, "Subscribed to Cyphal subject %u, extent=%zu", subject_id, extent);
}

void CyphalGateway::on_cyphal_message(uint16_t subject_id, const uint8_t* data, size_t size, uint8_t source_node_id) {
  ESP_LOGD(TAG, "Received Cyphal message on subject %u, size=%zu, from node %u", 
           subject_id, size, source_node_id);
  
  // 查找对应的HA实体
  auto it = cyphal_to_ha_map_.find(subject_id);
  if (it == cyphal_to_ha_map_.end()) {
    ESP_LOGW(TAG, "No HA mapping for subject %u", subject_id);
    return;
  }
  
  const std::string& entity_id = it->second;
  std::string state = parse_message_to_state(subject_id, data, size);
  
  ESP_LOGD(TAG, "Forwarding to HA: %s = %s", entity_id.c_str(), state.c_str());
  
  // 这里需要通过ESPHOME API将状态发送到Home Assistant
  // 实际实现需要依赖ESPHOME的API组件
}

void CyphalGateway::on_ha_state_change(const std::string& entity_id, const std::string& state) {
  ESP_LOGD(TAG, "Received HA state change: %s = %s", entity_id.c_str(), state.c_str());
  
  // 查找对应的Cyphal主题
  auto it = ha_to_cyphal_map_.find(entity_id);
  if (it == ha_to_cyphal_map_.end()) {
    ESP_LOGW(TAG, "No Cyphal mapping for HA entity %s", entity_id.c_str());
    return;
  }
  
  uint16_t subject_id = it->second;
  
  uint8_t buffer[64];
  size_t size = 0;
  
  if (serialize_state_to_message(state, buffer, &size)) {
    if (parent_) {
      parent_->publish_message(subject_id, buffer, size, CanardPriorityNominal);
      ESP_LOGD(TAG, "Forwarding to Cyphal: subject %u, size=%zu", subject_id, size);
    }
  }
}

std::string CyphalGateway::parse_message_to_state(uint16_t subject_id, const uint8_t* data, size_t size) {
  // 根据消息内容解析状态
  // 简化实现：假设数据是字符串或数值
  
  if (size == 0) {
    return "";
  }
  
  // 检查是否是布尔值
  if (size == 1) {
    return data[0] ? "on" : "off";
  }
  
  // 检查是否是浮点数
  if (size == 4) {
    float value;
    memcpy(&value, data, sizeof(float));
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f", value);
    return std::string(buf);
  }
  
  // 尝试作为字符串处理
  return std::string(reinterpret_cast<const char*>(data), size);
}

bool CyphalGateway::serialize_state_to_message(const std::string& state, uint8_t* buffer, size_t* size) {
  // 根据状态内容序列化消息
  
  // 检查是否是布尔状态
  if (state == "on" || state == "true") {
    buffer[0] = 1;
    *size = 1;
    return true;
  }
  if (state == "off" || state == "false") {
    buffer[0] = 0;
    *size = 1;
    return true;
  }
  
  // 尝试解析为数值
  float value = 0;
  if (sscanf(state.c_str(), "%f", &value) == 1) {
    memcpy(buffer, &value, sizeof(float));
    *size = sizeof(float);
    return true;
  }
  
  // 作为字符串处理
  size_t len = std::min(state.size(), static_cast<size_t>(63));
  memcpy(buffer, state.c_str(), len);
  *size = len;
  return true;
}

}  // namespace gateway
}  // namespace cyphal
}  // namespace esphome
