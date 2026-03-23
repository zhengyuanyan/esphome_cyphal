#include "cyphal_sensor.h"
#include "../cyphal_component.h"
#include "esphome/core/log.h"

// 标准DSDL类型头文件 - 基本类型
#include "types/uavcan/primitive/scalar/Real32_1_0.hpp"
#include "types/uavcan/primitive/scalar/Integer16_1_0.hpp"
#include "types/uavcan/primitive/scalar/Integer32_1_0.hpp"
#include "types/uavcan/primitive/scalar/Natural8_1_0.hpp"
#include "types/uavcan/primitive/scalar/Natural16_1_0.hpp"

// SI单位类型 - 物理量
#include "types/uavcan/si/sample/temperature/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/pressure/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/power/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/voltage/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/electric_current/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/energy/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/length/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/velocity/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/acceleration/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/mass/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/force/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/torque/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/frequency/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/angle/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/angular_velocity/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/electric_charge/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/luminance/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/duration/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/volume/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/volumetric_flow_rate/Scalar_1_0.hpp"
#include "types/uavcan/si/sample/magnetic_field_strength/Scalar_1_1.hpp"
#include "types/uavcan/si/sample/magnetic_flux_density/Scalar_1_0.hpp"

#include "nunavut/support/serialization.hpp"

