# 贡献指南

[English](CONTRIBUTING.md)

感谢您帮助改进 ESP32-S3-Touch-AMOLED-1.8 示例。本仓库面向客户，变更应让示例易于构建、比较并可在开发板上安全运行。

## 创建拉取请求前

- ESP-IDF 示例使用 ESP-IDF v5.5.x，除非示例另有说明。
- 对变更的 ESP-IDF 示例使用目标 `esp32s3` 构建。
- 将 Arduino 变更保留在 Arduino 示例目录；ESP-IDF CI 工作流不构建 Arduino 草图。
- 在 `main/idf_component.yml` 中优先使用托管 ESP-IDF 组件，而非提交组件副本。
- 不要提交生成的 `build/`、`managed_components/`、`dependencies.lock`、本地 `sdkconfig` 或缓存文件。
- 行为、硬件要求、menuconfig 选项或预期输出变更时更新示例 README。

## 示例风格

新的 ESP-IDF 示例应位于 `examples/esp-idf/<number>_<name>/`，并包含 `README.md`、`CMakeLists.txt`、`main/CMakeLists.txt`、`main/` 源文件和 `sdkconfig.defaults`。请遵循 [docs/EXAMPLES_GUIDE_ZH.md](docs/EXAMPLES_GUIDE_ZH.md) 的学习顺序；板级底层诊断保留在 `90_` 范围。

## 文档风格

- 使用仓库相对路径编写公开说明。
- 避免本地机器路径、用户名、私有网络路径或工具安装目录。
- 包含命令顺序、所需硬件、配置说明和预期结果。
- 示例依赖托管组件或上游项目时提供链接。

## CI

修改 `examples/esp-idf/` 的拉取请求会构建已修改的 ESP-IDF 示例；工作流或发现脚本变更会构建全部 ESP-IDF 示例。详见 [docs/CI_ZH.md](docs/CI_ZH.md)。
