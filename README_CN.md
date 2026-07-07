# ESP32-S3-Touch-AMOLED-1.8

[English](README.md)

本仓库提供 Waveshare ESP32-S3-Touch-AMOLED-1.8 开发板的工程示例、工厂固件和文档。

开发板集成 ESP32-S3、1.8 英寸 368 x 448 AMOLED 触摸屏、microSD、ES8311 音频编解码器、板载麦克风输入、RTC、PMU 和 IMU。ESP-IDF 示例优先使用在线托管的 `waveshare/esp32_s3_touch_amoled_1_8` BSP 组件，便于客户从统一的组件结构开始开发。

## 快速开始

推荐使用 ESP-IDF v5.5.x 或 v6.0.x，目标芯片为 `esp32s3`：

```bash
cd examples/esp-idf/00_board_check
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板串口。

`00_board_check` 是最适合首次运行的串口自检示例，会打印芯片、Flash、PSRAM、BSP 能力、显示分辨率、I2C、SD 和音频能力。需要图形界面验证时，再运行 `00_bsp_quickstart`。

## 示例路径

ESP-IDF 示例按照从简单到复杂的顺序组织：

| 目录 | 用途 |
| --- | --- |
| [00_board_check](examples/esp-idf/00_board_check/) | 板级与 BSP 能力自检 |
| [00_bsp_quickstart](examples/esp-idf/00_bsp_quickstart/) | BSP、显示、触摸、LVGL、亮度和 SD 快速验证 |
| [01_project_template](examples/esp-idf/01_project_template/) | 最小托管 BSP 项目模板 |
| [02_hello_world](examples/esp-idf/02_hello_world/) | ESP-IDF Hello World |
| [03_nvs_counter](examples/esp-idf/03_nvs_counter/) | NVS 持久化计数 |
| [04_freertos_tasks](examples/esp-idf/04_freertos_tasks/) | FreeRTOS 任务与队列 |
| [05_gpio_io](examples/esp-idf/05_gpio_io/) | GPIO 输入输出回环 |
| [06_gpio_interrupt](examples/esp-idf/06_gpio_interrupt/) | GPIO 中断 |
| [08_i2c_tools](examples/esp-idf/08_i2c_tools/) | I2C 扫描 |
| [09_sdmmc](examples/esp-idf/09_sdmmc/) | SDMMC/FAT 文件读写 |
| [10_wifi_station](examples/esp-idf/10_wifi_station/) | Wi-Fi Station |
| [12_i2s_codec](examples/esp-idf/12_i2s_codec/) | ES8311 扬声器播放 |
| [13_display_colorbar](examples/esp-idf/13_display_colorbar/) | AMOLED 色条显示 |
| [14_lvgl_demo_v9](examples/esp-idf/14_lvgl_demo_v9/) | LVGL v9 控件演示 |

`90_` 范围保留给板级硬件诊断：

| 目录 | 用途 |
| --- | --- |
| [90_axp2101_pmu](examples/esp-idf/90_axp2101_pmu/) | AXP2101 PMU 诊断 |
| [91_pcf85063_rtc](examples/esp-idf/91_pcf85063_rtc/) | PCF85063A RTC 诊断 |
| [92_qmi8658_imu](examples/esp-idf/92_qmi8658_imu/) | QMI8658 IMU 诊断 |

完整说明见 [examples/README.md](examples/README.md) 和 [docs/EXAMPLES_GUIDE.md](docs/EXAMPLES_GUIDE.md)。

## 文档

- [入门指南](docs/GETTING_STARTED.md)
- [示例指南](docs/EXAMPLES_GUIDE.md)
- [项目结构](docs/PROJECT_STRUCTURE.md)
- [CI 说明](docs/CI.md)
- [贡献指南](CONTRIBUTING.md)
- [支持说明](SUPPORT.md)
- [第三方软件说明](THIRD_PARTY.md)

## CI

GitHub Actions 使用 ESP-IDF v5.5.4 和 v6.0.2 为 `esp32s3` 构建选中的 ESP-IDF 示例。PR 中修改哪个 ESP-IDF 示例就构建哪个；修改 workflow、discovery 脚本、打包脚本或共享配置时构建全部 ESP-IDF 示例。成功的 CI 构建会上传源码构建出的可刷写固件压缩包；仓库内的工厂固件仅作为恢复/出厂镜像保留，不纳入 CI 构建。

## 支持

- 产品 Wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.8
- BSP 组件: https://components.espressif.com/components/waveshare/esp32_s3_touch_amoled_1_8
- Issues: https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/issues

提交问题时，请提供开发板版本、示例路径、ESP-IDF 版本、串口日志以及外接设备信息。

## 许可证

除子目录或文件另有说明外，本仓库源码和文档使用 Apache License 2.0。第三方库保留各自许可证和声明，详见 [THIRD_PARTY.md](THIRD_PARTY.md)。
