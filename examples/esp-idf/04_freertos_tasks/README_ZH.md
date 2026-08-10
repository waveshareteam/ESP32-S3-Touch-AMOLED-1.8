# 04_freertos_tasks

[English](README.md)

改编自 ESP32-P4 平台示例的 FreeRTOS 任务和队列演示。

生产者任务向队列发送运行时间消息，消费者任务将其输出。

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

串口监视器显示消息通过 FreeRTOS 队列从生产者任务传递到消费者任务。

## 注意事项

将此示例用作一个小型模式，把周期性采样、UI 更新和日志记录工作分离到不同任务中。
