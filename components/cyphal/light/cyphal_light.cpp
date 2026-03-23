#include "cyphal_light.h"
#include "../cyphal_component.h"
#include "esphome/core/log.h"

// 标准DSDL类型头文件
#include "types/uavcan/primitive/scalar/Natural8_1_0.hpp"
#include "types/reg/udral/physics/optics/HighColor_0_1.hpp"
#include "nunavut/support/serialization.hpp"

namespace esphome {
namespace cyphal {
namespace light {

static const char* const TAG = "cyphal.light";

// ==================== CyphalLightPublisher ====================

void CyphalLightPublisher::set_dsdl_type(const std::string& dsdl_type) {
  if (dsdl_type == "uavcan.primitive.scalar.Natural8") {
    dsdl_type_ = DsdlLightType::NATURAL8;
  } else if (dsdl_type == "uavcan.primitive.scalar.Natural16") {
    dsdl_type_ = DsdlLightType::NATURAL16;
  } else if (dsdl_type == "reg.udral.physics.optics.HighColor") {
    dsdl_type_ = DsdlLightType::HIGH_COLOR;
  } else if (dsdl_type == "uavcan.primitive.scalar.Real32") {
    dsdl_type_ = DsdlLightType::REAL32;
  } else {
    dsdl_type_ = DsdlLightType::CUSTOM;
  }
}

std::string CyphalLightPublisher::get_dsdl_type_name() const {
  switch (dsdl_type_) {
    case DsdlLightType::NATURAL8: return "uavcan.primitive.scalar.Natural8";
    case DsdlLightType::NATURAL16: return "uavcan.primitive.scalar.Natural16";
    case DsdlLightType::HIGH_COLOR: return "reg.udral.physics.optics.HighColor";
    case DsdlLightType::REAL32: return "uavcan.primitive.scalar.Real32";
    default: return "custom";
  }
}

void CyphalLightPublisher::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CyphalLightPublisher subject_id=%u, dsdl_type=%s", 
                subject_id_, get_dsdl_type_name().c_str());
  
  if (light_state_) {
    light_state_->add_remote_values_listener(this);
    // 立即发布初始状态
    this->publish_state(this->light_state_);
  }
}

esphome::light::LightTraits CyphalLightPublisher::get_traits() {
  esphome::light::LightTraits traits;
  if (dsdl_type_ == DsdlLightType::HIGH_COLOR) {
    traits.set_supported_color_modes({esphome::light::ColorMode::RGB});
  } else {
    traits.set_supported_color_modes({esphome::light::ColorMode::BRIGHTNESS});
  }
  return traits;
}

void CyphalLightPublisher::write_state(esphome::light::LightState* state) {
  // ESPHome 核心会调用此方法更新灯光
  // 如果我们是发布器，我们需要把这个状态同步到网络
  publish_state(state);
}

void CyphalLightPublisher::publish_state(esphome::light::LightState* state) {
  if (!state) return;
  
  uint8_t buffer[64];
  size_t size = 0;
  
  if (serialize_state(state, buffer, &size)) {
    if (parent_) {
      parent_->publish_message(subject_id_, buffer, size, CanardPriorityNominal);
      ESP_LOGD(TAG, "Published light state to subject %u", subject_id_);
    }
  }
}

bool CyphalLightPublisher::serialize_state(esphome::light::LightState* state, uint8_t* buffer, size_t* size) {
  nunavut::support::bitspan span(buffer, 64 * 8);
  auto values = state->current_values;
  float brightness = values.get_brightness();
  bool is_on = values.get_state() > 0.5f;

  if (dsdl_type_ == DsdlLightType::NATURAL8) {
    uavcan::primitive::scalar::Natural8_1_0 msg;
    msg.value = is_on ? static_cast<uint8_t>(brightness * 255.0f) : 0;
    auto result = serialize(msg, span);
    if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
  } else if (dsdl_type_ == DsdlLightType::HIGH_COLOR) {
    reg::udral::physics::optics::HighColor_0_1 msg;
    if (is_on) {
      msg.red   = static_cast<uint8_t>(values.get_red() * brightness * 31);
      msg.green = static_cast<uint8_t>(values.get_green() * brightness * 63);
      msg.blue  = static_cast<uint8_t>(values.get_blue() * brightness * 31);
    } else {
      msg.red = 0; msg.green = 0; msg.blue = 0;
    }
    auto result = serialize(msg, span);
    if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
  } else if (dsdl_type_ == DsdlLightType::REAL32) {
    uavcan::primitive::scalar::Real32_1_0 msg;
    msg.value = is_on ? brightness : 0.0f;
    auto result = serialize(msg, span);
    if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
  }

  ESP_LOGW(TAG, "Failed to serialize light state for type %s", get_dsdl_type_name().c_str());
  return false;
}

// ==================== CyphalLightSubscriber ====================

void CyphalLightSubscriber::set_dsdl_type(const std::string& dsdl_type) {
  if (dsdl_type == "uavcan.primitive.scalar.Natural8") {
    dsdl_type_ = DsdlLightType::NATURAL8;
  } else if (dsdl_type == "uavcan.primitive.scalar.Natural16") {
    dsdl_type_ = DsdlLightType::NATURAL16;
  } else if (dsdl_type == "reg.udral.physics.optics.HighColor") {
    dsdl_type_ = DsdlLightType::HIGH_COLOR;
  } else if (dsdl_type == "uavcan.primitive.scalar.Real32") {
    dsdl_type_ = DsdlLightType::REAL32;
  } else {
    dsdl_type_ = DsdlLightType::CUSTOM;
  }
}

std::string CyphalLightSubscriber::get_dsdl_type_name() const {
  switch (dsdl_type_) {
    case DsdlLightType::NATURAL8: return "uavcan.primitive.scalar.Natural8";
    case DsdlLightType::NATURAL16: return "uavcan.primitive.scalar.Natural16";
    case DsdlLightType::HIGH_COLOR: return "reg.udral.physics.optics.HighColor";
    case DsdlLightType::REAL32: return "uavcan.primitive.scalar.Real32";
    default: return "custom";
  }
}

void CyphalLightSubscriber::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CyphalLightSubscriber subject_id=%u, dsdl_type=%s", 
                subject_id_, get_dsdl_type_name().c_str());
  
  if (parent_) {
    parent_->add_receive_handler(
        subject_id_,
        extent_,
        2000000,
        [this](const void* payload, size_t size, uint8_t source_node_id) {
          this->handle_message_(static_cast<const uint8_t*>(payload), size, source_node_id);
        }
    );
  }
}