namespace esphome {
namespace cyphal {
namespace sensor {

static const char* const TAG = "cyphal.sensor";

// ==================== DSDL类型字符串转换 ====================

DsdlSensorType dsdl_type_from_string(const std::string& type_str) {
  if (type_str == "uavcan.primitive.scalar.Real32") return DsdlSensorType::REAL32;
  if (type_str == "uavcan.primitive.scalar.Integer16") return DsdlSensorType::INTEGER16;
  if (type_str == "uavcan.primitive.scalar.Integer32") return DsdlSensorType::INTEGER32;
  if (type_str == "uavcan.primitive.scalar.Natural8") return DsdlSensorType::NATURAL8;
  if (type_str == "uavcan.primitive.scalar.Natural16") return DsdlSensorType::NATURAL16;
  if (type_str == "uavcan.si.sample.temperature.Scalar") return DsdlSensorType::TEMPERATURE;
  if (type_str == "uavcan.si.sample.pressure.Scalar") return DsdlSensorType::PRESSURE;
  if (type_str == "uavcan.si.sample.power.Scalar") return DsdlSensorType::POWER;
  if (type_str == "uavcan.si.sample.voltage.Scalar") return DsdlSensorType::VOLTAGE;
  if (type_str == "uavcan.si.sample.electric_current.Scalar") return DsdlSensorType::CURRENT;
  if (type_str == "uavcan.si.sample.energy.Scalar") return DsdlSensorType::ENERGY;
  if (type_str == "uavcan.si.sample.length.Scalar") return DsdlSensorType::LENGTH;
  if (type_str == "uavcan.si.sample.velocity.Scalar") return DsdlSensorType::VELOCITY;
  if (type_str == "uavcan.si.sample.acceleration.Scalar") return DsdlSensorType::ACCELERATION;
  if (type_str == "uavcan.si.sample.mass.Scalar") return DsdlSensorType::MASS;
  if (type_str == "uavcan.si.sample.force.Scalar") return DsdlSensorType::FORCE;
  if (type_str == "uavcan.si.sample.torque.Scalar") return DsdlSensorType::TORQUE;
  if (type_str == "uavcan.si.sample.frequency.Scalar") return DsdlSensorType::FREQUENCY;
  if (type_str == "uavcan.si.sample.angle.Scalar") return DsdlSensorType::ANGLE;
  if (type_str == "uavcan.si.sample.angular_velocity.Scalar") return DsdlSensorType::ANGULAR_VELOCITY;
  if (type_str == "uavcan.si.sample.electric_charge.Scalar") return DsdlSensorType::ELECTRIC_CHARGE;
  if (type_str == "uavcan.si.sample.luminance.Scalar") return DsdlSensorType::LUMINANCE;
  if (type_str == "uavcan.si.sample.duration.Scalar") return DsdlSensorType::DURATION;
  if (type_str == "uavcan.si.sample.volume.Scalar") return DsdlSensorType::VOLUME;
  if (type_str == "uavcan.si.sample.volumetric_flow_rate.Scalar") return DsdlSensorType::VOLUMETRIC_FLOW_RATE;
  if (type_str == "uavcan.si.sample.magnetic_field_strength.Scalar") return DsdlSensorType::MAGNETIC_FIELD_STRENGTH;
  if (type_str == "uavcan.si.sample.magnetic_flux_density.Scalar") return DsdlSensorType::MAGNETIC_FLUX_DENSITY;
  return DsdlSensorType::REAL32;  // 默认
}

std::string dsdl_type_to_string(DsdlSensorType type) {
  switch (type) {
    case DsdlSensorType::REAL32: return "uavcan.primitive.scalar.Real32";
    case DsdlSensorType::INTEGER16: return "uavcan.primitive.scalar.Integer16";
    case DsdlSensorType::INTEGER32: return "uavcan.primitive.scalar.Integer32";
    case DsdlSensorType::NATURAL8: return "uavcan.primitive.scalar.Natural8";
    case DsdlSensorType::NATURAL16: return "uavcan.primitive.scalar.Natural16";
    case DsdlSensorType::TEMPERATURE: return "uavcan.si.sample.temperature.Scalar";
    case DsdlSensorType::PRESSURE: return "uavcan.si.sample.pressure.Scalar";
    case DsdlSensorType::POWER: return "uavcan.si.sample.power.Scalar";
    case DsdlSensorType::VOLTAGE: return "uavcan.si.sample.voltage.Scalar";
    case DsdlSensorType::CURRENT: return "uavcan.si.sample.electric_current.Scalar";
    case DsdlSensorType::ENERGY: return "uavcan.si.sample.energy.Scalar";
    case DsdlSensorType::LENGTH: return "uavcan.si.sample.length.Scalar";
    case DsdlSensorType::VELOCITY: return "uavcan.si.sample.velocity.Scalar";
    case DsdlSensorType::ACCELERATION: return "uavcan.si.sample.acceleration.Scalar";
    case DsdlSensorType::MASS: return "uavcan.si.sample.mass.Scalar";
    case DsdlSensorType::FORCE: return "uavcan.si.sample.force.Scalar";
    case DsdlSensorType::TORQUE: return "uavcan.si.sample.torque.Scalar";
    case DsdlSensorType::FREQUENCY: return "uavcan.si.sample.frequency.Scalar";
    case DsdlSensorType::ANGLE: return "uavcan.si.sample.angle.Scalar";
    case DsdlSensorType::ANGULAR_VELOCITY: return "uavcan.si.sample.angular_velocity.Scalar";
    case DsdlSensorType::ELECTRIC_CHARGE: return "uavcan.si.sample.electric_charge.Scalar";
    case DsdlSensorType::LUMINANCE: return "uavcan.si.sample.luminance.Scalar";
    case DsdlSensorType::DURATION: return "uavcan.si.sample.duration.Scalar";
    case DsdlSensorType::VOLUME: return "uavcan.si.sample.volume.Scalar";
    case DsdlSensorType::VOLUMETRIC_FLOW_RATE: return "uavcan.si.sample.volumetric_flow_rate.Scalar";
    case DsdlSensorType::MAGNETIC_FIELD_STRENGTH: return "uavcan.si.sample.magnetic_field_strength.Scalar";
    case DsdlSensorType::MAGNETIC_FLUX_DENSITY: return "uavcan.si.sample.magnetic_flux_density.Scalar";
    default: return "uavcan.primitive.scalar.Real32";
  }
}

// ==================== CyphalSensorPublisher ====================

void CyphalSensorPublisher::set_dsdl_type(const std::string& dsdl_type) {
  dsdl_type_ = dsdl_type_from_string(dsdl_type);
  ESP_LOGD(TAG, "Set DSDL type: %s -> enum %d", dsdl_type.c_str(), static_cast<int>(dsdl_type_));
}

std::string CyphalSensorPublisher::get_dsdl_type_name() const {
  return dsdl_type_to_string(dsdl_type_);
}

void CyphalSensorPublisher::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CyphalSensorPublisher subject_id=%u, dsdl_type=%s", 
                subject_id_, get_dsdl_type_name().c_str());
  
  if (source_sensor_) {
    source_sensor_->add_on_state_callback([this](float value) {
      this->publish_state(value);
    });
    // 如果源传感器已有状态，立即发布一次
    if (source_sensor_->has_state()) {
      this->publish_state(source_sensor_->state);
    }
  }
}

void CyphalSensorSubscriber::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CyphalSensorSubscriber subject_id=%u, dsdl_type=%s", 
                subject_id_, get_dsdl_type_name().c_str());
  
  if (parent_) {
    parent_->add_receive_handler(
        subject_id_,
        extent_,
        2000000,
        [this](const void* payload, size_t size, uint8_t source_node_id) {
          this->handle_message_(static_cast<const uint8_t*>(payload), size, source_node_id);
        }
    );
  }
}

void CyphalSensorPublisher::loop() {
  if (publish_interval_ms_ == 0 || !source_sensor_) {
    return;
  }
  
  uint32_t now = millis();
  if (now - last_publish_ms_ >= publish_interval_ms_) {
    last_publish_ms_ = now;
    publish();
  }
}

