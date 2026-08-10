# 持续集成

[English](CI.md)

本仓库使用 `Build Examples` GitHub Actions 工作流动态发现示例、构建示例并上传可刷写的源码构建固件工件。

## 发现

工作流通过 `scripts/discover_examples.py` 处理两个框架表面：

- ESP-IDF 工程从带有 `main/` 目录的 `examples/esp-idf/*/CMakeLists.txt` 发现。
- Arduino 草图从 `examples/arduino/examples/` 和 `examples/arduino-v2/examples/` 下的第一方草图目录发现。
- 捆绑库中的 Arduino 草图有意不纳入产品 CI。

`workflow_dispatch` 接受 `all`、示例目录名或仓库相对示例路径。维护者可运行完整矩阵或单个示例。

仓库策略作业在拉取请求中始终可见。其路由契约为失败关闭：不可用或不完整的差异数据是错误，而不是回退构建或通过。仅文档变更（包括 `Firmware/` 文档或不可变二进制文件）会被报告但默认不构建示例；`Firmware/` 不属于示例矩阵。可用的拉取请求或分支差异仅构建受影响的第一方示例。捆绑库变更会重建同一 Arduino 根目录的全部第一方草图。工作流、发现、策略、路由或发布打包变更运行相关框架的完整表面；共享 ESP-IDF 配置变更重建 ESP-IDF 表面。标签推送和手动 `all` 运行构建完整矩阵。

手动运行可通过 `target` 指定示例名、父目录（如 `08_LVGL_Animation`）或仓库相对路径。

## 矩阵

当前 CI 矩阵：

- ESP-IDF `v5.5.5` 和 `v6.0.2`，目标 `esp32s3`。
- Arduino-ESP32 core `3.3.11`，FQBN `esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB`，使用对应 `examples/arduino/libraries` 或 `examples/arduino-v2/libraries` 目录中的捆绑库。

这些框架版本已于 2026-08-10 从官方稳定版发布重新核实：ESP-IDF `v5.5.5`、`v6.0.2` 与 Arduino-ESP32 `3.3.11`。ESP-IDF 覆盖保留 v5.5 到 v6.0 的迁移背景，不使用 beta、release-candidate、preview 或 nightly 标签。完整矩阵包含 60 个固件构建作业：17 个 ESP-IDF 示例分别针对两个 ESP-IDF 版本，加上 16 个原版和 10 个 V2 Arduino 草图。

## 固件工件

每个成功的 ESP-IDF 和 Arduino 矩阵构建都会上传可刷写固件工件。CI 通过 `releases/package_firmware.py` 打包构建输出。

每个生成归档包含：

- 带 schema 版本、框架、框架版本、目标、`project_path`、git SHA、`timestamp_utc`、波特率、刷写命令和二进制偏移量的 `manifest.json`
- `flash.sh` 和 `flash.bat` 辅助脚本
- 带 esptool 命令参数的 `flash_args.txt`
- `bin/` 下由清单引用的固件二进制文件

从工作流运行下载工件 zip，解压后使用开发板串口运行 `flash.sh` 或 `flash.bat`。CI zip 名称包含框架、示例、框架版本、目标和短提交标识；外层 GitHub 工件名称保持稳定，便于筛选和脚本下载。生成归档仅是工作流工件，不要提交 `release-artifacts/`、`releases/dist/` 或 `releases/downloads/` 中生成的文件。`Firmware/` 下检入的是工厂或恢复二进制文件，是文档化资产而非源码构建输出，也不会触发源码构建打包。

## 硬件验证边界

CI 验证源码兼容性、编译和固件打包。Arduino FQBN 固定 ESP32-S3 目标、16 MB Flash 大小和这些示例所用的应用分区布局；它不证明运行时引脚、PSRAM、USB、显示、触摸、音频或传感器正确性。本仓库尚未包含板级原理图或等效硬件参考；在添加该参考之前，引脚级验证仍待完成。发布前应将硬件相关变更与官方开发板文档和托管 BSP 核对。

## 本地脚本检查

无需构建固件即可检查发现：

```bash
python scripts/discover_examples.py --surface esp-idf --selector 00_board_check
python scripts/discover_examples.py --surface esp-idf --selector all
python scripts/discover_examples.py --surface arduino --selector all
python scripts/discover_examples.py --surface arduino --selector 08_LVGL_Animation
python -B -m unittest discover -s tests -v
python -B scripts/check_repository_policy.py --config repository_policy.json
```

最后两条是仓库策略及其测试的精确非构建检查；产品构建是独立的本地或 GitHub Actions 责任，这些检查不编译示例或验证硬件。打包辅助工具需要现有 ESP-IDF 或 Arduino 构建输出，通常在框架构建完成后由 CI 运行。如示例需要硬件、凭据或尚不兼容所选框架版本的上游组件，请在此记录排除原因后再将其排除出 CI。
