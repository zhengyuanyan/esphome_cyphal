#pragma once

#include "esphome/core/component.h"
#include "esphome/components/api/api_connection.h"
#include "../cyphal_component.h"
#include <map>
#include <functional>

namespace esphome {
namespace cyphal {
namespace gateway {

/**
 * Cyphal网关组件
 * 用于Home Assistant与Cyphal网络之间的桥接
 * 
 * 功能：
 * 1. 将Home Assistant的状态变化转发到Cyphal网络
 * 2. 将Cyphal网络的消息转发到Home Assistant
 * 3. 支持节点与节点之间的通信
 */
class CyphalGateway : public Component {
 public:
  CyphalGateway();
  ~CyphalGateway() = default;

  // 设置Cyphal父组件
  void set_cyphal_parent(CyphalComponent* parent) { parent_ = parent; }
  
  // 设置网关节点ID（如果与普通节点不同）
  void set_gateway_node_id(uint8_t node_id) { gateway_node_id_ = node_id; }
  
  // 添加主题映射：HA实体 -> Cyphal主题
  void add_ha_to_cyphal_mapping(const std::string& entity_id, uint16_t subject_id);
  
  // 添加主题映射：Cyphal主题 -> HA实体
  void add_cyphal_to_ha_mapping(uint16_t subject_id, const std::string& entity_id);
  
  // 设置主题订阅
  void subscribe_cyphal_subject(uint16_t subject_id, size_t extent);

  // Component接口
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }
  void dump_config() override;

  // 从Cyphal网络接收消息并转发到HA
  void on_cyphal_message(uint16_t subject_id, const uint8_t* data, size_t size, uint8_t source_node_id);
  
  // 从HA接收状态变化并转发到Cyphal网络
  void on_ha_state_change(const std::string& entity_id, const std::string& state);

 protected:
  CyphalComponent* parent_{nullptr};
  uint8_t gateway_node_id_{0};
  
  // HA实体ID -> Cyphal主题ID映射
  std::map<std::string, uint16_t> ha_to_cyphal_map_;
  
  // Cyphal主题ID -> HA实体ID映射
  std::map<uint16_t, std::string> cyphal_to_ha_map_;
  
  // 订阅的主题及其回调
  struct SubscriptionEntry {
    size_t extent;
    std::vector<uint8_t> last_data;
    uint8_t last_source_node;
  };
  std::map<uint16_t, SubscriptionEntry> subscriptions_;
  
  // 处理接收到的Cyphal消息
  static void handle_cyphal_message_(const void* payload, size_t size, uint8_t source_node_id, void* user_data);
  
  // 解析消息并转换为HA状态
  std::string parse_message_to_state(uint16_t subject_id, const uint8_t* data, size_t size);
  
  // 将HA状态转换为Cyphal消息
  bool serialize_state_to_message(const std::string& state, uint8_t* buffer, size_t* size);
};

/**
 * Cyphal主题桥接配置
 */
struct CyphalSubjectBridge {
  uint16_t subject_id;
  std::string ha_entity_id;
  bool bidirectional;  // 是否双向桥接
  size_t extent;
};

}  // namespace gateway
}  // namespace cyphal
}  // namespace esphome
