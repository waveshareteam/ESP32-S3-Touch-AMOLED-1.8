# 发布脚本

[English](README.md)

本目录包含将 CI 构建输出打包为可刷写固件归档，以及下载已完成工作流工件的辅助脚本。

## ESP-IDF

先构建示例，再打包生成的构建目录：

```bash
idf.py -C examples/esp-idf/00_board_check -B build/00_board_check-v6.0.2 set-target esp32s3 build
python3 releases/package_firmware.py \
  --framework esp-idf \
  --project examples/esp-idf/00_board_check \
  --build-dir build/00_board_check-v6.0.2 \
  --name ESP32-S3-Touch-AMOLED-1.8-00_board_check-esp-idf-v6.0.2-esp32s3 \
  --framework-version v6.0.2 \
  --target esp32s3
```

脚本读取 ESP-IDF `flasher_args.json`，复制所需二进制文件，写入刷写辅助脚本，并在 `releases/dist/` 下创建 zip 文件。

## Arduino

将二进制文件导出到稳定输出目录，然后打包：

```bash
arduino-cli compile \
  --fqbn esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB \
  --libraries examples/arduino/libraries \
  --export-binaries \
  --output-dir build/arduino-01_HelloWorld-3.3.11 \
  examples/arduino/examples/01_HelloWorld

python3 releases/package_firmware.py \
  --framework arduino \
  --project examples/arduino/examples/01_HelloWorld \
  --build-dir build/arduino-01_HelloWorld-3.3.11 \
  --name ESP32-S3-Touch-AMOLED-1.8-arduino-01_HelloWorld-arduino-3.3.11-esp32s3 \
  --framework-version 3.3.11 \
  --target esp32s3
```

V2 草图使用 `examples/arduino-v2/libraries` 和 `examples/arduino-v2/examples/` 下的工程。

每个归档包含 `manifest.json`、`flash.sh`、`flash.bat`、`flash_args.txt`、包内 `README.md` 和 `bin/` 下的固件二进制文件。清单记录 `project_path`、`timestamp_utc`、框架与目标元数据、git SHA、波特率、刷写命令和二进制偏移量。

CI 将工作流提交 SHA 传给打包器，因此生成的 zip 名称以短提交标识结尾；外层 GitHub 工件名称保持稳定，便于筛选和下载。

## 下载 CI 工件

CI 运行完成后，使用以下命令下载并解压固件工件：

```bash
python3 releases/download_artifacts.py --run-id <run-id> --clean
```

省略 `--run-id` 时，脚本会查找当前分支最新成功的 `examples.yml` 运行：

```bash
python3 releases/download_artifacts.py --clean
```

解压后的固件写入 `releases/downloads/run-<run-id>/`。每个工件有独立目录，例如 `firmware-esp-idf-00_board_check-v6.0.2/` 或 `firmware-arduino-arduino-v2-01_HelloWorld-3.3.11/`，其中包含可直接刷写的 `flash.sh`、`flash.bat`、`manifest.json`、`flash_args.txt` 和 `bin/`。

使用 `--artifact <name>` 下载一个固件包，或使用 `--pattern "firmware-arduino-*"` 按 glob 过滤。脚本使用 `GH_TOKEN`、`GITHUB_TOKEN` 或 `gh auth token` 访问 GitHub 工件；安装 GitHub CLI 后，下载使用 `gh run download`，可直接复用 `gh auth login`。

## 生成的输出

生成的归档、下载的工作流工件和构建目录均被 git 忽略。请勿提交生成的 zip 文件、解压的固件文件夹或本地构建输出。

`Firmware/` 下的工厂二进制文件是独立恢复资产，不会重新打包为 CI 构建输出。它们对应的源码和构建说明尚未包含在本仓库中，可能在后续更新中提供。