void CyphalSensorPublisher::publish_state(float value) {
  uint8_t buffer[64];
  size_t size = 0;
  
  if (serialize_value(value, buffer, &size)) {
    if (parent_) {
      parent_->publish_message(subject_id_, buffer, size, CanardPriorityNominal);
    }
  }
  
  // 更新本地状态
  esphome::sensor::Sensor::publish_state(value);
}

void CyphalSensorPublisher::publish() {
  if (source_sensor_ && source_sensor_->has_state()) {
    publish_state(source_sensor_->state);
  }
}

// ==================== 序列化实现 ====================

bool CyphalSensorPublisher::serialize_value(float value, uint8_t* buffer, size_t* size) {
  nunavut::support::bitspan span(buffer, 64 * 8);
  
  switch (dsdl_type_) {
    case DsdlSensorType::REAL32: {
      uavcan::primitive::scalar::Real32_1_0 msg;
      msg.value = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::INTEGER16: {
      uavcan::primitive::scalar::Integer16_1_0 msg;
      msg.value = static_cast<int16_t>(value);
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::INTEGER32: {
      uavcan::primitive::scalar::Integer32_1_0 msg;
      msg.value = static_cast<int32_t>(value);
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::NATURAL8: {
      uavcan::primitive::scalar::Natural8_1_0 msg;
      msg.value = static_cast<uint8_t>(value);
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::NATURAL16: {
      uavcan::primitive::scalar::Natural16_1_0 msg;
      msg.value = static_cast<uint16_t>(value);
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::TEMPERATURE: {
      uavcan::si::sample::temperature::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.kelvin = value + 273.15f;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::PRESSURE: {
      uavcan::si::sample::pressure::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.pascal = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::POWER: {
      uavcan::si::sample::power::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.watt = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::VOLTAGE: {
      uavcan::si::sample::voltage::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.volt = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::CURRENT: {
      uavcan::si::sample::electric_current::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.ampere = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::ENERGY: {
      uavcan::si::sample::energy::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.joule = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::LENGTH: {
      uavcan::si::sample::length::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.meter = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::VELOCITY: {
      uavcan::si::sample::velocity::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.meter_per_second = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::ACCELERATION: {
      uavcan::si::sample::acceleration::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.meter_per_second_per_second = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::MASS: {
      uavcan::si::sample::mass::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.kilogram = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::FORCE: {
      uavcan::si::sample::force::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.newton = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::TORQUE: {
      uavcan::si::sample::torque::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.newton_meter = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::FREQUENCY: {
      uavcan::si::sample::frequency::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.hertz = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::ANGLE: {
      uavcan::si::sample::angle::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.radian = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::ANGULAR_VELOCITY: {
      uavcan::si::sample::angular_velocity::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.radian_per_second = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::ELECTRIC_CHARGE: {
      uavcan::si::sample::electric_charge::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.coulomb = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::LUMINANCE: {
      uavcan::si::sample::luminance::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.candela_per_square_meter = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::DURATION: {
      uavcan::si::sample::duration::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.second = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::VOLUME: {
      uavcan::si::sample::volume::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.cubic_meter = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::VOLUMETRIC_FLOW_RATE: {
      uavcan::si::sample::volumetric_flow_rate::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.cubic_meter_per_second = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::MAGNETIC_FIELD_STRENGTH: {
      uavcan::si::sample::magnetic_field_strength::Scalar_1_1 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.ampere_per_meter = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    case DsdlSensorType::MAGNETIC_FLUX_DENSITY: {
      uavcan::si::sample::magnetic_flux_density::Scalar_1_0 msg;
      msg.timestamp.microsecond = static_cast<uint64_t>(millis()) * 1000ULL;
      msg.tesla = value;
      auto result = serialize(msg, span);
      if (result >= 0) { *size = static_cast<size_t>(result.value()); return true; }
      break;
    }
    default:
      break;
  }
  
  ESP_LOGW(TAG, "Failed to serialize value for type %s", get_dsdl_type_name().c_str());
  return false;
}

// ==================== CyphalSensorSubscriber ====================

void CyphalSensorSubscriber::set_dsdl_type(const std::string& dsdl_type) {
  dsdl_type_ = dsdl_type_from_string(dsdl_type);
}

std::string CyphalSensorSubscriber::get_dsdl_type_name() const {
  return dsdl_type_to_string(dsdl_type_);
}

void CyphalSensorSubscriber::handle_message_(const uint8_t* data, size_t size, uint8_t source_node_id) {
  float value = NAN;
  if (deserialize_value(data, size, &value)) {
    ESP_LOGD(TAG, "Received value: %.2f from node %u on subject %u", 
             value, source_node_id, subject_id_);
    esphome::sensor::Sensor::publish_state(value);
  }
}

// ==================== 反序列化实现 ====================

bool CyphalSensorSubscriber::deserialize_value(const uint8_t* data, size_t size, float* value) {
  nunavut::support::const_bitspan span(data, size * 8);
  
  switch (dsdl_type_) {
    case DsdlSensorType::REAL32: {
      uavcan::primitive::scalar::Real32_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.value; return true; }
      break;
    }
    case DsdlSensorType::INTEGER16: {
      uavcan::primitive::scalar::Integer16_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = static_cast<float>(msg.value); return true; }
      break;
    }
    case DsdlSensorType::INTEGER32: {
      uavcan::primitive::scalar::Integer32_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = static_cast<float>(msg.value); return true; }
      break;
    }
    case DsdlSensorType::NATURAL8: {
      uavcan::primitive::scalar::Natural8_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = static_cast<float>(msg.value); return true; }
      break;
    }
    case DsdlSensorType::NATURAL16: {
      uavcan::primitive::scalar::Natural16_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = static_cast<float>(msg.value); return true; }
      break;
    }
    case DsdlSensorType::TEMPERATURE: {
      uavcan::si::sample::temperature::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.kelvin - 273.15f; return true; } // 转换为摄氏度
      break;
    }
    case DsdlSensorType::PRESSURE: {
      uavcan::si::sample::pressure::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.pascal; return true; }
      break;
    }
    case DsdlSensorType::POWER: {
      uavcan::si::sample::power::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.watt; return true; }
      break;
    }
    case DsdlSensorType::VOLTAGE: {
      uavcan::si::sample::voltage::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.volt; return true; }
      break;
    }
    case DsdlSensorType::CURRENT: {
      uavcan::si::sample::electric_current::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.ampere; return true; }
      break;
    }
    case DsdlSensorType::ENERGY: {
      uavcan::si::sample::energy::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.joule; return true; }
      break;
    }
    case DsdlSensorType::LENGTH: {
      uavcan::si::sample::length::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.meter; return true; }
      break;
    }
    case DsdlSensorType::VELOCITY: {
      uavcan::si::sample::velocity::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.meter_per_second; return true; }
      break;
    }
    case DsdlSensorType::ACCELERATION: {
      uavcan::si::sample::acceleration::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.meter_per_second_per_second; return true; }
      break;
    }
    case DsdlSensorType::MASS: {
      uavcan::si::sample::mass::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.kilogram; return true; }
      break;
    }
    case DsdlSensorType::FORCE: {
      uavcan::si::sample::force::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.newton; return true; }
      break;
    }
    case DsdlSensorType::TORQUE: {
      uavcan::si::sample::torque::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.newton_meter; return true; }
      break;
    }
    case DsdlSensorType::FREQUENCY: {
      uavcan::si::sample::frequency::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.hertz; return true; }
      break;
    }
    case DsdlSensorType::ANGLE: {
      uavcan::si::sample::angle::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.radian; return true; }
      break;
    }
    case DsdlSensorType::ANGULAR_VELOCITY: {
      uavcan::si::sample::angular_velocity::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.radian_per_second; return true; }
      break;
    }
    case DsdlSensorType::ELECTRIC_CHARGE: {
      uavcan::si::sample::electric_charge::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.coulomb; return true; }
      break;
    }
    case DsdlSensorType::LUMINANCE: {
      uavcan::si::sample::luminance::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.candela_per_square_meter; return true; }
      break;
    }
    case DsdlSensorType::DURATION: {
      uavcan::si::sample::duration::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.second; return true; }
      break;
    }
    case DsdlSensorType::VOLUME: {
      uavcan::si::sample::volume::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.cubic_meter; return true; }
      break;
    }
    case DsdlSensorType::VOLUMETRIC_FLOW_RATE: {
      uavcan::si::sample::volumetric_flow_rate::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.cubic_meter_per_second; return true; }
      break;
    }
    case DsdlSensorType::MAGNETIC_FIELD_STRENGTH: {
      uavcan::si::sample::magnetic_field_strength::Scalar_1_1 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.ampere_per_meter; return true; }
      break;
    }
    case DsdlSensorType::MAGNETIC_FLUX_DENSITY: {
      uavcan::si::sample::magnetic_flux_density::Scalar_1_0 msg;
      auto result = deserialize(msg, span);
      if (result >= 0) { *value = msg.tesla; return true; }
      break;
    }
    default:
      break;
  }
  
  ESP_LOGW(TAG, "Failed to deserialize value for type %s", get_dsdl_type_name().c_str());
  return false;
}

}  // namespace sensor
}  // namespace cyphal
}  // namespace esphome
