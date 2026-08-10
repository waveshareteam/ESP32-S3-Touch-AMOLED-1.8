# 02_hello_world

[English](README.md)

适用于 ESP32-S3-Touch-AMOLED-1.8 的经典 ESP-IDF hello-world 示例。

它会输出芯片信息，等待十秒后重启。

## 硬件

- ESP32-S3-Touch-AMOLED-1.8 开发板
- USB 数据线

## 构建和烧录

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板的串口。

## 预期结果

串口监视器会输出 ESP-IDF 目标芯片、CPU 核心数、无线功能、芯片修订版本、闪存大小和最小可用堆大小。随后开发板会倒计时并重启。

## 注意事项

在添加特定开发板 BSP 代码之前，使用此示例确认干净的 ESP-IDF 工具链和串口监视器流程。
