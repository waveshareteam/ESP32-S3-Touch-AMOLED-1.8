# 固件工件

[English](FIRMWARE.md)

本仓库有两类不同的固件工件。

## 默认固件

[`firmware/brookesia/`](../firmware/brookesia/) 包含本开发板默认 ESP-Brookesia Phone 固件的源码。Original 与 V2 使用独立工程包装层选择对应已发布 BSP 的显示和触摸实现，并共享 Phone 应用与板级应用。

检入的合并镜像由各自最终 ESP-IDF 构建生成的 `flash_args` 制作，包含 bootloader、分区表、OTA 数据、语音模型、应用和 SPIFFS 镜像：

- `firmware/ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin`
- `firmware/ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin`

精确的配置、构建和合并命令见[默认固件构建说明](../firmware/brookesia/README_ZH.md)。这些源码工程和整片镜像与下述示例 CI 打包流程分开维护。

### 启动关键配置

默认固件对一组配置敏感，每一项都对应一个在真实硬件上观测到的故障，放宽任何一项都会让故障复现：

- **LVGL 工作任务栈。** `bsp_display_start()` 使用 esp_lvgl_port 默认的 7168 字节，只适用于普通 LVGL 示例。Brookesia 桌面在构建与重绘应用网格时会走一条深得多的样式与事件链，从而溢出该栈。panic 打印为 `A stack overflow in task`，且任务名是乱码——因为溢出顺着栈底写进了相邻的任务控制块，FreeRTOS 才在随后发现。因此 `main/main.cpp` 改为通过 `bsp_display_start_with_config()` 启动面板，并用本地 `LVGL_PORT_INIT_CONFIG()` 申请 20 KB，与其他 Waveshare Brookesia 固件保持一致。
- **LVGL 绘制缓冲区高度。** `BSP_DISPLAY_LVGL_BUF_HEIGHT` 决定 LVGL 绘制缓冲区为 `H_RES x 高度` 像素，而 BSP 将该缓冲区放在 PSRAM。PSRAM 缓冲区无法直接交给面板，因此 `esp_lcd_panel_io_spi` 会让 SPI 主机为每次刷新把数据经一块同等大小的、DMA 可访问的**内部**缓冲区中转。在 BSP 默认的 100 行下，这意味着每次刷新申请 73,600 字节内部内存；当 Wi-Fi 与桌面程序占用内部 RAM 后该申请必然失败，帧被丢弃，屏幕只显示一部分。`sdkconfig.defaults` 将高度固定为 10 行（每次刷新 7,360 字节），可稳定满足。
- **Wi-Fi 协议栈初始化顺序。** `esp_wifi_init()` 的静态 RX 缓冲池需要一整块连续的、DMA 可访问的内部内存。如果桌面程序已经把这个堆打碎，驱动会静默退化为 2 个 RX 缓冲区，`esp_wifi_init()` 以 `ESP_ERR_NO_MEM` 失败，状态监视器和 Xiaozhi 语音应用都会失效。`app_main()` 在应用初始化之前调用 `brookesia::system_status::init_wifi_stack()`；该辅助函数是幂等的，因此状态监视器和 Xiaozhi 之后的重复调用依然成功。
- **SPIFFS 文件数与资源暂存。** Xiaozhi 字体、曲库与提示音共用 `storage` 分区（约占可用 SPIFFS 空间的 98%），且音乐播放器会为整首曲目保持一个文件句柄，因此 `CONFIG_BSP_SPIFFS_MAX_FILES` 设为 5。`v2/main/CMakeLists.txt` 与 `original/main/CMakeLists.txt` 中的暂存拷贝刻意在每次构建都执行：以时间戳把关的拷贝会静默保留过期的暂存目录，从而打包出过期的 SPIFFS 镜像且不报任何错误，最终只表现为设备上没有媒体。
- **时间保持。** PCF85063A 在没有有效时间时用固件编译时间戳播种；`rtc_service_sync_from_system_time()` 会在系统时钟与 RTC 相差超过数秒时回写，使校正后的时间能跨重启保留。SNTP（`ntp.aliyun.com`，时区 `CST-8`）在 `IP_EVENT_STA_GOT_IP` 时启动；`CONFIG_XIAOZHI_SYNC_SYSTEM_TIME_FROM_SERVER` 被停用，因为它的 `server_time` 处理使用另一套约定，会造成时钟双重偏移。

### 默认固件 HIL 状态

`firmware/ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin` 已作为完整 `0x0` 镜像写入 V2 板卡，并通过 USB-Serial-JTAG 控制台观测：桌面安装 11 个应用后启动到 `Brookesia firmware is ready`，无 panic、无重启；面板刷新不再出现 SPI DMA 分配失败；Wi-Fi 驱动报告全部 8 个静态 RX 缓冲区；AXP2101 状态监视器正常启动；挂载后的 SPIFFS 报告 `used: 5679377` / `total: 5775761`，说明内置媒体确实在镜像内。由于没有可用网络，SNTP 未能在本次验证中触发。

`firmware/ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin` 已作为完整 `0x0` 镜像烧录到 V1 板卡并人工试用：桌面可启动、各应用可打开，板载喇叭能正常出声。SD 卡媒体、Wi-Fi 关联与语音操作仍待按修订验证。

判断"没有声音"时要谨慎归因。在一块 V2 板卡上，整条软件链路验证全部通过——MP3 从 SPIFFS 打开、I2S 重配为 44100 Hz 立体声、ES8311 未静音且 DAC 音量正常、`BSP_POWER_AMP_IO` 使能位有效、直接写 `bsp_extra_i2s_write()` 的原始音调字节数精确——但依然无声；同一份镜像在 V1 板卡上可正常播放，故该 V2 板卡的喇叭损坏。请先按软件链路自检，再检查喇叭、功放与连线。

## 源码构建的 CI 工件

GitHub Actions 构建 `examples/esp-idf/` 下的 ESP-IDF 示例以及 `examples/arduino/examples/` 和 `examples/arduino-v2/examples/` 下的第一方 Arduino 草图。成功后工作流通过 `releases/package_firmware.py` 将构建输出打包为可刷写归档。

每个 CI 固件归档包含：

- 带 schema 版本、框架、目标、`project_path`、git SHA、`timestamp_utc`、波特率、刷写命令和二进制偏移量的 `manifest.json`
- `flash.sh`
- `flash.bat`
- 带 esptool 命令参数的 `flash_args.txt`
- `bin/` 下由清单引用的引导加载程序、分区表、应用、合并镜像或其他二进制文件

从工作流运行工件下载这些归档。CI zip 名称包含框架、示例、框架版本、目标和短提交标识；它们是 CI 验证输出，并非源文件，不应加入仓库。

## 本地发布检查

本地发布打包时，先构建目标，再从仓库根目录运行 `releases/package_firmware.py`。默认输出目录为 `releases/dist/`；CI 使用 `release-artifacts/`。生成或下载的固件包会忽略在 `release-artifacts/`、`releases/dist/` 和 `releases/downloads/` 中。
