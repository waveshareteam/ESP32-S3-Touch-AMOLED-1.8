# 05_gpio_io

[English](README.md)

改编自 ESP32-P4 平台示例的 GPIO 输入/输出回环演示。

默认配置不会驱动任何引脚。使用 `idf.py menuconfig` 选择两个已引出且未使用的 GPIO，然后用跳线将输出连接到输入。

## 硬件

- ESP32-S3-Touch-AMOLED-1.8 开发板
- USB 数据线
- 一根用于回环测试的跳线

## 配置

运行 `idf.py menuconfig` 并设置：

- 输出 GPIO
- 输入 GPIO
- 如有需要，设置轮询间隔

选择已引出且未被闪存、PSRAM、显示、触摸、SD 卡、音频、USB、PMU、RTC 或 IMU 功能保留的 GPIO。

## 构建和烧录

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板的串口。

## 预期结果

连接输出和输入引脚后，串口监视器会在输出切换时报告匹配的 GPIO 电平。

## 安全注意事项

- 请勿将两个输出端连接在一起。
- 除非了解开发板原理图和电压要求，否则不要驱动连接到板载外设的引脚。
