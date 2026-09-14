# Brookesia 固件组件

[English](README.md)

本目录包含 ESP32-S3-Touch-AMOLED-1.8 的自包含 ESP-Brookesia 应用和少量板级适配。
固件有两个工程根目录：Original profile 解析已发布的
`waveshare/esp32_s3_touch_amoled_1_8` BSP 1.1.4，V2 profile 解析 BSP 2.0.3。
公共组件仅使用两版共有的板级 API；两者均不替代对应 BSP。

## 应用

| 目录 | 功能 |
| --- | --- |
| [`brookesia_app_calculator/`](brookesia_app_calculator/) | 按运行时可视区域布局的计算器 |
| [`draw/`](draw/) | 全可视区域画板 |
| [`Crosshair/`](Crosshair/) | 显示与触摸对齐靶 |
| [`Gravitysphere/`](Gravitysphere/) | QMI8658 重力球应用 |
| [`Clock/`](Clock/) | RTC 时钟 |
| [`SpecAnalyzer/`](SpecAnalyzer/) | ES8311 单麦克风频谱分析仪 |
| [`MusicPlayer/`](MusicPlayer/) | SD 卡 MP3/WAV 播放器 |
| [`Gallery/`](Gallery/) | 按运行时解码边界处理的 SD 卡 JPEG 图库 |
| [`VideoPlayer/`](VideoPlayer/) | SD 卡 MJPEG/PCM AVI 播放器 |
| [`Recorder/`](Recorder/) | ES8311 单麦克风 WAV 录音机 |
| [`Settings/`](Settings/) | Wi-Fi、显示、音频、存储、电源和板级状态 |
| [`XiaozhiApp/`](XiaozhiApp/) | 原生 Wi-Fi 语音与文本应用 |
| [`brookesia_app_squareline_demo/`](brookesia_app_squareline_demo/) | 由 Brookesia 调整可视区域的 SquareLine UI 示例 |

本板没有已确认对应硬件，因此有意不包含 Camera、按键、GNSS、hosted Wi-Fi 和仅适用于
P4 的媒体路径。

## 公共服务与在线组件

| 目录或依赖 | 职责 |
| --- | --- |
| [`bsp_extra/`](bsp_extra/) | 面向媒体和语音应用的本地 ES8311 单编解码器音频/会话适配 |
| [`storage_service/`](storage_service/) | 媒体应用共享的 SD 卡挂载访问 |
| [`system_status/`](system_status/) | Wi-Fi，以及只读 AXP2101 与 TCA9554 presence 状态 |
| [`rtc_service/`](rtc_service/) | PCF85063A 读取与系统时钟同步 |
| [`chat_history/`](chat_history/) | 可选的 Xiaozhi 文本历史记录 |
| `waveshare/esp32_s3_touch_amoled_1_8` 1.1.4 / 2.0.3 | Original / V2 已发布 BSP profile：显示、触摸、I2C、SD 卡和 ES8311 板级连线 |
| `waveshare/qmi8658` ^2.0.0 | Gravitysphere 使用的 IMU 驱动；板端使用本地 0x6A/0x6B 探测 |
| `waveshare/pcf85063a` ^2.0.0 | `rtc_service` 使用的 RTC 访问 |
| `espressif/esp_new_jpeg` 1.* | Gallery 使用的 JPEG 解码 |
| `espressif/esp_codec_dev` | 由 BSP 解析，并通过 `bsp_extra` 使用的编解码器设备层 |

本工程没有选用在线 AXP2101 组件。本地 `system_status` 适配只读取 AXP 身份、状态和
电池寄存器，并仅探测 TCA9554、不会写入其寄存器。`bsp_extra` 也保持本地：Brookesia
应用需要的板级 ES8311 全双工/会话 API 没有可直接使用的已发布组件。

请构建任一完整 profile 以验证组件集成。构建成功不能证明实体板上的显示、触摸、音频、
传感器、电源或存储行为。
