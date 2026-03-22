#include "cyphal_component.h"
#include "esphome/core/log.h"
#include <cstring>
#include <stdalign.h>

#ifdef ESP_PLATFORM
#include "esp_timer.h"
#endif

namespace esphome
{
  namespace cyphal
  {

    static const char *const TAG = "cyphal";
    alignas(O1HEAP_ALIGNMENT) static unsigned char heap_mem[16384];

    void *CyphalComponent::allocate(void *user, size_t size)
    {
      auto *comp = static_cast<CyphalComponent *>(user);
      return o1heapAllocate(comp->heap_, size);
    }

    void CyphalComponent::deallocate(void *user, size_t size, void *ptr)
    {
      (void)size;
      auto *comp = static_cast<CyphalComponent *>(user);
      o1heapFree(comp->heap_, ptr);
    }

    int8_t CyphalComponent::tx_handler(void *user, CanardMicrosecond deadline_us, CanardMutableFrame *frame)
    {
      (void)deadline_us;
      auto comp = static_cast<CyphalComponent *>(user);
      return comp->send_frame_(frame);
    }

    int8_t CyphalComponent::send_frame_(CanardMutableFrame *frame)
    {
      if (!canbus_)
        return -1;

      std::vector<uint8_t> data(frame->payload.size);
      memcpy(data.data(), frame->payload.data, frame->payload.size);

      canbus::Error err = canbus_->send_data(frame->extended_can_id, true, false, data);
      return (err == canbus::ERROR_OK) ? 1 : -1;
    }

    CanardMicrosecond CyphalComponent::current_micros()
    {
#ifdef ESP_PLATFORM
      return (CanardMicrosecond)esp_timer_get_time();
#else
      return micros(); // 32 位，可能溢出，但大多数场景够用
#endif
    }

    void CyphalComponent::setup()
    {
      ESP_LOGCONFIG(TAG, "Setup Cyphal node %u", node_id_);

      // 初始化 O1Heap
      heap_ = o1heapInit(heap_mem, sizeof(heap_mem));
      if (!heap_)
      {
        ESP_LOGE(TAG, "Failed to init O1Heap");
        return;
      }

      CanardMemoryResource mem;
      mem.user_reference = this;
      mem.allocate = allocate;
      mem.deallocate = deallocate;

      ins_ = canardInit(mem);
      ins_.node_id = node_id_;

      // 初始化 TX 队列
      tx_queue_ = canardTxInit(tx_queue_size_, mtu_, mem);

      // 注册 CAN 接收回调
      if (canbus_)
      {
        canbus_->add_callback([this](uint32_t can_id, bool extended_id, bool rtr, const std::vector<uint8_t> &data)
                              { this->on_frame_received(can_id, extended_id, rtr, data); });
      }

      // 注册订阅
      for (auto &[subject_id, info] : subscriptions_)
      {
        ESP_LOGCONFIG(TAG, "Registered subscription for subject %u", subject_id);
        canardRxSubscribe(&ins_, CanardTransferKindMessage, subject_id,
                          info.sub.extent, info.sub.transfer_id_timeout_usec, &info.sub);
      }
    }

    void CyphalComponent::loop()
    {
      // 定时发布
      uint32_t now = millis();
      for (auto &[subject_id, pub] : timed_publishers_)
      {
        if (now - pub.last_run_ms >= pub.interval_ms)
        {
          pub.last_run_ms = now;
          if (pub.func)
            pub.func(this);
        }
      }

      // 处理 TX 队列发送
      if (canbus_)
      {
        CanardMicrosecond now_us = current_micros();
        canardTxPoll(&tx_queue_, &ins_, now_us, this, tx_handler, nullptr, nullptr);
      }
    }

    void CyphalComponent::dump_config()
    {
      ESP_LOGCONFIG(TAG, "Cyphal Node ID: %u TX Queue: %zu MTU: %zu", node_id_, tx_queue_size_, mtu_);
      ESP_LOGCONFIG(TAG, "Subscriptions: %zu, Publishers: %zu", subscriptions_.size(), timed_publishers_.size());
    }

    void CyphalComponent::add_receive_handler(uint16_t subject_id, size_t extent, uint32_t timeout_us, ReceiveHandler handler)
    {
      SubscriptionInfo info;
      info.sub.extent = extent;
      info.sub.transfer_id_timeout_usec = timeout_us;
      info.handler = handler;
      subscriptions_[subject_id] = info;
      ESP_LOGCONFIG(TAG, "Added receive handler for subject %u", subject_id);
    }

    void CyphalComponent::add_timed_publisher(uint16_t subject_id, uint32_t interval_ms, PublishFunc func)
    {
      TimedPublisherInfo info;
      info.interval_ms = interval_ms;
      info.func = func;
      timed_publishers_[subject_id] = info;
      ESP_LOGCONFIG(TAG, "Added timed publisher for subject %u interval %ums", subject_id, interval_ms);
    }

    uint8_t CyphalComponent::get_next_transfer_id(uint16_t subject_id)
    {
      auto it = transfer_id_counters_.find(subject_id);
      if (it == transfer_id_counters_.end())
      {
        transfer_id_counters_[subject_id] = 0;
        return 0;
      }
      uint8_t id = it->second;
      it->second = (it->second + 1) & CANARD_TRANSFER_ID_MAX;
      return id;
    }

    bool CyphalComponent::publish_message(uint16_t subject_id, const uint8_t *payload, size_t size,
                                          CanardPriority priority)
    {
      CanardTransferMetadata metadata;
      metadata.priority = priority;
      metadata.transfer_kind = CanardTransferKindMessage;
      metadata.port_id = subject_id;
      metadata.remote_node_id = CANARD_NODE_ID_UNSET;
      metadata.transfer_id = get_next_transfer_id(subject_id);

      CanardPayload canard_payload;
      canard_payload.data = payload;
      canard_payload.size = size;

      CanardMicrosecond now_us = current_micros();
      int32_t result = canardTxPush(&tx_queue_, &ins_, 0, &metadata, canard_payload, now_us, nullptr);
      if (result < 0)
      {
        ESP_LOGW(TAG, "Failed to push subject %u: error %d", subject_id, result);
        return false;
      }
      ESP_LOGD(TAG, "Pushed subject %u with %d frames", subject_id, result);
      return true;
    }

    void CyphalComponent::on_frame_received(uint32_t can_id, bool extended_id, bool rtr, const std::vector<uint8_t> &data)
    {
      if (!extended_id)
        return; // Cyphal 使用扩展 ID，忽略标准 ID 帧

      CanardFrame canard_frame;
      canard_frame.extended_can_id = can_id;
      canard_frame.payload.data = data.data();
      canard_frame.payload.size = data.size();

      CanardRxTransfer transfer;
      CanardRxSubscription *sub = nullptr;
      if (canardRxAccept(&ins_, current_micros(), &canard_frame, 0, &transfer, &sub) > 0)
      {
        handle_transfer_(transfer);
      }
    }

    void CyphalComponent::handle_transfer_(const CanardRxTransfer &transfer)
    {
      auto it = subscriptions_.find(transfer.metadata.port_id);
      if (it != subscriptions_.end() && it->second.handler)
      {
        it->second.handler(transfer.payload.data, transfer.payload.size, transfer.metadata.remote_node_id);
        ESP_LOGD(TAG, "Handled transfer for subject %u", transfer.metadata.port_id);
      }
    }

  } // namespace cyphal
} // namespace esphome