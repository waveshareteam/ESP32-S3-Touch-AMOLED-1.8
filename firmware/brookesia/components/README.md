# Brookesia firmware components

[简体中文](README_ZH.md)

These are the self-contained ESP-Brookesia applications and the small board
adapters for ESP32-S3-Touch-AMOLED-1.8. The firmware has two project roots:
the Original profile resolves the published `waveshare/esp32_s3_touch_amoled_1_8`
BSP at 1.1.4, while the V2 profile resolves it at 2.0.3. The shared components
use only the common board APIs; neither profile is a replacement for its BSP.

## Applications

| Directory | Function |
| --- | --- |
| [`brookesia_app_calculator/`](brookesia_app_calculator/) | Calculator with a runtime visual-area layout |
| [`draw/`](draw/) | Full visual-area drawing panel |
| [`Gravitysphere/`](Gravitysphere/) | QMI8658 gravity-ball application |
| [`Clock/`](Clock/) | PCF85063A/SNTP-backed clock |
| [`SpecAnalyzer/`](SpecAnalyzer/) | ES8311 single-microphone spectrum analyzer (no title banner) |
| [`MusicPlayer/`](MusicPlayer/) | SPIFFS MP3/WAV player with an SD-card fallback |
| [`Gallery/`](Gallery/) | SD-card JPEG gallery with runtime decode bounds |
| [`VideoPlayer/`](VideoPlayer/) | SD-card MJPEG/PCM AVI player |
| [`Recorder/`](Recorder/) | ES8311 single-microphone WAV recorder |
| [`Settings/`](Settings/) | Wi-Fi, display, audio, storage, power, and board status |
| [`XiaozhiApp/`](XiaozhiApp/) | Native Wi-Fi voice and text application |
| [`brookesia_app_squareline_demo/`](brookesia_app_squareline_demo/) | Brookesia-resized SquareLine UI demonstration |

Camera, button, GNSS, hosted-Wi-Fi, and P4-only media paths are intentionally
not present: they have no confirmed matching hardware on this board.

## Shared services and online components

| Directory or dependency | Responsibility |
| --- | --- |
| [`bsp_extra/`](bsp_extra/) | Local ES8311 single-codec audio/session adapter for the media and voice applications |
| [`storage_service/`](storage_service/) | Shared SD-card mount access for media applications |
| [`system_status/`](system_status/) | Wi-Fi plus read-only AXP2101 and TCA9554-presence status |
| [`rtc_service/`](rtc_service/) | PCF85063A readout and system-clock synchronization |
| [`chat_history/`](chat_history/) | Optional Xiaozhi text history |
| `waveshare/esp32_s3_touch_amoled_1_8` 1.1.4 / 2.0.3 | Original / V2 published BSP profiles: display, touch, I2C, SD card, and ES8311 board wiring |
| `waveshare/qmi8658` ^2.0.0 | IMU driver used by Gravitysphere, with board-local 0x6A/0x6B probing |
| `waveshare/pcf85063a` ^2.0.0 | RTC access used by `rtc_service` |
| `espressif/esp_new_jpeg` 1.* | JPEG decoding used by Gallery |
| `espressif/esp_codec_dev` | Codec-device layer resolved by the BSP and used through `bsp_extra` |

No online AXP2101 component is selected. The local `system_status` adapter reads
only the AXP identity/status/battery registers and probes TCA9554 without
changing its registers. `bsp_extra` is also local because the board-specific
ES8311 full-duplex/session API needed by the Brookesia applications is not a
published component.

Build either complete profile to validate integration. A successful build does
not establish display, touch, audio, sensor, power, or storage behavior on a
physical board.
