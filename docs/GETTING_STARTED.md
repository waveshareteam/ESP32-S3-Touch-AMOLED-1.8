# Getting Started

This guide uses the ESP-IDF examples for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

## Hardware

- Waveshare ESP32-S3-Touch-AMOLED-1.8 board
- USB cable with data support
- Optional microSD card for SD examples
- Optional speaker or headphones for audio examples
- Optional jumper wires for GPIO loopback or interrupt examples

## ESP-IDF Version

Use ESP-IDF v5.5.x for the current ESP-IDF examples. The CI workflow builds with ESP-IDF v5.5.4 and target `esp32s3`.

The examples use ESP-IDF Component Manager to download managed dependencies during the first build. Make sure the build host has internet access or a configured component cache.

## Build the Board Check

```bash
cd examples/esp-idf/00_board_check
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the board serial port.

The example downloads the managed `waveshare/esp32_s3_touch_amoled_1_8` component through the IDF Component Manager and prints chip, flash, PSRAM, BSP capability, display, I2C, SD, and audio information. It is the fastest first check because it does not require a display UI loop, Wi-Fi credentials, SD card, or audio accessory.

For a visual touch test, run:

```bash
cd examples/esp-idf/00_bsp_quickstart
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

That example initializes the AMOLED panel and touch controller, then displays an LVGL dashboard with a brightness slider and SD refresh button.

## Managed Components

For new ESP-IDF projects, prefer this dependency in `main/idf_component.yml`:

```yaml
dependencies:
  idf: ">=5.5,<6.0"
  waveshare/esp32_s3_touch_amoled_1_8: "^2.0.1"
```

Then include the BSP API:

```c
#include "bsp/esp-bsp.h"
```

Common entry points include `bsp_display_start()`, `bsp_display_lock()`, `bsp_display_brightness_set()`, `bsp_sdcard_mount()`, `bsp_audio_init()`, and `bsp_audio_codec_speaker_init()`.

Avoid copying the generated `managed_components/`, `dependencies.lock`, or local `sdkconfig` files into a new application unless you intentionally want to pin that generated state.

## Example Order

The BSP-component examples under `examples/esp-idf` follow the ESP32-P4 platform learning sequence where it applies to this ESP32-S3 board: board check, BSP quick start, project template, hello world, NVS, FreeRTOS, GPIO, I2C, SD card, Wi-Fi, audio, display, and LVGL.

Use `90_axp2101_pmu`, `91_pcf85063_rtc`, and `92_qmi8658_imu` only when you need low-level PMU, RTC, or IMU diagnostics. The RTC and IMU examples use managed Waveshare sensor components, while the PMU example remains a local diagnostic.

See [EXAMPLES_GUIDE.md](EXAMPLES_GUIDE.md) for the full path and hardware notes.

## Flash Size and PSRAM

Display and LVGL examples need PSRAM enabled and enough application partition space. The display examples include `sdkconfig.defaults` and, where needed, `partitions.csv` tuned for this board.

## Troubleshooting

- Run `00_board_check` before debugging a higher-level example.
- Use `08_i2c_tools` when PMU, RTC, touch, IMU, or codec control devices are not detected.
- For audio output, connect a speaker or headphones to the board audio output and start with a low volume setting.
- For SD card examples, use a FAT-formatted microSD card and check the serial log for mount errors.
