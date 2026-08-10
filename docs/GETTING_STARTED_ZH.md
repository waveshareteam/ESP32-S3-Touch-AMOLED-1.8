# 入门指南

[English](GETTING_STARTED.md)

本指南使用 Waveshare ESP32-S3-Touch-AMOLED-1.8 开发板的 ESP-IDF 示例。

## 硬件

- Waveshare ESP32-S3-Touch-AMOLED-1.8 开发板
- 支持数据传输的 USB 线
- SD 示例可选 FAT 格式 microSD 卡
- 音频示例可选扬声器或耳机
- GPIO 回环或中断示例可选跳线

## ESP-IDF 版本

当前 ESP-IDF 示例使用 ESP-IDF v5.5.x 或 v6.0.x。CI 为目标 `esp32s3` 使用 ESP-IDF v5.5.5 和 v6.0.2 构建。首次构建时，示例通过 ESP-IDF Component Manager 下载托管依赖；请确保构建主机可联网或已配置组件缓存。

## 构建板级检查

```bash
cd examples/esp-idf/00_board_check
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板串口。该示例下载托管的 `waveshare/esp32_s3_touch_amoled_1_8` 组件，并打印芯片、Flash、PSRAM、BSP 能力、显示、I2C、SD 和音频信息。它不需要显示 UI 循环、Wi-Fi 凭据、SD 卡或音频配件，是最快的首次检查。

视觉触摸测试：

```bash
cd examples/esp-idf/00_bsp_quickstart
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

该示例初始化 AMOLED 面板和触摸控制器，并显示带亮度滑块和 SD 刷新按钮的 LVGL 仪表盘。

## 托管组件

新 ESP-IDF 工程在 `main/idf_component.yml` 中优先使用：

```yaml
dependencies:
  idf: ">=5.5,<6.1"
  waveshare/esp32_s3_touch_amoled_1_8: "^2.0.3"
```

然后包含 BSP API：

```c
#include "bsp/esp-bsp.h"
```

常见入口包括 `bsp_display_start()`、`bsp_display_lock()`、`bsp_display_brightness_set()`、`bsp_sdcard_mount()`、`bsp_audio_init()` 和 `bsp_audio_codec_speaker_init()`。除非有意固定生成状态，请勿将生成的 `managed_components/`、`dependencies.lock` 或本地 `sdkconfig` 文件复制到新应用。

## 示例顺序

`examples/esp-idf` 下的 BSP 组件示例在适用处遵循 ESP32-P4 平台学习顺序：板级检查、BSP 快速开始、工程模板、hello world、NVS、FreeRTOS、GPIO、I2C、SD 卡、Wi-Fi、音频、显示和 LVGL。仅在需要底层 PMU、RTC 或 IMU 诊断时使用 `90_axp2101_pmu`、`91_pcf85063_rtc` 和 `92_qmi8658_imu`；RTC 和 IMU 使用托管 Waveshare 传感器组件，而 PMU 示例仍是本地诊断。完整路径和硬件说明请参见 [EXAMPLES_GUIDE_ZH.md](EXAMPLES_GUIDE_ZH.md)。

## Flash 大小和 PSRAM

显示和 LVGL 示例需要启用 PSRAM 以及足够的应用分区空间。显示示例包含为本开发板调整的 `sdkconfig.defaults`，并在需要时包含 `partitions.csv`。

## 故障排除

- 在调试更高层示例前运行 `00_board_check`。
- PMU、RTC、触摸、IMU 或编解码器控制设备未检测到时使用 `08_i2c_tools`。
- 音频输出时连接扬声器或耳机，并从低音量开始。
- SD 示例使用 FAT 格式 microSD 卡，并检查串口日志中的挂载错误。
