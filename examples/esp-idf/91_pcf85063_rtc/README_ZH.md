# 91_pcf85063_rtc

[English](README.md)

适用于 Waveshare ESP32-S3-Touch-AMOLED-1.8 开发板的 PCF85063A RTC 诊断示例。

此示例使用托管的 `waveshare/pcf85063a` 组件，而不是签入的 RTC 驱动程序副本。它用于在确认开发板 I2C 总线后进行低层 RTC bring-up。

## 检查内容

- 通过本地 `board_variant` 辅助程序检测开发板显示屏/触摸版本。
- 打开 SDA 15 和 SCL 14 上的板载 I2C 总线。
- 探测地址为 `0x51` 的 PCF85063A。
- 通过托管的 Waveshare 组件初始化 RTC。
- 每秒读取并记录一次 RTC 日期/时间。
- 仅当当前 RTC 内容看似无效时，写入示例时间 `2026-01-01 00:00:00`。

## 托管组件

依赖项在 [main/idf_component.yml](main/idf_component.yml) 中声明：

```yaml
dependencies:
  idf: ">=5.5,<6.1"
  waveshare/pcf85063a:
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

- 如果未检测到 RTC，请先使用 `08_i2c_tools`。
- 当需要只读 RTC 检查时，将 [main/pcf85063_rtc.c](main/pcf85063_rtc.c) 中的 `RTC_SET_SAMPLE_TIME_WHEN_UNSET` 设为 `0`。
- 对于新的应用程序结构，请从 `01_project_template` 开始，并且只添加所需的传感器组件。
