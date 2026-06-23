# Getting Started

This guide uses the ESP-IDF examples for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

## Hardware

- Waveshare ESP32-S3-Touch-AMOLED-1.8 board
- USB cable with data support
- Optional microSD card for SD examples
- Optional speaker, headphones, or microphone path for audio examples

## ESP-IDF Version

Use ESP-IDF v5.5.x for the current ESP-IDF examples. The CI workflow builds with ESP-IDF v5.5.4 and target `esp32s3`.

## Build the BSP Quick Start

```bash
cd examples/ESP-IDF-v5.5.1/00_BSP_QuickStart
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the board serial port.

The example downloads the managed `waveshare/esp32_s3_touch_amoled_1_8` component through the IDF Component Manager, initializes the AMOLED panel and touch controller, and displays a small LVGL dashboard with a brightness slider and SD refresh button.

## Managed Components

For new ESP-IDF projects, prefer this dependency in `main/idf_component.yml`:

```yaml
dependencies:
  idf: ">=5.5,<6.0"
  waveshare/esp32_s3_touch_amoled_1_8: "^1.0.0"
```

Then include the BSP API:

```c
#include "bsp/esp-bsp.h"
```

Common entry points include `bsp_display_start()`, `bsp_display_lock()`, `bsp_display_brightness_set()`, `bsp_sdcard_mount()`, `bsp_audio_init()`, `bsp_audio_codec_speaker_init()`, and `bsp_audio_codec_microphone_init()`.

## Flash Size and PSRAM

Display and LVGL examples need PSRAM enabled and enough application partition space. The BSP quick-start example includes `sdkconfig.defaults` and `partitions.csv` tuned for this board.
