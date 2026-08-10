# 13_display_colorbar

[English](README.md)

适用于 ESP32-S3-Touch-AMOLED-1.8 和 V2 开发板的最小 CO5300 面板测试。

## 硬件

- ESP32-S3-Touch-AMOLED-1.8 开发板
- USB 数据线

## 直接构建和烧录

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash
```

将 `PORT` 替换为开发板的串口。

串口监视器为可选：

```bash
idf.py -p PORT monitor
```

## 预期结果

AMOLED 会在复位后立即显示八个垂直 RGB565 色条。

## 注意事项

- 本开发板通过 QSPI 而非 MIPI-DSI 连接 CO5300。因此，`esp_lcd_dpi_panel_set_pattern()` 不可用。
- 示例直接初始化托管的 `esp_lcd_co5300` 驱动程序，并使用 `esp_lcd_panel_draw_bitmap()` 绘图；它不会初始化 LVGL 或触摸驱动程序。
- 单次 I2C 地址探测会检测 V2 开发板，并应用其 16-pixel 面板 X 偏移。
