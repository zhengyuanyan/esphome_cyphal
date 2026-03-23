#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include <functional>
#include <vector>

namespace esphome {
namespace cyphal {

// 前向声明
class CyphalComponent;

/**
 * Cyphal订阅器基类
 * 用于从Cyphal网络接收消息
 */
class CyphalSubscriber : public Component {
 public:
  CyphalSubscriber() = default;
  virtual ~CyphalSubscriber() = default;

  // 设置主题ID
  void set_subject_id(uint16_t subject_id) { subject_id_ = subject_id; }
  uint16_t get_subject_id() const { return subject_id_; }

  // 设置消息大小（用于内存分配）
  void set_extent(size_t extent) { extent_ = extent; }

  // 设置传输ID超时（微秒）
  void set_transfer_id_timeout(uint32_t timeout_us) { transfer_id_timeout_us_ = timeout_us; }

  // 设置Cyphal组件引用
  void set_cyphal_parent(CyphalComponent* parent) { parent_ = parent; }

  // Component接口
  void setup() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

  // 处理接收到的数据（由CyphalComponent调用）
  void handle_message(const uint8_t* data, size_t size, uint8_t source_node_id);

  // 设置接收回调
  void set_receive_callback(std::function<void(const uint8_t*, size_t, uint8_t)> callback) {
    receive_callback_ = callback;
  }

 protected:
  CyphalComponent* parent_{nullptr};
  uint16_t subject_id_{0};
  size_t extent_{256};
  uint32_t transfer_id_timeout_us_{2000000};
  std::function<void(const uint8_t*, size_t, uint8_t)> receive_callback_;

  // 子类实现：处理接收到的数据
  virtual void process_data(const uint8_t* data, size_t size, uint8_t source_node_id) {
    if (receive_callback_) {
      receive_callback_(data, size, source_node_id);
    }
  }
};

/**
 * Cyphal消息接收触发器
 */
class CyphalReceiveTrigger : public Trigger<std::vector<uint8_t>, uint8_t> {
 public:
  CyphalReceiveTrigger(CyphalSubscriber* parent);

 protected:
  CyphalSubscriber* parent_;
};

}  // namespace cyphal
}  // namespace esphome
