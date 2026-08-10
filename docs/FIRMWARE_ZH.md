# 固件工件

[English](FIRMWARE.md)

本仓库有两类不同的固件工件。

## 工厂二进制文件

`Firmware/` 包含随产品提供的预构建工厂或恢复镜像。这些二进制文件有意随仓库检入，使用户无需源码构建即可恢复或还原开发板。其对应源码和构建说明尚未包含在本仓库中，可能在后续更新中提供。

工厂二进制文件不是源码工程：

- GitHub Actions 不会重新构建它们。
- 不会将其重新打包为 CI 工件。
- 不应视为本仓库生成的输出。

### 只读身份记录

下列 SHA-256 值用于标识随仓库提供的工厂二进制文件。这些是只读身份记录，不是可复现构建声明。

- `Firmware/ESP32-S3-Touch-AMOLED-1.8-FactoryXiaozhi_250805.bin`: `033BA27F0D1824835E90FE6B41D2DB8C1F13CDA7E1D80C82B3F7537DAFB8DC8D`
- `Firmware/ESP32-S3-Touch-AMOLED-1.8-V2-FactoryXiaozhi_260601.bin`: `6F188FB9D35EE793A3423934A4FA4E7C1FEF9CC9DAE76F9F177DABE854A6CDB3`

工厂二进制文件的文档应聚焦板卡修订、预期用途和刷写或恢复说明。

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
