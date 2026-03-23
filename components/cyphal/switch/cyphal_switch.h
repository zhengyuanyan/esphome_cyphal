#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "../cyphal_component.h"
#include <functional>

namespace esphome {
namespace cyphal {
namespace switch_ {

/**
 * DSDL类型枚举 - 支持的开关DSDL类型
 */
enum class DsdlSwitchType {
  BIT,              // uavcan.primitive.scalar.Bit
  NATURAL8,         // uavcan.primitive.scalar.Natural8
  INTEGER8,         // uavcan.primitive.scalar.Integer8
  CUSTOM            // 自定义类型
};

/**
 * Cyphal开关发布器
 * 将本地开关状态发布到Cyphal网络
 */
class CyphalSwitchPublisher : public Component, public esphome::switch_::Switch {
 public:
  CyphalSwitchPublisher() = default;

  void set_subject_id(uint16_t subject_id) { subject_id_ = subject_id; }
  void set_cyphal_parent(CyphalComponent* parent) { parent_ = parent; }
  void set_source_switch(esphome::switch_::Switch* sw) { source_switch_ = sw; }
  void set_dsdl_type(const std::string& dsdl_type);
  void set_dsdl_type_enum(DsdlSwitchType type) { dsdl_type_ = type; }

  void setup() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  void write_state(bool state) override;
  void publish_state(bool state);

 protected:
  CyphalComponent* parent_{nullptr};
  esphome::switch_::Switch* source_switch_{nullptr};
  uint16_t subject_id_{0};
  DsdlSwitchType dsdl_type_{DsdlSwitchType::BIT};
  
  virtual bool serialize_state(bool state, uint8_t* buffer, size_t* size);
};

/**
 * Cyphal开关订阅器
 * 从Cyphal网络接收开关命令
 */
class CyphalSwitchSubscriber : public Component, public esphome::switch_::Switch {
 public:
  CyphalSwitchSubscriber() = default;

  void set_subject_id(uint16_t subject_id) { subject_id_ = subject_id; }
  void set_extent(size_t extent) { extent_ = extent; }
  void set_cyphal_parent(CyphalComponent* parent) { parent_ = parent; }
  void set_dsdl_type(const std::string& dsdl_type);
  void set_dsdl_type_enum(DsdlSwitchType type) { dsdl_type_ = type; }

  void setup() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  void write_state(bool state) override;

 protected:
  CyphalComponent* parent_{nullptr};
  uint16_t subject_id_{0};
  size_t extent_{32};
  DsdlSwitchType dsdl_type_{DsdlSwitchType::BIT};
  
  void handle_message_(const uint8_t* data, size_t size, uint8_t source_node_id);
  virtual bool deserialize_state(const uint8_t* data, size_t size);
};

}  // namespace switch_
}  // namespace cyphal
}  // namespace esphome
