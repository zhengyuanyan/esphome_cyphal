#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "../cyphal/cyphal_component.h"
#include "Dali.h"
#include <vector>

namespace esphome {
namespace dali {

struct CyphalDaliMapping {
  uint16_t subject_id;
  uint8_t dali_address;
  uint8_t address_type;
  std::string dsdl_type;
};

class DaliHub : public Component {
 public:
  DaliHub() = default;

  void set_tx_pin(uint8_t pin) { tx_pin_ = pin; }
  void set_rx_pin(uint8_t pin) { rx_pin_ = pin; }
  void set_timer_num(uint8_t num) { timer_num_ = num; }
  void set_active_low(bool active_low) { active_low_ = active_low; }
  void set_cyphal_parent(cyphal::CyphalComponent* parent) { cyphal_parent_ = parent; }

  void add_cyphal_mapping(uint16_t subject_id, uint8_t dali_address, uint8_t address_type, const std::string& dsdl_type) {
    mappings_.push_back({subject_id, dali_address, address_type, dsdl_type});
  }

  void setup() override;
  void loop() override;
  void dump_config() override;

  DaliClass& get_dali() { return *dali_; }

  // 发送 DALI 命令的辅助方法
  void send_arc(uint8_t address, uint8_t level, uint8_t addr_type = 0);
  void send_cmd(uint8_t address, uint8_t command, uint8_t addr_type = 0);

 protected:
  uint8_t tx_pin_{0};
  uint8_t rx_pin_{0};
  uint8_t timer_num_{0};
  bool active_low_{true};
  cyphal::CyphalComponent* cyphal_parent_{nullptr};
  
  DaliClass* dali_{nullptr};
  std::vector<CyphalDaliMapping> mappings_;
};

}  // namespace dali
}  // namespace esphome
