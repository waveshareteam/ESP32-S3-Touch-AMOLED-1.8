# 92_qmi8658_imu

[English](README.md)

适用于 Waveshare ESP32-S3-Touch-AMOLED-1.8 开发板的 QMI8658 IMU 诊断示例。

此示例使用托管的 `waveshare/qmi8658` 组件，而不是旧的签入驱动程序副本。它保持代码简洁，并专注于开发板级 IMU bring-up 路径。

## 检查内容

- 通过本地 `board_variant` 辅助程序检测开发板显示屏/触摸版本。
- 打开 SDA 15 和 SCL 14 上的板载 I2C 总线。
- 先探测 QMI8658 地址 `0x6B`，然后回退到 `0x6A`。
- 读取并记录 `WHO_AM_I` 寄存器。
- 在配置传感器前执行数据手册规定的软复位序列。
- 将加速度计量程配置为 `4G`，输出数据速率为 `250 Hz`。
- 将陀螺仪量程配置为 `256 dps`，输出数据速率为 `250 Hz`。
- 当数据就绪时，每 200 ms 记录加速度计、陀螺仪、温度和时间戳数据。

## 托管组件

依赖项在 [main/idf_component.yml](main/idf_component.yml) 中声明：

```yaml
dependencies:
  idf: ">=5.5,<6.1"
  waveshare/qmi8658:
    version: "^2.0.0"
    public: true
```

## 构建和烧录

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板的串口。

## 注意事项

- 如果未检测到 QMI8658，请先使用 `08_i2c_tools`。
- 示例以 `m/s^2` 报告加速度，以 `dps` 报告角速度。
- 更改复位或量程配置后，请在两个开发板版本上重新检查静止读数；垂直轴上的加速度应接近一个重力加速度，陀螺仪数值应保持接近零。
- 对于新的应用程序结构，请从 `01_project_template` 开始，并且只添加所需的传感器组件。
