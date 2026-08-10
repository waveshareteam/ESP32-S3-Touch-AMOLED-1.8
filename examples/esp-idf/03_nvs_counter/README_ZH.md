# 03_nvs_counter

[English](README.md)

改编自 ESP32-P4 平台示例的 NVS 持久化启动计数器。

反复复位开发板，并在串口监视器中观察计数器递增。

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

串口监视器会输出 `Saved boot count: N`。按下复位后，该数字应在每次启动后递增。

## 注意事项

此示例直接使用 ESP-IDF NVS API。在存储应用设置（例如 Wi-Fi 凭据、校准值或用户偏好）之前，它很有帮助。