esphome::light::LightTraits CyphalLightSubscriber::get_traits() {
  esphome::light::LightTraits traits;
  if (dsdl_type_ == DsdlLightType::HIGH_COLOR) {
    traits.set_supported_color_modes({esphome::light::ColorMode::RGB});
  } else {
    traits.set_supported_color_modes({esphome::light::ColorMode::BRIGHTNESS});
  }
  return traits;
}

void CyphalLightSubscriber::write_state(esphome::light::LightState* state) {
  // 当在 ESPHome 中控制灯光时，我们可以选择同步回 Cyphal 网络
  // 这里可以调用序列化和发布逻辑
}

void CyphalLightSubscriber::handle_message_(const uint8_t* data, size_t size, uint8_t source_node_id) {
  if (!light_state_) return;
  
  bool state = false;
  float brightness = 1.0f, red = 1.0f, green = 1.0f, blue = 1.0f, color_temp = 0.0f;
  
  if (deserialize_state(data, size, &state, &brightness, &red, &green, &blue, &color_temp)) {
    auto call = light_state_->make_call();
    call.set_state(state);
    if (dsdl_type_ == DsdlLightType::HIGH_COLOR) {
      call.set_rgb(red, green, blue);
    } else {
      call.set_brightness(brightness);
    }
    call.perform();
    ESP_LOGD(TAG, "Received light state from node %u: on=%d, bri=%.2f", source_node_id, state, brightness);
  }
}

bool CyphalLightSubscriber::deserialize_state(const uint8_t* data, size_t size, 
                                               bool* state, float* brightness, 
                                               float* red, float* green, float* blue,
                                               float* color_temp) {
  nunavut::support::const_bitspan span(data, size * 8);

  if (dsdl_type_ == DsdlLightType::NATURAL8) {
    uavcan::primitive::scalar::Natural8_1_0 msg;
    if (deserialize(msg, span) >= 0) {
      *state = msg.value > 0;
      *brightness = msg.value / 255.0f;
      return true;
    }
  } else if (dsdl_type_ == DsdlLightType::HIGH_COLOR) {
    reg::udral::physics::optics::HighColor_0_1 msg;
    if (deserialize(msg, span) >= 0) {
      *red = msg.red / 31.0f;
      *green = msg.green / 63.0f;
      *blue = msg.blue / 31.0f;
      *state = (*red > 0 || *green > 0 || *blue > 0);
      *brightness = 1.0f; // HighColor 已经包含了亮度
      return true;
    }
  } else if (dsdl_type_ == DsdlLightType::REAL32) {
    uavcan::primitive::scalar::Real32_1_0 msg;
    if (deserialize(msg, span) >= 0) {
      *state = msg.value > 0.001f;
      *brightness = msg.value;
      return true;
    }
  }

  // 降级处理
  if (size >= 1) {
    *state = data[0] > 0;
    *brightness = data[0] / 255.0f;
    return true;
  }
  return false;
}

}  // namespace light
}  // namespace cyphal
}  // namespace esphome
