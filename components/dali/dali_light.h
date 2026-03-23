#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/light_output.h"
#include "dali_hub.h"

namespace esphome {
namespace dali {

class DaliLight : public Component, public light::LightOutput {
 public:
  DaliLight() = default;

  void set_dali_parent(DaliHub* parent) { parent_ = parent; }
  void set_address(uint8_t address) { address_ = address; }
  void set_address_type(uint8_t type) { address_type_ = type; }

  light::LightTraits get_traits() override {
    light::LightTraits traits;
    traits.set_supported_color_modes({light::ColorMode::BRIGHTNESS});
    return traits;
  }

  void write_state(light::LightState* state) override {
    auto values = state->current_values;
    float brightness = values.get_brightness();
    bool is_on = values.get_state() > 0.5f;
    
    // DALI 亮度范围 0-254
    uint8_t dali_level = is_on ? static_cast<uint8_t>(brightness * 254.0f) : 0;
    
    if (parent_) {
      if (address_type_ == 2) { // BROADCAST
        parent_->get_dali().sendArcBroadcast(dali_level);
      } else {
        parent_->get_dali().sendArc(address_, dali_level, address_type_);
      }
    }
  }

  void setup() override {}
  void dump_config() override {
    ESP_LOGCONFIG("dali.light", "DALI Light:");
    ESP_LOGCONFIG("dali.light", "  Address: %u", address_);
    ESP_LOGCONFIG("dali.light", "  Type: %u", address_type_);
  }

 protected:
  DaliHub* parent_{nullptr};
  uint8_t address_{0};
  uint8_t address_type_{0}; // 0: SHORT, 1: GROUP, 2: BROADCAST
};

}  // namespace dali
}  // namespace esphome
