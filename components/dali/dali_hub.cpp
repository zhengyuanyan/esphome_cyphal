#include "dali_hub.h"
#include "esphome/core/log.h"
#include "types/uavcan/primitive/scalar/Natural8_1_0.hpp"
#include "types/uavcan/primitive/scalar/Bit_1_0.hpp"
#include "types/uavcan/primitive/scalar/Real32_1_0.hpp"
#include "types/uavcan/si/sample/luminance/Scalar_1_0.hpp"
#include "nunavut/support/serialization.hpp"

namespace esphome {
namespace dali {

static const char* const TAG = "dali.hub";

void DaliHub::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DALI Hub (tx=%u, rx=%u, timer=%u, active_low=%s)", 
                tx_pin_, rx_pin_, timer_num_, active_low_ ? "true" : "false");
  
  dali_ = new DaliClass(timer_num_);
  dali_->begin(tx_pin_, rx_pin_, active_low_);
  
  // 设置接收回调
  dali_->setCallback([](uint8_t* data, uint8_t bits) {
    ESP_LOGD("dali", "Received %u bits: 0x%02X 0x%02X 0x%02X", bits, data[0], data[1], data[2]);
  });

  // 注册 Cyphal 映射
  if (cyphal_parent_) {
    for (const auto& mapping : mappings_) {
      size_t extent = 64; // 默认较大的 extent
      if (mapping.dsdl_type == "uavcan.primitive.scalar.Natural8") extent = 1;
      else if (mapping.dsdl_type == "uavcan.primitive.scalar.Bit") extent = 1;
      else if (mapping.dsdl_type == "uavcan.primitive.scalar.Real32") extent = 4;
      else if (mapping.dsdl_type == "uavcan.si.sample.luminance.Scalar") extent = 4;

      cyphal_parent_->add_receive_handler(
        mapping.subject_id,
        extent,
        2000000,
        [this, mapping](const void* payload, size_t size, uint8_t source_node_id) {
          uint8_t level = 0;
          nunavut::support::const_bitspan span((const uint8_t*)payload, size * 8);

          if (mapping.dsdl_type == "uavcan.primitive.scalar.Natural8") {
            uavcan::primitive::scalar::Natural8_1_0 msg;
            if (deserialize(msg, span) >= 0) {
              level = msg.value;
            }
          } else if (mapping.dsdl_type == "uavcan.primitive.scalar.Bit") {
            uavcan::primitive::scalar::Bit_1_0 msg;
            if (deserialize(msg, span) >= 0) {
              level = msg.value ? 254 : 0;
            }
          } else if (mapping.dsdl_type == "uavcan.primitive.scalar.Real32") {
            uavcan::primitive::scalar::Real32_1_0 msg;
            if (deserialize(msg, span) >= 0) {
              level = static_cast<uint8_t>(msg.value * 254.0f);
            }
          } else if (mapping.dsdl_type == "uavcan.si.sample.luminance.Scalar") {
            uavcan::si::sample::luminance::Scalar_1_0 msg;
            if (deserialize(msg, span) >= 0) {
              // 假设 DALI 亮度 0-254 对应某种照度/亮度范围，这里简单取值
              level = static_cast<uint8_t>(msg.candela_per_square_meter > 254.0f ? 254 : msg.candela_per_square_meter);
            }
          }

          if (mapping.address_type == 2) {
            this->dali_->sendArcBroadcast(level);
          } else {
            this->dali_->sendArc(mapping.dali_address, level, mapping.address_type);
          }
        }
      );
    }
  }
}

void DaliHub::loop() {
#ifndef DALI_NO_COMMISSIONING
  dali_->commission_tick();
#endif
}

void DaliHub::dump_config() {
  ESP_LOGCONFIG(TAG, "DALI Hub:");
  ESP_LOGCONFIG(TAG, "  TX Pin: %u", tx_pin_);
  ESP_LOGCONFIG(TAG, "  RX Pin: %u", rx_pin_);
  ESP_LOGCONFIG(TAG, "  Timer: %u", timer_num_);
  ESP_LOGCONFIG(TAG, "  Active Low: %s", active_low_ ? "YES" : "NO");
}

void DaliHub::send_arc(uint8_t address, uint8_t level, uint8_t addr_type) {
  if (dali_) {
    dali_->sendArc(address, level, addr_type);
  }
}

void DaliHub::send_cmd(uint8_t address, uint8_t command, uint8_t addr_type) {
  if (dali_) {
    dali_->sendCmd(address, (DaliCmd)command, addr_type);
  }
}

}  // namespace dali
}  // namespace esphome
