# 09_sdmmc

[English](README.md)

改编自 ESP32-P4 平台示例的 SDMMC/FAT 文件系统示例。

此移植使用托管 S3 BSP 中的 `bsp_sdcard_mount()`，向挂载的卡写入文件、重命名一个文件、读回两个文件，然后卸载该卡。

## 硬件

- ESP32-S3-Touch-AMOLED-1.8 开发板
- USB 数据线
- FAT 格式化的 microSD 卡

## 构建和烧录

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板的串口。

## 预期结果

串口监视器会报告 SD 卡挂载、文件写入、重命名、读回和卸载步骤。如果未插入卡或无法挂载卡，示例会记录挂载错误。

## 注意事项

- 如果未报告 BSP SD 能力，请先使用 `00_board_check`。
- 调试卡检测或挂载失败时，请使用已知正常且采用 FAT 格式化的 microSD 卡。
