# ESPHome Cyphal 组件

这是一个用于 [ESPHome](https://esphome.io) 的自定义组件，支持通过 [Cyphal](https://opencyphal.org/) (原 UAVCAN) 协议进行 CAN 总线通信。

## 特点

- 支持节点模式和网关模式。
- 基于 `libcanard` 和 `o1heap`。
- 支持标准 DSDL 类型。
- 提供常见的 ESPHome 实体：传感器、二进制传感器、开关、灯光。
- 支持发布和订阅两种工作模式。

## 安装

在你的 ESPHome 配置文件中添加 `external_components`：

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/your-username/esphome_cyphal
```

## 核心配置

### `cyphal`

这是核心组件，负责管理 CAN 总线和 Cyphal 节点。

```yaml
cyphal:
  id: my_cyphal
  canbus_id: can_bus
  node_id: 42
  mode: node # node 或 gateway
```

**配置选项:**

- **id** (必填, ID): 为此组件指定一个 ID，以便其他组件引用。
- **canbus_id** (必填, ID): 引用 `canbus` 组件的 ID。
- **node_id** (必填, 0-127): 本节点的 Cyphal ID。
- **mode** (可选, 字符串): 节点模式，默认为 `node`。

---

## 传感器

### `sensor.cyphal`

支持发布本地传感器数据到 Cyphal 网络，或订阅 Cyphal 网络中的数据。

#### 订阅模式 (默认)

```yaml
sensor:
  - platform: cyphal
    cyphal_id: my_cyphal
    name: "Remote Temperature"
    subject_id: 3001
    dsdl_type: "uavcan.si.sample.temperature.Scalar"
    type: subscribe
```

#### 发布模式

```yaml
sensor:
  - platform: cyphal
    cyphal_id: my_cyphal
    name: "Local Temperature Publisher"
    subject_id: 3002
    dsdl_type: "uavcan.si.sample.temperature.Scalar"
    type: publish
    source_sensor: local_temp_sensor_id
    publish_interval: 5s
```

---

## 二进制传感器

### `binary_sensor.cyphal`

#### 订阅模式 (默认)

```yaml
binary_sensor:
  - platform: cyphal
    cyphal_id: my_cyphal
    name: "Remote Motion Sensor"
    subject_id: 1001
    dsdl_type: "uavcan.primitive.scalar.Bit"
```

---

## 开关

### `switch.cyphal`

#### 发布模式 (将本地开关状态同步到网络)

```yaml
switch:
  - platform: cyphal
    cyphal_id: my_cyphal
    name: "Remote Relay"
    subject_id: 2001
    type: publish
```

---

## 灯光

### `light.cyphal`

#### 订阅模式

```yaml
light:
  - platform: cyphal
    cyphal_id: my_cyphal
    name: "Remote Light"
    subject_id: 4001
    type: subscribe
```

## 许可

MIT License
