#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../cyphal_component.h"

namespace esphome {
namespace cyphal {
namespace binary_sensor {

/**
 * DSDL类型枚举 - 支持的二进制传感器DSDL类型
 */
enum class DsdlBinarySensorType {
  BIT,              // uavcan.primitive.scalar.Bit
  NATURAL8,         // uavcan.primitive.scalar.Natural8
  INTEGER8,         // uavcan.primitive.scalar.Integer8
  CUSTOM            // 自定义类型
};

/**
 * Cyphal二进制传感器发布器
 * 将本地二进制传感器状态发布到Cyphal网络
 */
class CyphalBinarySensorPublisher : public Component, public esphome::binary_sensor::BinarySensor {
 public:
  CyphalBinarySensorPublisher() = default;

  void set_subject_id(uint16_t subject_id) { subject_id_ = subject_id; }
  void set_cyphal_parent(CyphalComponent* parent) { parent_ = parent; }
  void set_source_sensor(esphome::binary_sensor::BinarySensor* sensor) { source_sensor_ = sensor; }
  void set_dsdl_type(const std::string& dsdl_type);
  void set_dsdl_type_enum(DsdlBinarySensorType type) { dsdl_type_ = type; }

  void setup() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  void publish_state(bool state);

 protected:
  CyphalComponent* parent_{nullptr};
  esphome::binary_sensor::BinarySensor* source_sensor_{nullptr};
  uint16_t subject_id_{0};
  DsdlBinarySensorType dsdl_type_{DsdlBinarySensorType::BIT};
};

/**
 * Cyphal二进制传感器订阅器
 * 从Cyphal网络接收二进制状态
 */
class CyphalBinarySensorSubscriber : public Component, public esphome::binary_sensor::BinarySensor {
 public:
  CyphalBinarySensorSubscriber() = default;

  void set_subject_id(uint16_t subject_id) { subject_id_ = subject_id; }
  void set_extent(size_t extent) { extent_ = extent; }
  void set_cyphal_parent(CyphalComponent* parent) { parent_ = parent; }
  void set_dsdl_type(const std::string& dsdl_type);
  void set_dsdl_type_enum(DsdlBinarySensorType type) { dsdl_type_ = type; }

  void setup() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

 protected:
  CyphalComponent* parent_{nullptr};
  uint16_t subject_id_{0};
  size_t extent_{32};
  DsdlBinarySensorType dsdl_type_{DsdlBinarySensorType::BIT};
  
  void handle_message_(const uint8_t* data, size_t size, uint8_t source_node_id);
};

}  // namespace binary_sensor
}  // namespace cyphal
}  // namespace esphome
