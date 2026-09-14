# ESP32-S3-Touch-AMOLED-1.8 Brookesia firmware

[简体中文](README_ZH.md)

This directory contains the default **ESP-Brookesia Phone** firmware source for
ESP32-S3-Touch-AMOLED-1.8. It targets the board's 368 × 448 AMOLED display,
16 MB flash, and 8 MB PSRAM. It is not a standalone LVGL peripheral dashboard:
the shared application starts the Brookesia Phone launcher and installs the
board-appropriate applications.

## Profiles

The display and touch implementations are selected only by the two wrapper
projects. Their shared `components/` and `main/` source intentionally use the
common BSP API.

| Profile | Project root | Published BSP | Delivered combined image |
| --- | --- | --- | --- |
| Original | [`original/`](original/) | `waveshare/esp32_s3_touch_amoled_1_8` 1.1.4 | `firmware/ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260824.bin` |
| V2 | [`v2/`](v2/) | `waveshare/esp32_s3_touch_amoled_1_8` 2.0.3 | `firmware/ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260824.bin` |

Build either wrapper directly; the top-level directory is deliberately not an
ESP-IDF project because one Component Manager resolution cannot select both
BSP versions.

## Included Brookesia applications

- Calculator, Draw, and Crosshair (display/touch interaction)
- Gravitysphere (QMI8658 IMU)
- Clock (PCF85063A-backed system time)
- SpecAnalyzer and Recorder (ES8311 single-microphone capture)
- Music, Gallery, and Video (microSD media)
- Settings (Wi-Fi, brightness, audio, storage, power, and board diagnostics)
- Xiaozhi native Wi-Fi voice and text application

The SquareLine demo component is retained in the source tree as an optional
Brookesia-resized demo, but is not installed by the default launcher. Camera,
buttons, GNSS, hosted Wi-Fi, and P4-specific media paths are intentionally not
included because they do not match confirmed hardware on this board.

## Peripheral and component boundary

The published BSP supplies the AMOLED/touch path, I2C, microSD, display
brightness API, and ES8311 board wiring. The firmware covers the confirmed
display/touch, Wi-Fi, microSD, ES8311 speaker and microphone, AXP2101 status,
TCA9554 presence, QMI8658 IMU, and PCF85063A RTC surfaces.

Online components are used where available:

- `waveshare/esp32_s3_touch_amoled_1_8` 1.1.4 or 2.0.3, selected solely by the
  wrapper manifest
- `waveshare/qmi8658` 2.0.1 for the IMU; the application probes 0x6A and 0x6B
  before initialization
- `waveshare/pcf85063a` 2.0.0 for RTC access
- BSP-resolved `espressif/esp_codec_dev` and `espressif/esp_new_jpeg`, plus
  the audio/media dependencies declared by the local components

Two local adapters remain intentionally. `bsp_extra` presents one
board-specific ES8311 full-duplex/session interface to the audio applications.
`system_status` has no selected online AXP2101 driver; it only reads AXP2101
identity, status, and battery-percent registers, and only probes the TCA9554
at 0x20 without changing its registers. See the bilingual
[component guide](components/README.md) for the detailed ownership boundary.

## SD-card media layout

Media applications mount a FAT/FAT32 card at `/sdcard` and use these optional
directories:

| Directory | Content |
| --- | --- |
| `/sdcard/music` | MP3 or WAV music |
| `/sdcard/photos` | Baseline JPG/JPEG images |
| `/sdcard/video` | MJPEG/PCM AVI video |
| `/sdcard/Waveshare/Recordings` | Recorder WAV output |
| `/sdcard/Waveshare/AIChats` | Xiaozhi text history |

Use modest-resolution, low-frame-rate MJPEG/PCM AVI files. The video player
does not support H.264 or compressed AVI audio.

## Build and combine

Use ESP-IDF **5.5.5**. Run the commands from the repository root. Build each
profile independently:

```text
idf.py -C firmware/brookesia/original set-target esp32s3
idf.py -C firmware/brookesia/original build

idf.py -C firmware/brookesia/v2 set-target esp32s3
idf.py -C firmware/brookesia/v2 build
```

Generate each whole-flash combined image from that profile's final generated
`flash_args`; do not substitute guessed offsets:

```text
cd firmware/brookesia/original/build
python -m esptool --chip esp32s3 merge_bin \
  -o ../../../ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260824.bin @flash_args

cd ../../v2/build
python -m esptool --chip esp32s3 merge_bin \
  -o ../../../ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260824.bin @flash_args
```

The two named images were compiled and packaged locally with ESP-IDF 5.5.5.
They were not flashed, and no hardware-in-the-loop result is claimed. Physical
display/touch, audio, SD-card, Wi-Fi, PMU, IMU, RTC, and voice-operation
validation remains required on the corresponding board revision.
