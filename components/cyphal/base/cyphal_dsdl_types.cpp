#include "cyphal_dsdl_types.h"

namespace esphome {
namespace cyphal {

DsdlTypeRegistry& DsdlTypeRegistry::instance() {
  static DsdlTypeRegistry instance;
  return instance;
}

void DsdlTypeRegistry::register_type(const std::string& name, const DsdlTypeInfo& info) {
  types_[name] = info;
}

const DsdlTypeInfo* DsdlTypeRegistry::get_type(const std::string& name) const {
  auto it = types_.find(name);
  if (it != types_.end()) {
    return &it->second;
  }
  return nullptr;
}

bool DsdlTypeRegistry::has_type(const std::string& name) const {
  return types_.find(name) != types_.end();
}

void DsdlTypeRegistry::init_standard_types() {
  // uavcan.primitive.scalar.Real32 - 通用浮点数值
  register_type("uavcan.primitive.scalar.Real32", {
    .full_name = "uavcan.primitive.scalar.Real32",
    .include_path = "types/uavcan/primitive/scalar/Real32_1_0.hpp",
    .cpp_namespace = "uavcan::primitive::scalar",
    .cpp_class = "Real32_1_0",
    .extent_bytes = 64,
    .has_timestamp = false,
    .value_field = "value",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.primitive.scalar.Integer16 - 通用整数
  register_type("uavcan.primitive.scalar.Integer16", {
    .full_name = "uavcan.primitive.scalar.Integer16",
    .include_path = "types/uavcan/primitive/scalar/Integer16_1_0.hpp",
    .cpp_namespace = "uavcan::primitive::scalar",
    .cpp_class = "Integer16_1_0",
    .extent_bytes = 64,
    .has_timestamp = false,
    .value_field = "value",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.temperature.Scalar - 温度
  register_type("uavcan.si.sample.temperature.Scalar", {
    .full_name = "uavcan.si.sample.temperature.Scalar",
    .include_path = "types/uavcan/si/sample/temperature/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::temperature",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "kelvin",
    .to_si = celsius_to_kelvin,
    .from_si = kelvin_to_celsius
  });
  
  // uavcan.si.sample.pressure.Scalar - 压力
  register_type("uavcan.si.sample.pressure.Scalar", {
    .full_name = "uavcan.si.sample.pressure.Scalar",
    .include_path = "types/uavcan/si/sample/pressure/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::pressure",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "pascal",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.power.Scalar - 功率
  register_type("uavcan.si.sample.power.Scalar", {
    .full_name = "uavcan.si.sample.power.Scalar",
    .include_path = "types/uavcan/si/sample/power/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::power",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "watt",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.voltage.Scalar - 电压
  register_type("uavcan.si.sample.voltage.Scalar", {
    .full_name = "uavcan.si.sample.voltage.Scalar",
    .include_path = "types/uavcan/si/sample/voltage/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::voltage",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "volt",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.electric_current.Scalar - 电流
  register_type("uavcan.si.sample.electric_current.Scalar", {
    .full_name = "uavcan.si.sample.electric_current.Scalar",
    .include_path = "types/uavcan/si/sample/electric_current/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::electric_current",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "ampere",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.energy.Scalar - 能量
  register_type("uavcan.si.sample.energy.Scalar", {
    .full_name = "uavcan.si.sample.energy.Scalar",
    .include_path = "types/uavcan/si/sample/energy/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::energy",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "joule",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.length.Scalar - 长度/距离
  register_type("uavcan.si.sample.length.Scalar", {
    .full_name = "uavcan.si.sample.length.Scalar",
    .include_path = "types/uavcan/si/sample/length/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::length",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "meter",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.velocity.Scalar - 速度
  register_type("uavcan.si.sample.velocity.Scalar", {
    .full_name = "uavcan.si.sample.velocity.Scalar",
    .include_path = "types/uavcan/si/sample/velocity/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::velocity",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "meter_per_second",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.acceleration.Scalar - 加速度
  register_type("uavcan.si.sample.acceleration.Scalar", {
    .full_name = "uavcan.si.sample.acceleration.Scalar",
    .include_path = "types/uavcan/si/sample/acceleration/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::acceleration",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "meter_per_second_per_second",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.mass.Scalar - 质量
  register_type("uavcan.si.sample.mass.Scalar", {
    .full_name = "uavcan.si.sample.mass.Scalar",
    .include_path = "types/uavcan/si/sample/mass/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::mass",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "kilogram",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.force.Scalar - 力
  register_type("uavcan.si.sample.force.Scalar", {
    .full_name = "uavcan.si.sample.force.Scalar",
    .include_path = "types/uavcan/si/sample/force/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::force",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "newton",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.torque.Scalar - 扭矩
  register_type("uavcan.si.sample.torque.Scalar", {
    .full_name = "uavcan.si.sample.torque.Scalar",
    .include_path = "types/uavcan/si/sample/torque/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::torque",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "newton_meter",
    .to_si = identity,
    .from_si = identity
  });
  
  // uavcan.si.sample.frequency.Scalar - 频率
  register_type("uavcan.si.sample.frequency.Scalar", {
    .full_name = "uavcan.si.sample.frequency.Scalar",
    .include_path = "types/uavcan/si/sample/frequency/Scalar_1_0.hpp",
    .cpp_namespace = "uavcan::si::sample::frequency",
    .cpp_class = "Scalar_1_0",
    .extent_bytes = 64,
    .has_timestamp = true,
    .value_field = "hertz",
    .to_si = identity,
    .from_si = identity
  });
}

}  // namespace cyphal
}  // namespace esphome
