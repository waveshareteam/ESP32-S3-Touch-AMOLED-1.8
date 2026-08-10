# 90_axp2101_pmu

[English](README.md)

适用于 Waveshare ESP32-S3-Touch-AMOLED-1.8 开发板的 AXP2101 PMU 诊断示例。

这是一个旧版本地组件示例。它保留在 `90_` 范围内，因为常规应用项目应从托管在线 BSP 示例开始，而不是复制低层 PMU 驱动程序代码。

## 检查内容

- 通过本地 `board_variant` 辅助程序检测开发板显示屏/触摸版本。
- 打开 PMU 使用的板载 I2C 总线。
- 通过捆绑的 XPowersLib port 初始化 AXP2101。
- 记录 PMU 稳压器状态并处理 PMU 中断轮询。

## 构建和烧录

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板的串口。

## 注意事项

- 此诊断示例包含用于 PMU bring-up 的本地组件副本。它不是新项目推荐的模板。
- 对于新应用，请使用 `01_project_template` 和托管的 `waveshare/esp32_s3_touch_amoled_1_8` BSP 依赖项。
- 除非已知所连接负载的电压要求，否则不要更改 PMU 输出设置。
