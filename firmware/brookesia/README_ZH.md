# ESP32-S3-Touch-AMOLED-1.8 Brookesia 固件

[English](README.md)

此目录是 ESP32-S3-Touch-AMOLED-1.8 的默认 **ESP-Brookesia Phone** 固件源码，
面向开发板的 368 × 448 AMOLED、16 MB Flash 和 8 MB PSRAM。它不是独立的 LVGL
外设仪表盘；共享应用会启动 Brookesia Phone 启动器，并安装适合该开发板的应用。

## 两个硬件 Profile

显示和触摸实现只由两个工程包装层选择；它们共享的 `components/` 与 `main/` 源码仅使用
两版 BSP 都提供的公共 API。

| Profile | 工程根目录 | 已发布 BSP | 交付的合并镜像 |
| --- | --- | --- | --- |
| Original | [`original/`](original/) | `waveshare/esp32_s3_touch_amoled_1_8` 1.1.4 | `firmware/ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260824.bin` |
| V2 | [`v2/`](v2/) | `waveshare/esp32_s3_touch_amoled_1_8` 2.0.3 | `firmware/ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260824.bin` |

必须直接构建其中一个包装工程。顶层目录有意不是 ESP-IDF 工程，因为单次 Component
Manager 解析不能同时选择两套 BSP 版本。

## 默认 Brookesia 应用

- Calculator、Draw 和 Crosshair（显示/触摸交互）
- Gravitysphere（QMI8658 IMU）
- Clock（使用 PCF85063A 同步的系统时间）
- SpecAnalyzer 与 Recorder（ES8311 单麦克风采集）
- Music、Gallery 和 Video（microSD 媒体）
- Settings（Wi-Fi、亮度、音频、存储、电源和板级诊断）
- Xiaozhi 原生 Wi-Fi 语音与文本应用

源码树保留了经过 Brookesia 尺寸适配的 SquareLine demo 组件作为可选演示，但默认启动器
不会安装它。Camera、按键、GNSS、Hosted Wi-Fi 以及 P4 专用媒体路径均未包含，因为它们
不对应本开发板已确认的硬件。

## 外设与组件边界

已发布 BSP 提供 AMOLED/触摸通路、I2C、microSD、显示亮度 API 以及 ES8311 板级连线。
本固件覆盖已确认的显示/触摸、Wi-Fi、microSD、ES8311 扬声器和麦克风、AXP2101 状态、
TCA9554 presence、QMI8658 IMU 与 PCF85063A RTC。

能使用在线组件的部分采用以下组件：

- 仅由包装层 manifest 选择的 `waveshare/esp32_s3_touch_amoled_1_8` 1.1.4 或 2.0.3
- `waveshare/qmi8658` 2.0.1；应用会先探测 0x6A 和 0x6B，再初始化 IMU
- `waveshare/pcf85063a` 2.0.0，用于 RTC
- BSP 解析到的 `espressif/esp_codec_dev`、`espressif/esp_new_jpeg`，以及本地音频/媒体
  组件声明的依赖

有两个本地适配层是有意保留的。`bsp_extra` 向音频应用提供一套板级 ES8311 全双工/session
接口。没有选用在线 AXP2101 驱动；`system_status` 仅只读 AXP2101 的 identity、status 和
电池百分比寄存器，并且只探测 0x20 的 TCA9554、不会改变其任何寄存器。详细维护边界请见
双语[组件说明](components/README_ZH.md)。

## SD 卡媒体目录

媒体应用将 FAT/FAT32 存储卡挂载到 `/sdcard`，可使用以下目录：

| 目录 | 内容 |
| --- | --- |
| `/sdcard/music` | MP3 或 WAV 音乐 |
| `/sdcard/photos` | 基线编码 JPG/JPEG 图片 |
| `/sdcard/video` | MJPEG/PCM AVI 视频 |
| `/sdcard/Waveshare/Recordings` | Recorder 输出的 WAV 文件 |
| `/sdcard/Waveshare/AIChats` | Xiaozhi 文本历史 |

建议使用中等分辨率、低帧率、PCM 音频的 MJPEG AVI。视频播放器不支持 H.264 或压缩音频
的 AVI。

## 构建与合并

使用 ESP-IDF **5.5.5**，并从仓库根目录执行。两个 profile 必须分别构建：

```text
idf.py -C firmware/brookesia/original set-target esp32s3
idf.py -C firmware/brookesia/original build

idf.py -C firmware/brookesia/v2 set-target esp32s3
idf.py -C firmware/brookesia/v2 build
```

必须使用各 profile 最终构建目录生成的 `flash_args` 合并整片镜像，不能猜测 offset：

```text
cd firmware/brookesia/original/build
python -m esptool --chip esp32s3 merge_bin \
  -o ../../../ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260824.bin @flash_args

cd ../../v2/build
python -m esptool --chip esp32s3 merge_bin \
  -o ../../../ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260824.bin @flash_args
```

这两份指定名称的镜像已用 ESP-IDF 5.5.5 在本地完成编译和打包；未进行烧录，也未主张
任何 HIL 结果。仍需在对应板卡修订上验证显示/触摸、音频、SD 卡、Wi-Fi、PMU、IMU、RTC
以及语音应用的实际行为。
