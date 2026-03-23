#pragma once

#include <cstdint>
#include <functional>

namespace esphome {
namespace cyphal {

// Cyphal节点操作模式
enum class CyphalNodeMode {
  NODE,       // 普通节点模式
  GATEWAY     // 网关模式（用于Home Assistant桥接）
};

// 节点健康状态 (uavcan.node.Health)
enum class CyphalHealth : uint8_t {
  NOMINAL = 0,
  ADVISORY = 1,
  CAUTION = 2,
  WARNING = 3
};

// 节点运行模式 (uavcan.node.Mode)
enum class CyphalOperationalMode : uint8_t {
  OPERATIONAL = 0,
  INITIALIZATION = 1,
  MAINTENANCE = 2,
  SOFTWARE_UPDATE = 3
};

// 消息元数据
struct CyphalMessageMetadata {
  uint16_t subject_id;
  uint8_t transfer_id;
  uint8_t priority;
  uint8_t remote_node_id;
};

// 接收回调类型
using CyphalReceiveCallback = std::function<void(const uint8_t*, size_t, uint8_t)>;

// 发布回调类型
using CyphalPublishCallback = std::function<void()>;

}  // namespace cyphal
}  // namespace esphome
