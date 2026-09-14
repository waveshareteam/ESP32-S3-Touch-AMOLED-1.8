# 固件工件

[English](FIRMWARE.md)

本仓库有两类不同的固件工件。

## 默认固件

[`firmware/brookesia/`](../firmware/brookesia/) 包含本开发板默认 ESP-Brookesia Phone 固件的源码。Original 与 V2 使用独立工程包装层选择对应已发布 BSP 的显示和触摸实现，并共享 Phone 应用与板级应用。

检入的合并镜像由各自最终 ESP-IDF 构建生成的 `flash_args` 制作，包含 bootloader、分区表、OTA 数据、语音模型、应用和 SPIFFS 镜像：

- `firmware/ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260824.bin`
- `firmware/ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260824.bin`

精确的配置、构建和合并命令见[默认固件构建说明](../firmware/brookesia/README_ZH.md)。这些源码工程和整片镜像与下述示例 CI 打包流程分开维护。

## 源码构建的 CI 工件

GitHub Actions 构建 `examples/esp-idf/` 下的 ESP-IDF 示例以及 `examples/arduino/examples/` 和 `examples/arduino-v2/examples/` 下的第一方 Arduino 草图。成功后工作流通过 `releases/package_firmware.py` 将构建输出打包为可刷写归档。

每个 CI 固件归档包含：

- 带 schema 版本、框架、目标、`project_path`、git SHA、`timestamp_utc`、波特率、刷写命令和二进制偏移量的 `manifest.json`
- `flash.sh`
- `flash.bat`
- 带 esptool 命令参数的 `flash_args.txt`
- `bin/` 下由清单引用的引导加载程序、分区表、应用、合并镜像或其他二进制文件

从工作流运行工件下载这些归档。CI zip 名称包含框架、示例、框架版本、目标和短提交标识；它们是 CI 验证输出，并非源文件，不应加入仓库。

## 本地发布检查

本地发布打包时，先构建目标，再从仓库根目录运行 `releases/package_firmware.py`。默认输出目录为 `releases/dist/`；CI 使用 `release-artifacts/`。生成或下载的固件包会忽略在 `release-artifacts/`、`releases/dist/` 和 `releases/downloads/` 中。
