# `bsp_extra` 开发板适配

[English](README.md)

`bsp_extra` 是位于 ESP32-S3-Touch-AMOLED-1.8 BSP 之上的固件本地音频适配层。
它使用 BSP 解析到的 `esp_codec_dev`，通过 ES8311 扬声器与单麦克风辅助函数为
MusicPlayer、VideoPlayer、Recorder、SpecAnalyzer 和 Xiaozhi 提供音频。

本目录只持有媒体播放器回调和共享音频会话协调；显示、触摸、I2C、GPIO 与 SD 卡
支持仍由托管 BSP 提供。

## BSP 提供的音频引脚

应用通过已解析的 BSP 宏使用下列信号，不应重复定义开发板 GPIO 字面量：

| 信号 | GPIO |
| --- | ---: |
| I2S MCLK | 42 |
| I2S BCLK | 9 |
| I2S LRCK | 45 |
| ESP32-S3 输出到 ES8311 的数据 | 8 |
| ES8311 麦克风输入到 ESP32-S3 的数据 | 10 |
| 功率放大器使能 | 46 |

## 共享会话规则

ES8311、I2S 时钟和功率放大器使能构成一条共享硬件通路。音频应用在打开流
之前先获取本地会话；在关闭前停止全部工作任务和文件；只有编解码器关闭后才能释放
会话。这样可避免一个应用仍持有媒体或录音任务时，另一个应用重新配置编解码器。

支持的客户端为 MusicPlayer、VideoPlayer、Recorder、SpecAnalyzer 和 Xiaozhi。
构建只能验证 API 集成；在声明硬件结果前，仍需在目标板确认应用切换、扬声器和麦克风
行为。
