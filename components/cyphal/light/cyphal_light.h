#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_state.h"
#include "../cyphal_component.h"
#include <functional>
#include <string>

namespace esphome {
namespace cyphal {
namespace light {

/**
 * 灯光DSDL类型枚举
 */
enum class DsdlLightType {
  NATURAL8,         // uavcan.primitive.scalar.Natural8 - 亮度 0-255
  NATURAL16,        // uavcan.primitive.scalar.Natural16 - 亮度 0-65535
  HIGH_COLOR,       // reg.udral.physics.optics.HighColor - RGB565
  REAL32,           // uavcan.primitive.scalar.Real32 - 通用浮点
  RGB8,             // uavcan.primitive.array.Natural8[3] - RGB
  RGBW8,            // RGBW 四通道
  RGBWW8,           // RGBWW 五通道 (RGB + 冷暖白)
  COLOR_TEMP,       // 色温控制
  HSV,              // HSV色彩空间
  CUSTOM            // 自定义类型
};

/**
 * Cyphal灯光发布器
 * 将本地灯光状态发布到Cyphal网络
 * 支持通过YAML配置任意DSDL类型
 */
class CyphalLightPublisher : public Component, 
                             public esphome::light::LightOutput,
                             public esphome::light::LightRemoteValuesListener {
 public:
  CyphalLightPublisher() = default;

  void set_subject_id(uint16_t subject_id) { subject_id_ = subject_id; }
  void set_cyphal_parent(CyphalComponent* parent) { parent_ = parent; }
  void set_light_state(esphome::light::LightState* state) { light_state_ = state; }
  
  // 实现 LightRemoteValuesListener 接口
  void on_light_remote_values_update() override {
    this->publish_state(this->light_state_);
  }

  // 设置DSDL类型（通过字符串）
  void set_dsdl_type(const std::string& dsdl_type);
  
  // 设置DSDL类型（通过枚举）
  void set_dsdl_type_enum(DsdlLightType type) { dsdl_type_ = type; }

  void setup() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  esphome::light::LightTraits get_traits() override;
  void write_state(esphome::light::LightState* state) override;
  void publish_state(esphome::light::LightState* state);
  
  // 获取当前DSDL类型
  DsdlLightType get_dsdl_type() const { return dsdl_type_; }
  
  // 获取DSDL类型名称
  std::string get_dsdl_type_name() const;

 protected:
  CyphalComponent* parent_{nullptr};
  esphome::light::LightState* light_state_{nullptr};
  uint16_t subject_id_{0};
  DsdlLightType dsdl_type_{DsdlLightType::NATURAL8};
  
  bool serialize_state(esphome::light::LightState* state, uint8_t* buffer, size_t* size);
};

/**
 * Cyphal灯光订阅器
 * 从Cyphal网络接收灯光控制命令
 * 支持通过YAML配置任意DSDL类型
 */
class CyphalLightSubscriber : public Component, public esphome::light::LightOutput {
 public:
  CyphalLightSubscriber() = default;

  void set_subject_id(uint16_t subject_id) { subject_id_ = subject_id; }
  void set_extent(size_t extent) { extent_ = extent; }
  void set_cyphal_parent(CyphalComponent* parent) { parent_ = parent; }
  void set_light_state(esphome::light::LightState* state) { light_state_ = state; }
  
  // 设置DSDL类型（通过字符串）
  void set_dsdl_type(const std::string& dsdl_type);
  
  // 设置DSDL类型（通过枚举）
  void set_dsdl_type_enum(DsdlLightType type) { dsdl_type_ = type; }

  void setup() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  esphome::light::LightTraits get_traits() override;
  void write_state(esphome::light::LightState* state) override;
  
  // 获取当前DSDL类型
  DsdlLightType get_dsdl_type() const { return dsdl_type_; }
  
  // 获取DSDL类型名称
  std::string get_dsdl_type_name() const;

 protected:
  CyphalComponent* parent_{nullptr};
  esphome::light::LightState* light_state_{nullptr};
  uint16_t subject_id_{0};
  size_t extent_{64};
  DsdlLightType dsdl_type_{DsdlLightType::NATURAL8};
  
  void handle_message_(const uint8_t* data, size_t size, uint8_t source_node_id);
  bool deserialize_state(const uint8_t* data, size_t size, 
                         bool* state, float* brightness, 
                         float* red, float* green, float* blue, 
                         float* color_temp = nullptr);
};

// 辅助函数：将DSDL类型字符串转换为枚举
DsdlLightType dsdl_light_type_from_string(const std::string& type_str);

// 辅助函数：将枚举转换为DSDL类型字符串
std::string dsdl_light_type_to_string(DsdlLightType type);

}  // namespace light
}  // namespace cyphal
}  // namespace esphome
