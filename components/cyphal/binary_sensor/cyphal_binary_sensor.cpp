#include "cyphal_binary_sensor.h"
#include "../cyphal_component.h"
#include "esphome/core/log.h"

// 标准DSDL类型头文件
#include "types/uavcan/primitive/scalar/Bit_1_0.hpp"
#include "types/uavcan/primitive/scalar/Natural8_1_0.hpp"
#include "types/uavcan/primitive/scalar/Integer8_1_0.hpp"
#include "nunavut/support/serialization.hpp"

namespace esphome {
namespace cyphal {
namespace binary_sensor {

static const char* const TAG = "cyphal.binary_sensor";

// ==================== CyphalBinarySensorPublisher ====================

void CyphalBinarySensorPublisher::set_dsdl_type(const std::string& dsdl_type) {
  if (dsdl_type == "uavcan.primitive.scalar.Bit") {
    dsdl_type_ = DsdlBinarySensorType::BIT;
  } else if (dsdl_type == "uavcan.primitive.scalar.Natural8") {
    dsdl_type_ = DsdlBinarySensorType::NATURAL8;
  } else if (dsdl_type == "uavcan.primitive.scalar.Integer8") {
    dsdl_type_ = DsdlBinarySensorType::INTEGER8;
  } else {
    dsdl_type_ = DsdlBinarySensorType::CUSTOM;
  }
}

void CyphalBinarySensorPublisher::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CyphalBinarySensorPublisher subject_id=%u", subject_id_);
  
  if (source_sensor_) {
    source_sensor_->add_on_state_callback([this](bool state) {
      this->publish_state(state);
    });
    // 如果源传感器已有状态，立即发布一次
    if (source_sensor_->has_state()) {
      this->publish_state(source_sensor_->state);
    }
  }
}

void CyphalBinarySensorPublisher::publish_state(bool state) {
  uint8_t buffer[16];
  size_t size = 0;
  bool success = false;

  if (dsdl_type_ == DsdlBinarySensorType::BIT) {
    uavcan::primitive::scalar::Bit_1_0 msg;
    msg.value = state;
    nunavut::support::bitspan span(buffer, 16 * 8);
    auto result = serialize(msg, span);
    if (result >= 0) {
      size = static_cast<size_t>(result.value());
      success = true;
    }
  } else if (dsdl_type_ == DsdlBinarySensorType::NATURAL8) {
    uavcan::primitive::scalar::Natural8_1_0 msg;
    msg.value = state ? 1 : 0;
    nunavut::support::bitspan span(buffer, 16 * 8);
    auto result = serialize(msg, span);
    if (result >= 0) {
      size = static_cast<size_t>(result.value());
      success = true;
    }
  } else if (dsdl_type_ == DsdlBinarySensorType::INTEGER8) {
    uavcan::primitive::scalar::Integer8_1_0 msg;
    msg.value = state ? 1 : 0;
    nunavut::support::bitspan span(buffer, 16 * 8);
    auto result = serialize(msg, span);
    if (result >= 0) {
      size = static_cast<size_t>(result.value());
      success = true;
    }
  }
  
  if (success) {
    if (parent_) {
      parent_->publish_message(subject_id_, buffer, size, CanardPriorityNominal);
      ESP_LOGD(TAG, "Published binary state: %s to subject %u", state ? "ON" : "OFF", subject_id_);
    }
  } else {
    ESP_LOGW(TAG, "Failed to serialize binary state for subject %u", subject_id_);
  }
  
  esphome::binary_sensor::BinarySensor::publish_state(state);
}

// ==================== CyphalBinarySensorSubscriber ====================

void CyphalBinarySensorSubscriber::set_dsdl_type(const std::string& dsdl_type) {
  if (dsdl_type == "uavcan.primitive.scalar.Bit") {
    dsdl_type_ = DsdlBinarySensorType::BIT;
  } else if (dsdl_type == "uavcan.primitive.scalar.Natural8") {
    dsdl_type_ = DsdlBinarySensorType::NATURAL8;
  } else if (dsdl_type == "uavcan.primitive.scalar.Integer8") {
    dsdl_type_ = DsdlBinarySensorType::INTEGER8;
  } else {
    dsdl_type_ = DsdlBinarySensorType::CUSTOM;
  }
}

void CyphalBinarySensorSubscriber::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CyphalBinarySensorSubscriber subject_id=%u", subject_id_);
  
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

void CyphalBinarySensorSubscriber::handle_message_(const uint8_t* data, size_t size, uint8_t source_node_id) {
  bool state = false;
  bool success = false;

  if (dsdl_type_ == DsdlBinarySensorType::BIT) {
    uavcan::primitive::scalar::Bit_1_0 msg;
    nunavut::support::const_bitspan span(data, size * 8);
    if (deserialize(msg, span) >= 0) {
      state = msg.value;
      success = true;
    }
  } else if (dsdl_type_ == DsdlBinarySensorType::NATURAL8) {
    uavcan::primitive::scalar::Natural8_1_0 msg;
    nunavut::support::const_bitspan span(data, size * 8);
    if (deserialize(msg, span) >= 0) {
      state = msg.value != 0;
      success = true;
    }
  } else if (dsdl_type_ == DsdlBinarySensorType::INTEGER8) {
    uavcan::primitive::scalar::Integer8_1_0 msg;
    nunavut::support::const_bitspan span(data, size * 8);
    if (deserialize(msg, span) >= 0) {
      state = msg.value != 0;
      success = true;
    }
  }

  if (success) {
    ESP_LOGD(TAG, "Received binary state: %s from node %u on subject %u", 
             state ? "ON" : "OFF", source_node_id, subject_id_);
    esphome::binary_sensor::BinarySensor::publish_state(state);
  } else {
    // 降级处理：直接读取第一个字节
    if (size >= 1) {
      state = data[0] != 0;
      esphome::binary_sensor::BinarySensor::publish_state(state);
    }
  }
}

}  // namespace binary_sensor
}  // namespace cyphal
}  // namespace esphome
