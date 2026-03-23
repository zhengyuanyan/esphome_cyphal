#include "cyphal_subscriber.h"
#include "../cyphal_component.h"
#include "esphome/core/log.h"

namespace esphome {
namespace cyphal {

static const char* const TAG = "cyphal.subscriber";

void CyphalSubscriber::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CyphalSubscriber subject_id=%u, extent=%zu", 
                subject_id_, extent_);

  if (parent_) {
    parent_->add_receive_handler(
        subject_id_, 
        extent_, 
        transfer_id_timeout_us_,
        [](const void* payload, size_t size, uint8_t source_node_id, void* user_data) {
          auto* sub = static_cast<CyphalSubscriber*>(user_data);
          sub->handle_message(static_cast<const uint8_t*>(payload), size, source_node_id);
        },
        this
    );
  }
}

void CyphalSubscriber::handle_message(const uint8_t* data, size_t size, uint8_t source_node_id) {
  ESP_LOGD(TAG, "Received message on subject %u, size=%zu, from node %u", 
           subject_id_, size, source_node_id);
  process_data(data, size, source_node_id);
}

// CyphalReceiveTrigger 实现
CyphalReceiveTrigger::CyphalReceiveTrigger(CyphalSubscriber* parent) : parent_(parent) {
  parent->set_receive_callback([this](const uint8_t* data, size_t size, uint8_t source_node_id) {
    std::vector<uint8_t> vec(data, data + size);
    this->trigger(vec, source_node_id);
  });
}

}  // namespace cyphal
}  // namespace esphome
