#include "cyphal_switch.h"
#include "../cyphal_component.h"
#include "esphome/core/log.h"

// 标准DSDL类型头文件
#include "types/uavcan/primitive/scalar/Bit_1_0.hpp"
#include "types/uavcan/primitive/scalar/Natural8_1_0.hpp"
#include "nunavut/support/serialization.hpp"

namespace esphome {
namespace cyphal {
namespace switch_ {

static const char* const TAG = "cyphal.switch";

// ==================== CyphalSwitchPublisher ====================

void CyphalSwitchPublisher::set_dsdl_type(const std::string& dsdl_type) {
  if (dsdl_type == "uavcan.primitive.scalar.Bit") {
    dsdl_type_ = DsdlSwitchType::BIT;
  } else if (dsdl_type == "uavcan.primitive.scalar.Natural8") {
    dsdl_type_ = DsdlSwitchType::NATURAL8;
  } else if (dsdl_type == "uavcan.primitive.scalar.Integer8") {
    dsdl_type_ = DsdlSwitchType::INTEGER8;
  } else {
    dsdl_type_ = DsdlSwitchType::CUSTOM;
  }
}

void CyphalSwitchPublisher::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CyphalSwitchPublisher subject_id=%u", subject_id_);
  
  if (source_switch_) {
    // 订阅源开关的状态变化
    source_switch_->add_on_state_callback([this](bool state) {
      this->publish_state(state);
    });
    // 如果源开关已有状态，立即发布一次
    if (source_switch_->has_state()) {
      this->publish_state(source_switch_->state);
    }
  }
}

void CyphalSwitchPublisher::write_state(bool state) {
  publish_state(state);
  esphome::switch_::Switch::publish_state(state);
}

void CyphalSwitchPublisher::publish_state(bool state) {
  uint8_t buffer[16];
  size_t size = 0;
  
  if (serialize_state(state, buffer, &size)) {
    if (parent_) {
      parent_->publish_message(subject_id_, buffer, size, CanardPriorityNominal);
      ESP_LOGD(TAG, "Published switch state: %s to subject %u", state ? "ON" : "OFF", subject_id_);
    }
  }
}

bool CyphalSwitchPublisher::serialize_state(bool state, uint8_t* buffer, size_t* size) {
  nunavut::support::bitspan span(buffer, 16 * 8);

  if (dsdl_type_ == DsdlSwitchType::BIT) {
    uavcan::primitive::scalar::Bit_1_0 msg;
    msg.value = state;
    auto result = serialize(msg, span);
    if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
  } else if (dsdl_type_ == DsdlSwitchType::NATURAL8) {
    uavcan::primitive::scalar::Natural8_1_0 msg;
    msg.value = state ? 1 : 0;
    auto result = serialize(msg, span);
    if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
  } else if (dsdl_type_ == DsdlSwitchType::INTEGER8) {
    uavcan::primitive::scalar::Integer8_1_0 msg;
    msg.value = state ? 1 : 0;
    auto result = serialize(msg, span);
    if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
  }

  ESP_LOGW(TAG, "Failed to serialize switch state for subject %u", subject_id_);
  return false;
}

// ==================== CyphalSwitchSubscriber ====================

void CyphalSwitchSubscriber::set_dsdl_type(const std::string& dsdl_type) {
  if (dsdl_type == "uavcan.primitive.scalar.Bit") {
    dsdl_type_ = DsdlSwitchType::BIT;
  } else if (dsdl_type == "uavcan.primitive.scalar.Natural8") {
    dsdl_type_ = DsdlSwitchType::NATURAL8;
  } else if (dsdl_type == "uavcan.primitive.scalar.Integer8") {
    dsdl_type_ = DsdlSwitchType::INTEGER8;
  } else {
    dsdl_type_ = DsdlSwitchType::CUSTOM;
  }
}

void CyphalSwitchSubscriber::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CyphalSwitchSubscriber subject_id=%u", subject_id_);
  
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

void CyphalSwitchSubscriber::write_state(bool state) {
  // 当在 ESPHome 中控制开关时，如果是订阅模式，我们通常不发布回网络
  // 但为了同步状态，我们可以选择发布。这里默认只更新本地状态。
  // 如果需要发布回网络，应该使用 Publisher 模式。
  this->publish_state(state);
}

void CyphalSwitchSubscriber::handle_message_(const uint8_t* data, size_t size, uint8_t source_node_id) {
  ESP_LOGD(TAG, "Received switch command on subject %u, size=%zu, from node %u", 
           subject_id_, size, source_node_id);
  
  if (deserialize_state(data, size)) {
    esphome::switch_::Switch::publish_state(true);
  } else {
    esphome::switch_::Switch::publish_state(false);
  }
}

bool CyphalSwitchSubscriber::deserialize_state(const uint8_t* data, size_t size) {
  nunavut::support::const_bitspan span(data, size * 8);

  if (dsdl_type_ == DsdlSwitchType::BIT) {
    uavcan::primitive::scalar::Bit_1_0 msg;
    if (deserialize(msg, span) >= 0) { return msg.value; }
  } else if (dsdl_type_ == DsdlSwitchType::NATURAL8) {
    uavcan::primitive::scalar::Natural8_1_0 msg;
    if (deserialize(msg, span) >= 0) { return msg.value != 0; }
  } else if (dsdl_type_ == DsdlSwitchType::INTEGER8) {
    uavcan::primitive::scalar::Integer8_1_0 msg;
    if (deserialize(msg, span) >= 0) { return msg.value != 0; }
  }

  // 降级处理：直接读取第一个字节
  if (size >= 1) { return data[0] != 0; }
  return false;
}

}  // namespace switch_
}  // namespace cyphal
}  // namespace esphome
