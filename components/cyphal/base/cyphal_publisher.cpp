#include "cyphal_publisher.h"
#include "../cyphal_component.h"
#include "esphome/core/log.h"

namespace esphome {
namespace cyphal {

static const char* const TAG = "cyphal.publisher";

void CyphalPublisher::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CyphalPublisher subject_id=%u", subject_id_);
}

void CyphalPublisher::loop() {
  if (publish_interval_ms_ == 0) {
    return;  // 手动发布模式
  }

  uint32_t now = millis();
  if (now - last_publish_ms_ >= publish_interval_ms_) {
    last_publish_ms_ = now;
    publish();
  }
}

void CyphalPublisher::publish() {
  if (!parent_) {
    ESP_LOGW(TAG, "Cannot publish: no parent set");
    return;
  }

  uint8_t buffer[256];
  size_t size = 0;

  if (prepare_data(buffer, &size)) {
    publish_raw(buffer, size);
  }
}

bool CyphalPublisher::publish_raw(const uint8_t* data, size_t size) {
  if (!parent_) {
    ESP_LOGW(TAG, "Cannot publish: no parent set");
    return false;
  }

  return parent_->publish_message(subject_id_, data, size, 
                                   static_cast<CanardPriority>(priority_));
}

// CyphalPublishTrigger 实现
CyphalPublishTrigger::CyphalPublishTrigger(CyphalPublisher* parent) : parent_(parent) {
  parent->set_data_source([this](uint8_t* buffer, size_t* size) {
    this->trigger();
    // 触发后由用户在YAML中填充数据
  });
}

}  // namespace cyphal
}  // namespace esphome
