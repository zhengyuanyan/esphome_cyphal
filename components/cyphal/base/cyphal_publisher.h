#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include <functional>

namespace esphome {
namespace cyphal {

// 前向声明
class CyphalComponent;

/**
 * Cyphal发布器基类
 * 用于向Cyphal网络发布消息
 */
class CyphalPublisher : public Component {
 public:
  CyphalPublisher() = default;
  virtual ~CyphalPublisher() = default;

  // 设置主题ID
  void set_subject_id(uint16_t subject_id) { subject_id_ = subject_id; }
  uint16_t get_subject_id() const { return subject_id_; }

  // 设置发布间隔（毫秒）
  void set_publish_interval(uint32_t interval_ms) { publish_interval_ms_ = interval_ms; }

  // 设置优先级 (0-7, 默认4)
  void set_priority(uint8_t priority) { priority_ = priority & 0x07; }

  // 设置Cyphal组件引用
  void set_cyphal_parent(CyphalComponent* parent) { parent_ = parent; }

  // 设置数据源回调（用于获取要发布的数据）
  void set_data_source(std::function<void(uint8_t*, size_t*)> callback) {
    data_source_ = callback;
  }

  // Component接口
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  // 手动触发发布
  void publish();

  // 发布原始数据
  bool publish_raw(const uint8_t* data, size_t size);

 protected:
  CyphalComponent* parent_{nullptr};
  uint16_t subject_id_{0};
  uint32_t publish_interval_ms_{1000};
  uint8_t priority_{4};
  uint32_t last_publish_ms_{0};
  std::function<void(uint8_t*, size_t*)> data_source_;

  // 子类实现：准备要发布的数据
  virtual bool prepare_data(uint8_t* buffer, size_t* size) { 
    if (data_source_) {
      data_source_(buffer, size);
      return true;
    }
    return false; 
  }
};

/**
 * Cyphal定时发布触发器
 */
class CyphalPublishTrigger : public Trigger<> {
 public:
  CyphalPublishTrigger(CyphalPublisher* parent);

 protected:
  CyphalPublisher* parent_;
};

}  // namespace cyphal
}  // namespace esphome
