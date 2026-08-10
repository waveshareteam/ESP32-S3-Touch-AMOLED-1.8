# BSP 快速入门

[English](README.md)

此示例是 Waveshare ESP32-S3-Touch-AMOLED-1.8 开发板推荐的 ESP-IDF 起点。

它使用在线 `waveshare/esp32_s3_touch_amoled_1_8` BSP 组件，而非签入的驱动程序副本。

## 功能

- 通过 BSP 初始化 CO5300 AMOLED 面板和 CST816 触摸控制器。
- 启动 LVGL 并渲染一个小型触摸仪表盘。
- 通过 LVGL 滑块和 BSP API 设置显示亮度。
- 显示堆和 PSRAM 信息。
- 启动时及通过屏幕刷新按钮，使用 `bsp_sdcard_mount()` 检测板载 SD 卡；当卡存在时写入 `/sdcard/bsp.txt`。

## 构建和烧录

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板的串口。

## 注意事项

- 未插入 SD 卡会在屏幕和串口日志中报告，但不会导致示例失败。
- 当禁用长文件名支持时，短标记文件名仍可与 FAT 卷兼容。
- 依赖项声明在 [main/idf_component.yml](main/idf_component.yml) 中。构建期间，IDF Component Manager 会将 BSP 下载到 `managed_components`。
- 自定义分区表为 LVGL、BSP 和托管组件代码留出了空间。
