# 10_wifi_station

[English](README.md)

面向 ESP32-S3、改编自 ESP32-P4 平台示例的 Wi-Fi Station 示例。

使用 `idf.py menuconfig` 配置 SSID 和密码，然后构建并烧录。

## 硬件

- ESP32-S3-Touch-AMOLED-1.8 开发板
- USB 数据线
- 2.4 GHz Wi-Fi 接入点

## 配置

运行 `idf.py menuconfig` 并设置：

- Wi-Fi SSID
- Wi-Fi 密码
- 可选的重试次数

切勿将 Wi-Fi 凭据提交到 `sdkconfig` 或文档中。

## 构建和烧录

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
```

将 `PORT` 替换为开发板的串口。

## 预期结果

串口监视器会报告 Wi-Fi 初始化、连接进度和获取到的 IP 地址。

## 注意事项

ESP32-S3 Station 模式使用 2.4 GHz Wi-Fi。如果开发板无法发现或加入网络，请检查 AP 的频段设置。
