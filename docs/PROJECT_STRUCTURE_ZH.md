# 项目结构

[English](PROJECT_STRUCTURE.md)

仓库将硬件资产、示例和 CI 支持分开，便于用户快速找到合适的入口。

| 路径 | 作用 |
| --- | --- |
| `README.md` | 仓库概览和推荐入口 |
| `README_ZH.md` | 维护中的简体中文仓库概览 |
| `README_CN.md` | 指向 `README_ZH.md` 的兼容入口 |
| `docs/` | 设置、示例、固件和 CI 文档 |
| `examples/README_ZH.md` | 中文示例索引 |
| `examples/esp-idf/` | 独立 ESP-IDF 工程 |
| `examples/esp-idf/00_board_check` | 托管 BSP 串口冒烟检查 |
| `examples/esp-idf/00_bsp_quickstart` | 托管 BSP 显示、触摸、LVGL 和 SD 冒烟检查 |
| `examples/esp-idf/01_project_template` | 最小托管 BSP 工程模板 |
| `examples/esp-idf/0*_*, examples/esp-idf/1*_*` | 板卡、操作系统、外设、显示和 LVGL 示例 |
| `examples/esp-idf/90_*` | 板级硬件诊断和托管传感器检查 |
| `examples/arduino/` | 原版 Arduino 草图和捆绑库 |
| `examples/arduino-v2/` | V2 Arduino 草图和捆绑库 |
| `config/` | 共享 ESP-IDF 配置覆盖层和 CI 默认值 |
| `Firmware/` | 工厂固件二进制文件和说明；不进入源码构建 CI |
| `releases/` | 固件打包脚本和发布说明 |
| `CONTRIBUTING_ZH.md` | 贡献、示例和文档指南 |
| `CODE_OF_CONDUCT_ZH.md` | 社区参与期望 |
| `SUPPORT_ZH.md` | 支持渠道和 issue 检查表 |
| `SECURITY_ZH.md` | 漏洞报告策略 |
| `THIRD_PARTY_ZH.md` | 托管组件和捆绑库声明 |
| `.editorconfig` | 通用编辑器空白和编码设置 |
| `.gitattributes` | Git 文本和二进制文件处理 |
| `.github/workflows/` | GitHub Actions 工作流，包括 `Build Examples` |
| `scripts/` | GitHub Actions 共用的 CI 发现辅助工具 |
| `.github/ISSUE_TEMPLATE/` | 公开 issue 表单 |
| `.github/PULL_REQUEST_TEMPLATE_ZH.md` | 中文拉取请求检查表 |

新的 ESP-IDF 工作应优先使用托管组件，并避免添加生成的构建输出、本地依赖缓存或机器专用文件。
