# 示例

[English](README.md)

本目录包含 Waveshare ESP32-S3-Touch-AMOLED-1.8 开发板的 ESP-IDF 工程和 Arduino 草图。

## ESP-IDF

[esp-idf](esp-idf/) 下的每个目录都是独立的 ESP-IDF 工程。除非示例 README 另有说明，请在所选示例目录中运行 ESP-IDF 命令。

```bash
cd examples/esp-idf/00_board_check
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板串口。

## BSP 组件示例

这些示例参考 ESP32-P4 平台的 ESP-IDF 示例顺序，并针对 ESP32-S3-Touch-AMOLED-1.8 和托管 `waveshare/esp32_s3_touch_amoled_1_8` 组件进行适配。

| 目录 | 用途 | 说明 |
| --- | --- | --- |
| [00_board_check](esp-idf/00_board_check/) | 开发板能力和引脚摘要 | 推荐的首次纯串口检查 |
| [00_bsp_quickstart](esp-idf/00_bsp_quickstart/) | 交互式 BSP/LVGL 快速开始 | 显示、触摸、亮度和 SD 探测 |
| [01_project_template](esp-idf/01_project_template/) | 最小托管 BSP 工程模板 | 声明在线 BSP 依赖 |
| [02_hello_world](esp-idf/02_hello_world/) | ESP-IDF hello world | 芯片信息、倒计时和重启 |
| [03_nvs_counter](esp-idf/03_nvs_counter/) | NVS 持久化启动计数器 | 演示 Flash 后备状态 |
| [04_freertos_tasks](esp-idf/04_freertos_tasks/) | FreeRTOS 任务和队列演示 | 生产者/消费者日志 |
| [05_gpio_io](esp-idf/05_gpio_io/) | GPIO 输入/输出回环 | 在 menuconfig 中选择安全的外露引脚 |
| [06_gpio_interrupt](esp-idf/06_gpio_interrupt/) | GPIO 中断处理 | 在 menuconfig 中选择安全输入引脚 |
| [08_i2c_tools](esp-idf/08_i2c_tools/) | I2C 扫描工具 | 默认使用板载 I2C 总线 |
| [09_sdmmc](esp-idf/09_sdmmc/) | SDMMC/FAT 示例 | 通过 BSP SD API 挂载 |
| [10_wifi_station](esp-idf/10_wifi_station/) | Wi-Fi station 示例 | 在 menuconfig 中配置凭据 |
| [12_i2s_codec](esp-idf/12_i2s_codec/) | ES8311 扬声器播放 | 使用 BSP 音频编解码器句柄 |
| [13_display_colorbar](esp-idf/13_display_colorbar/) | 显示色条渲染 | 通过 BSP 面板绘制 RGB565 色条 |
| [14_lvgl_demo_v9](esp-idf/14_lvgl_demo_v9/) | LVGL v9 控件演示 | 使用 BSP 显示、触摸、LVGL 和亮度 |

## 硬件诊断

这些示例用于主 BSP 路径之外的板级启动检查，并归入 `90_` 范围；RTC 和 IMU 检查使用托管 Waveshare 传感器组件。

| 目录 | 用途 | 说明 |
| --- | --- | --- |
| [90_axp2101_pmu](esp-idf/90_axp2101_pmu/) | PMU 检查 | 本地组件诊断 |
| [91_pcf85063_rtc](esp-idf/91_pcf85063_rtc/) | RTC 检查 | 使用 `waveshare/pcf85063a` |
| [92_qmi8658_imu](esp-idf/92_qmi8658_imu/) | IMU 加速度和陀螺仪读数 | 使用 `waveshare/qmi8658` |

以前的本地组件 SD、带 RAM 的 LVGL 和直接 I2S 编解码器演示已移除，以避免重复工作流。请分别使用 `09_sdmmc`、`00_bsp_quickstart`、`12_i2s_codec` 和 `14_lvgl_demo_v9`。

## Arduino

Arduino 示例分为两套维护中的第一方集合。两者均使用 CI 选择的最新稳定 Arduino-ESP32 core 和各自匹配的捆绑库：

| 目录 | 用途 |
| --- | --- |
| [arduino](arduino/) | 原版 Arduino 示例集合 |
| [arduino-v2](arduino-v2/) | V2 Arduino 示例集合 |

CI 构建两个目录，但不构建捆绑库中的示例。

## 添加示例

- 将 ESP-IDF 示例放在 `esp-idf/<number>_<name>/` 下。
- 包含 `README.md`、`CMakeLists.txt`、`main/` 和 `sdkconfig.defaults`。
- 优先在 `main/idf_component.yml` 中使用托管组件，而非检入组件副本。
- 记录所需硬件、menuconfig 选项、预期串口输出和安全说明。
- 不要提交生成的 `build/`、`managed_components/`、依赖锁文件或本地 `sdkconfig`，除非有意维护。
- 公开说明保持通用且使用仓库相对路径；不要包含本地机器路径或私有网络路径。

建议学习路径请参见 [../docs/EXAMPLES_GUIDE_ZH.md](../docs/EXAMPLES_GUIDE_ZH.md)。
