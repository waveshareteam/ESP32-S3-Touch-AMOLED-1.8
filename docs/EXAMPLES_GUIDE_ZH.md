# ESP-IDF 示例指南

[English](EXAMPLES_GUIDE.md)

从纯串口检查开始，再逐步使用板级服务、外设和显示 UI。

## 建议路径

1. [00_board_check](../examples/esp-idf/00_board_check/) 确认目标、Flash、PSRAM、BSP 组件、显示几何、I2C 引脚、SD 引脚和音频能力。
2. [00_bsp_quickstart](../examples/esp-idf/00_bsp_quickstart/) 启动托管 BSP 显示、触摸、亮度控制、LVGL 仪表盘和 SD 探测。
3. [01_project_template](../examples/esp-idf/01_project_template/) 是新应用最小而实用的托管 BSP 工程模板。
4. [02_hello_world](../examples/esp-idf/02_hello_world/)、[03_nvs_counter](../examples/esp-idf/03_nvs_counter/) 和 [04_freertos_tasks](../examples/esp-idf/04_freertos_tasks/) 在板级外设之前覆盖 ESP-IDF 基础。
5. [05_gpio_io](../examples/esp-idf/05_gpio_io/) 和 [06_gpio_interrupt](../examples/esp-idf/06_gpio_interrupt/) 演示可配置 GPIO 回环和中断测试，默认不占用开发板关键引脚。
6. [08_i2c_tools](../examples/esp-idf/08_i2c_tools/) 扫描板载 I2C 总线，在调试 PMU、RTC、触摸或音频控制设备之前很有用。
7. [09_sdmmc](../examples/esp-idf/09_sdmmc/) 通过托管 BSP 挂载 microSD 卡并执行 FAT 文件操作。
8. [10_wifi_station](../examples/esp-idf/10_wifi_station/) 使用 menuconfig 提供的凭据将 ESP32-S3 连接到接入点。
9. [12_i2s_codec](../examples/esp-idf/12_i2s_codec/) 通过 BSP 音频 API 验证 ES8311 扬声器播放。
10. [13_display_colorbar](../examples/esp-idf/13_display_colorbar/) 直接通过 BSP 显示面板句柄绘制 RGB565 测试条。
11. [14_lvgl_demo_v9](../examples/esp-idf/14_lvgl_demo_v9/) 启动带 BSP 显示、触摸、LVGL port 和亮度控制的 LVGL v9 控件演示。

## 硬件诊断

不完全属于托管 BSP 学习路径的板级诊断保留在 `90_` 范围：

| 目录 | 检查用途 |
| --- | --- |
| [90_axp2101_pmu](../examples/esp-idf/90_axp2101_pmu/) | 使用本地诊断组件进行 AXP2101 PMU 寄存器启动 |
| [91_pcf85063_rtc](../examples/esp-idf/91_pcf85063_rtc/) | 通过 `waveshare/pcf85063a` 进行 PCF85063A RTC 计时 |
| [92_qmi8658_imu](../examples/esp-idf/92_qmi8658_imu/) | 通过 `waveshare/qmi8658` 获取 QMI8658 加速度、陀螺仪、温度和时间戳 |

以前的本地 `04_SD_MMC`、`05_LVGL_WITH_RAM` 和直接 `06_I2SCodec` 演示已移除，因为托管 BSP 示例以更少重复驱动代码覆盖这些工作流。

## 组件策略

新的 ESP-IDF 示例应尽可能使用托管依赖。推荐的开发板依赖是：

```yaml
dependencies:
  idf: ">=5.5,<6.1"
  waveshare/esp32_s3_touch_amoled_1_8: "^2.0.3"
```

ESP Component Registry 在 2026-08-10 将 `2.0.3` 列为 `esp32s3` 的最新 `waveshare/esp32_s3_touch_amoled_1_8` 版本。仅当示例有意演示本地补丁或兼容性回退时使用检入的组件副本。RTC 和 IMU 诊断分别使用仍为当前托管传感器版本的 `waveshare/pcf85063a` `^2.0.0` 和 `waveshare/qmi8658` `^2.0.0`。`idf: ">=5.5,<6.1"` 上限匹配已验证的 v5.5/v6.0 矩阵；当需要较新的 v6 次版本且其已通过 CI 时，必须重新评估该上限。

## CI 覆盖

`Build Examples` 工作流以 ESP-IDF v5.5.5、v6.0.2 构建第一方 ESP-IDF 示例，并以 Arduino-ESP32 core 3.3.11 构建第一方 Arduino 草图。这些稳定版本已于 2026-08-10 从官方发布重新核实；ESP-IDF 覆盖保留 v5.5 到 v6.0 的迁移背景且不含预发布版本。拉取请求和受支持分支上的推送会构建受变更影响的示例；标签推送和手动以 `target=all` 运行会构建完整矩阵。成功源码构建上传可刷写固件归档；检入的工厂二进制文件不进入源码构建打包。详情见 [CI_ZH.md](CI_ZH.md)、[FIRMWARE_ZH.md](FIRMWARE_ZH.md) 和 [../releases/README_ZH.md](../releases/README_ZH.md)。

## 面向客户的检查表

发布或修改示例前，请确保 README 包含所需硬件、构建命令、配置选项、预期结果和安全说明。公开文档中不要包含本地机器路径、私有网络路径、用户名、生成构建目录或临时排障笔记。
