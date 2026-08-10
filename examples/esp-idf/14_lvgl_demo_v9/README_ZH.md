# 14_lvgl_demo_v9

[English](README.md)

改编自 ESP32-P4 平台示例的 LVGL v9 控件演示。

此移植使用托管 S3 BSP 初始化 AMOLED 面板、触摸输入、LVGL port 和亮度控制，然后启动 `lv_demo_widgets()`。

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

AMOLED 显示屏会显示 LVGL 控件演示并响应触摸输入。串口监视器会报告显示屏和 LVGL 初始化状态。

## 注意事项

- 此示例需要 PSRAM 和更大的应用程序分区；请保留提供的 `sdkconfig.defaults` 和 `partitions.csv`。
- 如果显示屏未点亮，请先运行 `13_display_colorbar`。
- 在 ESP-IDF 5.5 上，示例会在 BSP 探测备用触摸控制器地址时抑制预期的 I2C NACK 日志，然后恢复先前的日志级别。
