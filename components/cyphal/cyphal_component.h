#pragma once

#include "esphome/core/component.h"
#include "esphome/components/canbus/canbus.h"
#include "canard.h"
#include "o1heap.h"

#include <map>
#include <vector>

namespace esphome
{
  namespace cyphal
  {

    class CyphalComponent : public Component
    {
    public:
      void setup() override;
      void loop() override;
      void dump_config() override;

      void set_canbus(canbus::Canbus *can) { canbus_ = can; }
      void set_node_id(uint8_t id) { node_id_ = id; }
      void set_tx_queue_size(size_t size) { tx_queue_size_ = size; }
      void set_mtu(size_t mtu) { mtu_ = mtu; }

      // 订阅处理
      using ReceiveHandler = void (*)(const void *, size_t, uint8_t);
      void add_receive_handler(uint16_t subject_id, size_t extent, uint32_t timeout_us, ReceiveHandler handler);

      // 定时发布
      using PublishFunc = void (*)(CyphalComponent *);
      void add_timed_publisher(uint16_t subject_id, uint32_t interval_ms, PublishFunc func);

      // 发送消息
      bool publish_message(uint16_t subject_id, const uint8_t *payload, size_t size,
                           CanardPriority priority = CanardPriorityNominal);

      // CAN 接收回调
      void on_frame_received(uint32_t can_id, bool extended_id, bool rtr, const std::vector<uint8_t> &data);

    protected:
      canbus::Canbus *canbus_ = nullptr;
      uint8_t node_id_ = CANARD_NODE_ID_UNSET;
      size_t tx_queue_size_ = 32;
      size_t mtu_ = CANARD_MTU_CAN_CLASSIC;

      // libcanard 内存管理
      O1HeapInstance *heap_ = nullptr;

      CanardInstance ins_;
      CanardTxQueue tx_queue_;

      struct SubscriptionInfo
      {
        CanardRxSubscription sub;
        ReceiveHandler handler;
      };
      std::map<uint16_t, SubscriptionInfo> subscriptions_;

      struct TimedPublisherInfo
      {
        uint32_t interval_ms;
        PublishFunc func;
        uint32_t last_run_ms = 0;
      };
      std::map<uint16_t, TimedPublisherInfo> timed_publishers_;

      // 为每个主题维护 transfer_id
      std::map<uint16_t, uint8_t> transfer_id_counters_;

      // 内存回调
      static void *allocate(void *user, size_t size);
      static void deallocate(void *user, size_t size, void *ptr);

      // 发送帧
      static int8_t tx_handler(void *user, CanardMicrosecond deadline_us, CanardMutableFrame *frame);
      int8_t send_frame_(CanardMutableFrame *frame);

      // 处理接收
      void handle_transfer_(const CanardRxTransfer &transfer);

      // 获取微秒时间（64 位）
      static CanardMicrosecond current_micros();

      // 辅助函数
      uint8_t get_next_transfer_id(uint16_t subject_id);
    };

  } // namespace cyphal
} // namespace esphome