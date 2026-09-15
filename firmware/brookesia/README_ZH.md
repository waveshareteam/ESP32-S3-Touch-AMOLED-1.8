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
| Original | [`original/`](original/) | `waveshare/esp32_s3_touch_amoled_1_8` 1.1.4 | `firmware/ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin` |
| V2 | [`v2/`](v2/) | `waveshare/esp32_s3_touch_amoled_1_8` 2.0.3 | `firmware/ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin` |

必须直接构建其中一个包装工程。顶层目录有意不是 ESP-IDF 工程，因为单次 Component
Manager 解析不能同时选择两套 BSP 版本。

## 默认 Brookesia 应用

- Calculator 和 Draw（显示/触摸交互）
- Gravitysphere（QMI8658 IMU）：小球在整个屏幕范围内运动，场地为圆角矩形
- Clock（PCF85063A + SNTP）：RTC 无有效时间时用固件编译时间戳播种，Wi-Fi 连接后由
  SNTP 校正
- SpecAnalyzer 与 Recorder（ES8311 单麦克风采集）：频谱分析仪只绘制频谱画布，不再
  叠加标题条
- Music 使用内置 SPIFFS 曲库，SD 卡作为回退；Gallery 和 Video 仍为 microSD 媒体
- Settings（Wi-Fi、亮度、音频、存储、电源和板级诊断）：以沉浸式全屏页面打开，隐藏
  状态栏
- Xiaozhi 原生 Wi-Fi 语音与文本应用，提示音从 SPIFFS 读取

显示/触摸对齐靶 Crosshair 已从产品中移除。

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
  -o ../../../ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin @flash_args

cd ../../v2/build
python -m esptool --chip esp32s3 merge_bin \
  -o ../../../ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin @flash_args
```

这两份指定名称的镜像已用 ESP-IDF 5.5.5 在本地完成编译和打包。

## 启动关键配置

以下配置直接影响能否正常工作，修改前请参考
[docs/FIRMWARE_ZH.md](../../docs/FIRMWARE_ZH.md#启动关键配置)：

- [`main/main.cpp`](main/main.cpp) 中的 `LVGL_PORT_INIT_CONFIG()` 将 LVGL 工作
  任务栈提升到 20 KB。esp_lvgl_port 默认的 7168 字节只适用于普通 LVGL 示例，
  Brookesia 桌面会溢出该栈并触发 `A stack overflow in task taskLVGL` panic。
- [`sdkconfig.defaults`](sdkconfig.defaults) 中的
  `CONFIG_BSP_DISPLAY_LVGL_BUF_HEIGHT=10`。BSP 把 LVGL 绘制缓冲区放在 PSRAM，
  因此每次刷新都需要一块同等大小的、DMA 可访问的**内部** RAM 临时缓冲区。BSP
  默认的 100 行意味着每次刷新申请 73,600 字节，在 Wi-Fi 与桌面程序占用内部 RAM
  之后必然失败并丢帧。
- `app_main()` 中提前调用 `brookesia::system_status::init_wifi_stack()`，避免桌面
  程序把内部堆打碎。`esp_wifi_init()` 的静态 RX 缓冲池需要一整块连续内部内存，
  分配失败时会静默退化为 2 个 RX 缓冲区。
- [`sdkconfig.defaults`](sdkconfig.defaults) 中的 `CONFIG_BSP_SPIFFS_MAX_FILES=5`。
  Xiaozhi 字体、曲库和提示音现在共用同一个 SPIFFS 分区，且音乐播放器会为整首曲目
  保持一个文件句柄。
- [`sdkconfig.defaults`](sdkconfig.defaults) 中的
  `CONFIG_XIAOZHI_SYNC_SYSTEM_TIME_FROM_SERVER=n`。系统时钟由 RTC、编译时间戳播种和
  SNTP 共同负责；否则 Xiaozhi 组件会用 `server_time` 加上它自己的时区偏移覆盖系统
  时钟，那是另一套约定，会在 SNTP 应用 `CST-8` 之后造成双重偏移。
- [SPIFFS 资源暂存](v2/main/CMakeLists.txt)在**每次**构建都会执行，而不再依赖时间戳。
  以时间戳把关的拷贝会静默保留过期的暂存目录，从而打包出过期的 SPIFFS 镜像且不报
  任何错误——设备上就是没有内置媒体。若怀疑镜像缺少媒体，先看控制台打印的
  `Partition size: ... used:`。

## 硬件验证

`ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin` 已作为完整 `0x0` 镜像烧录
到 V2 板卡，并通过 USB-Serial-JTAG 控制台验证：桌面安装 11 个应用后启动到
`Brookesia firmware is ready`，LVGL 工作任务不再溢出，面板刷新不再出现 SPI DMA
分配失败，Wi-Fi 驱动成功初始化全部 8 个静态 RX 缓冲区，AXP2101 状态监视器正常
启动，观测期间未出现 panic 或重启。

同一次验证也覆盖了媒体与时间改动：挂载后的 SPIFFS 报告
`used: 5679377` / `total: 5775761`，说明曲库与 Xiaozhi 提示音确实已进入烧录镜像；
`rtc_service` 打印 `System time synchronized from RTC: ...`，且该时间在多次复位之间
持续推进，因此时钟应用、状态栏和 Settings ▸ Power 都能拿到有效时间。由于没有可用
的 Wi-Fi 网络，SNTP 未能在本次验证中触发；它会在 `IP_EVENT_STA_GOT_IP` 时启动并
应用 `CST-8`。

`ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin` 已作为完整 `0x0` 镜像烧录到 V1
板卡并人工试用：桌面可启动、各应用可打开，且**板载喇叭能正常出声**。该修订剩余待验证
的仅是本次未覆盖的外设部分（SD 卡媒体、Wi-Fi 关联、语音操作）。

### 音频无声不一定是固件问题

排查"没有声音"时要留意：整条播放链路是共用的，很容易被误判为软件问题。在早前使用的
V2 板卡上，链路已逐段验证通过却依然无声：`bsp_extra_player_play_file()` 能从
`/spiffs/music` 打开 MP3，I2S 被重配为 44100 Hz 立体声，ES8311 报告未静音
（`REG31=00`）且 DAC 音量正常（`REG32=0xB2`），`BSP_POWER_AMP_IO` 的功放使能位读回为
有效，直接把 1 kHz 正弦波写入 `bsp_extra_i2s_write()` 也按精确字节数完成。而同一份
镜像在 V1 板卡上播放正常，因此那块 V2 板卡的**喇叭本身损坏**。可把上述顺序当作软件侧
的自检清单：如果全部通过，故障就在喇叭、功放或板级连线上，而不在本固件。
