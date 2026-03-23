#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <map>

namespace esphome {
namespace cyphal {

/**
 * DSDL类型信息结构
 * 存储DSDL类型的元数据，用于动态类型解析
 */
struct DsdlTypeInfo {
  std::string full_name;           // 完整DSDL名称，如 "uavcan.si.sample.temperature.Scalar"
  std::string include_path;        // C++头文件路径
  std::string cpp_namespace;       // C++命名空间
  std::string cpp_class;           // C++类名
  size_t extent_bytes;             // 序列化缓冲区大小
  bool has_timestamp;              // 是否包含时间戳字段
  std::string value_field;         // 值字段名 (如 "kelvin", "value")
  float (*to_si)(float);           // 转换为SI单位的函数
  float (*from_si)(float);         // 从SI单位转换的函数
};

/**
 * DSDL类型注册表
 * 管理所有支持的DSDL类型
 */
class DsdlTypeRegistry {
 public:
  static DsdlTypeRegistry& instance();
  
  // 注册DSDL类型
  void register_type(const std::string& name, const DsdlTypeInfo& info);
  
  // 获取类型信息
  const DsdlTypeInfo* get_type(const std::string& name) const;
  
  // 检查类型是否支持
  bool has_type(const std::string& name) const;
  
  // 初始化所有标准DSDL类型
  void init_standard_types();
  
 private:
  DsdlTypeRegistry() { init_standard_types(); }
  std::map<std::string, DsdlTypeInfo> types_;
};

// 温度转换函数
inline float celsius_to_kelvin(float c) { return c + 273.15f; }
inline float kelvin_to_celsius(float k) { return k - 273.15f; }

// 单位转换函数
inline float identity(float v) { return v; }

}  // namespace cyphal
}  // namespace esphome
