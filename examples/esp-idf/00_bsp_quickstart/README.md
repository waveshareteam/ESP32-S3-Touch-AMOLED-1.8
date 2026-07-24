# BSP Quick Start

This example is the recommended ESP-IDF starting point for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

It uses the online `waveshare/esp32_s3_touch_amoled_1_8` BSP component instead of checked-in driver copies.

## Features

- Initializes the CO5300 AMOLED panel and CST816 touch controller through the BSP.
- Starts LVGL and renders a small touch dashboard.
- Sets display brightness through an LVGL slider and the BSP API.
- Shows heap and PSRAM information.
- Probes the onboard SD card through `bsp_sdcard_mount()` at startup and from the on-screen refresh button, then writes `/sdcard/bsp.txt` when a card is present.

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for the board.

## Notes

- A missing SD card is reported on screen and in the serial log, but it does not fail the example.
- The short marker filename remains compatible with FAT volumes when long filename support is disabled.
- The dependency is declared in [main/idf_component.yml](main/idf_component.yml). During build, IDF Component Manager downloads the BSP into `managed_components`.
- The custom partition table leaves room for LVGL, BSP, and managed component code.
