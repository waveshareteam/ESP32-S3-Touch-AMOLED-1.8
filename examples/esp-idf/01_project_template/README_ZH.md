# 01_project_template

[English](README.md)

改编自 ESP32-P4 平台示例的最小 ESP-IDF 项目骨架。

与空项目不同，此模板已声明在线 `waveshare/esp32_s3_touch_amoled_1_8` 依赖项并公开 BSP 开发板定义，因此它是新建开发板项目的实用起点。

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

串口监视器会输出开发板名称、I2C 引脚、显示分辨率，以及周期性的 `template running` 消息。开始新的 ESP-IDF 应用且需要使用托管 BSP 组件时，请复制此项目。

## 注意事项

- 仅初始化应用需要的开发板服务，例如显示、触摸、SD 卡、音频、RTC 或 IMU。
- 将托管依赖项保留在 `main/idf_component.yml` 中。
- 请勿将生成的 `managed_components/`、`dependencies.lock` 或本地 `sdkconfig` 文件复制到可复用模板中。
