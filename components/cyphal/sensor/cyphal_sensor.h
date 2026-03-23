#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "../cyphal_component.h"
#include <functional>
#include <string>

namespace esphome {
namespace cyphal {
namespace sensor {

/**
 * DSDL类型枚举 - 支持的传感器DSDL类型
 */
enum class DsdlSensorType {
  REAL32,           // uavcan.primitive.scalar.Real32
  INTEGER16,        // uavcan.primitive.scalar.Integer16
  INTEGER32,        // uavcan.primitive.scalar.Integer32
  NATURAL8,         // uavcan.primitive.scalar.Natural8
  NATURAL16,        // uavcan.primitive.scalar.Natural16
  TEMPERATURE,      // uavcan.si.sample.temperature.Scalar
  PRESSURE,         // uavcan.si.sample.pressure.Scalar
  POWER,            // uavcan.si.sample.power.Scalar
  VOLTAGE,          // uavcan.si.sample.voltage.Scalar
  CURRENT,          // uavcan.si.sample.electric_current.Scalar
  ENERGY,           // uavcan.si.sample.energy.Scalar
  LENGTH,           // uavcan.si.sample.length.Scalar
  VELOCITY,         // uavcan.si.sample.velocity.Scalar
  ACCELERATION,     // uavcan.si.sample.acceleration.Scalar
  MASS,             // uavcan.si.sample.mass.Scalar
  FORCE,            // uavcan.si.sample.force.Scalar
  TORQUE,           // uavcan.si.sample.torque.Scalar
  FREQUENCY,        // uavcan.si.sample.frequency.Scalar
  ANGLE,            // uavcan.si.sample.angle.Scalar
  ANGULAR_VELOCITY, // uavcan.si.sample.angular_velocity.Scalar
  ELECTRIC_CHARGE,  // uavcan.si.sample.electric_charge.Scalar
  LUMINANCE,        // uavcan.si.sample.luminance.Scalar
  DURATION,         // uavcan.si.sample.duration.Scalar
  VOLUME,           // uavcan.si.sample.volume.Scalar
  VOLUMETRIC_FLOW_RATE, // uavcan.si.sample.volumetric_flow_rate.Scalar
  MAGNETIC_FIELD_STRENGTH, // uavcan.si.sample.magnetic_field_strength.Scalar
  MAGNETIC_FLUX_DENSITY,   // uavcan.si.sample.magnetic_flux_density.Scalar
  CUSTOM            // 自定义类型
};

/**
 * Cyphal传感器发布器
 * 将本地传感器数据发布到Cyphal网络
 * 支持通过YAML配置任意DSDL类型
 */
class CyphalSensorPublisher : public Component, public esphome::sensor::Sensor {
 public:
  CyphalSensorPublisher() = default;

  // 设置主题ID
  void set_subject_id(uint16_t subject_id) { subject_id_ = subject_id; }
  
  // 设置发布间隔
  void set_publish_interval(uint32_t interval_ms) { publish_interval_ms_ = interval_ms; }
  
  // 设置Cyphal父组件
  void set_cyphal_parent(CyphalComponent* parent) { parent_ = parent; }
  
  // 设置DSDL类型（通过字符串）
  void set_dsdl_type(const std::string& dsdl_type);
  
  // 设置DSDL类型（通过枚举）
  void set_dsdl_type_enum(DsdlSensorType type) { dsdl_type_ = type; }
  
  // 设置源传感器（从哪个传感器读取数据）
  void set_source_sensor(esphome::sensor::Sensor* sensor) { source_sensor_ = sensor; }

  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }
  
  // 发布当前值
  void publish_state(float value);
  
  // 手动触发发布
  void publish();
  
  // 获取当前DSDL类型
  DsdlSensorType get_dsdl_type() const { return dsdl_type_; }
  
  // 获取DSDL类型名称
  std::string get_dsdl_type_name() const;

 protected:
  CyphalComponent* parent_{nullptr};
  esphome::sensor::Sensor* source_sensor_{nullptr};
  uint16_t subject_id_{0};
  uint32_t publish_interval_ms_{1000};
  uint32_t last_publish_ms_{0};
  DsdlSensorType dsdl_type_{DsdlSensorType::REAL32};
  
  bool serialize_value(float value, uint8_t* buffer, size_t* size);
};

/**
 * Cyphal传感器订阅器
 * 从Cyphal网络接收传感器数据
 * 支持通过YAML配置任意DSDL类型
 */
class CyphalSensorSubscriber : public Component, public esphome::sensor::Sensor {
 public:
  CyphalSensorSubscriber() = default;

  // 设置主题ID
  void set_subject_id(uint16_t subject_id) { subject_id_ = subject_id; }
  
  // 设置消息大小
  void set_extent(size_t extent) { extent_ = extent; }
  
  // 设置Cyphal父组件
  void set_cyphal_parent(CyphalComponent* parent) { parent_ = parent; }
  
  // 设置DSDL类型（通过字符串）
  void set_dsdl_type(const std::string& dsdl_type);
  
  // 设置DSDL类型（通过枚举）
  void set_dsdl_type_enum(DsdlSensorType type) { dsdl_type_ = type; }

  void setup() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }
  
  // 获取当前DSDL类型
  DsdlSensorType get_dsdl_type() const { return dsdl_type_; }
  
  // 获取DSDL类型名称
  std::string get_dsdl_type_name() const;

 protected:
  CyphalComponent* parent_{nullptr};
  uint16_t subject_id_{0};
  size_t extent_{256};
  DsdlSensorType dsdl_type_{DsdlSensorType::REAL32};
  
  void handle_message_(const uint8_t* data, size_t size, uint8_t source_node_id);
  bool deserialize_value(const uint8_t* data, size_t size, float* value);
};

// 辅助函数：将DSDL类型字符串转换为枚举
DsdlSensorType dsdl_type_from_string(const std::string& type_str);

// 辅助函数：将枚举转换为DSDL类型字符串
std::string dsdl_type_to_string(DsdlSensorType type);

}  // namespace sensor
}  // namespace cyphal
}  // namespace esphome
