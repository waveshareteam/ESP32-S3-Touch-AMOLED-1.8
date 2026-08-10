# 08_i2c_tools

[English](README.md)

改编自 ESP32-P4 平台示例的 I2C 扫描工具。

默认情况下，它扫描开发板 I2C 总线，该总线由 PMU、触摸、RTC/音频控制及其他板载设备使用。如有需要，请使用 `idf.py menuconfig` 选择外部 I2C 总线。

## 硬件

- ESP32-S3-Touch-AMOLED-1.8 开发板
- USB 数据线
- 可选的外部 I2C 设备

## 配置

默认设置扫描板载 I2C 总线。对于外部设备，运行 `idf.py menuconfig` 并选择外部总线的 SDA、SCL、上拉和扫描范围设置。

## 构建和烧录

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板的串口。

## 预期结果

串口监视器会输出检测到的 I2C 地址。在调试 PMU、RTC、触摸、IMU 或编解码器控制故障之前，请先使用此工具。

## 注意事项

I2C 地址存在并不能证明完整设备驱动程序配置正确，但它确认总线和上拉电阻基本正常工作。
