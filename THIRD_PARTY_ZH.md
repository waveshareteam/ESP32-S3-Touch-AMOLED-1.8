# 第三方软件

[English](THIRD_PARTY.md)

本仓库包含源码示例、固件资产、托管组件引用和捆绑的第三方库。

## 托管 ESP-IDF 组件

多数 ESP-IDF 示例通过 ESP-IDF Component Manager 解析组件，包括：

- `waveshare/esp32_s3_touch_amoled_1_8`
- `waveshare/pcf85063a`
- `waveshare/qmi8658`

确切版本在各示例的 `main/idf_component.yml` 中声明，并在构建时解析。

## 捆绑库

Arduino 示例树包含 LVGL、Adafruit BusIO、SensorLib、GFX Library for Arduino 和板级辅助库等捆绑库；它们在各自目录中保留上游许可证。`examples/esp-idf/90_axp2101_pmu` 诊断示例包含用于底层 PMU 启动的本地 XPowersLib 移植，XPowersLib 文件保留上游 MIT 许可证声明。

## 固件二进制文件

`Firmware/` 下的文件是本开发板的预构建工厂固件镜像。使用说明请参见 [Firmware/README.txt](Firmware/README.txt)。

## 许可证摘要

除非文件或子目录另有说明，仓库源码和文档采用 Apache License 2.0。第三方库、生成资产和固件二进制文件可能具有附加或不同的许可条款；重新分发前请查看相应目录的声明。
