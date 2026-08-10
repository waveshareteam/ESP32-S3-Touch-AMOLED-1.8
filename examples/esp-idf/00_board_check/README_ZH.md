# 00_board_check

[English](README.md)

适用于 ESP32-S3-Touch-AMOLED-1.8 的开发板上电检查，改编自 ESP32-P4 平台示例。

它会输出芯片、闪存、PSRAM、BSP 能力以及关键开发板引脚信息。该示例使用托管的 `waveshare/esp32_s3_touch_amoled_1_8` 组件，不需要显示屏、触摸、SD 卡、Wi-Fi 或音频配件。

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

串口监视器会输出目标芯片、闪存、PSRAM、BSP 能力、显示尺寸、I2C 引脚、SD 引脚、音频支持和周期性堆状态。如果此示例失败，请先修复基本 ESP-IDF 环境配置，再继续使用显示、SD、音频或 Wi-Fi 示例。
