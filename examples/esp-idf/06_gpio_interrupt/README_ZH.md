# 06_gpio_interrupt

[English](README.md)

改编自 ESP32-P4 平台示例的 GPIO 中断演示。

默认配置是安全的空运行。测试中断前，请使用 `idf.py menuconfig` 选择一个已引出且未使用的输入 GPIO。

## 硬件

- ESP32-S3-Touch-AMOLED-1.8 开发板
- USB 数据线
- 可选的跳线、按钮或外部信号源

## 配置

运行 `idf.py menuconfig` 并设置输入 GPIO 和中断边沿。请选择已引出且未被板载外设保留的引脚。

## 构建和烧录

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板的串口。

## 预期结果

当配置的输入按所选边沿变化时，串口监视器会报告中断计数。

## 安全注意事项

请将外部信号保持在开发板 I/O 电压限制内。请勿将外部信号连接到保留的开发板引脚。
