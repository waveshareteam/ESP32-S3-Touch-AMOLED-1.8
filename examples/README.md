# Examples

This directory contains ESP-IDF projects and Arduino sketches for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

## ESP-IDF

Each directory under [esp-idf](esp-idf/) is a standalone ESP-IDF project. Run ESP-IDF commands from inside the selected example directory unless the example README says otherwise.

```bash
cd examples/esp-idf/00_board_check
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

## BSP Component Examples

These examples are adapted from the ESP32-P4 platform ESP-IDF example order, with hardware-specific changes for ESP32-S3-Touch-AMOLED-1.8 and the managed `waveshare/esp32_s3_touch_amoled_1_8` component.

| Directory | Purpose | Notes |
| --- | --- | --- |
| [00_board_check](esp-idf/00_board_check/) | Board capability and pin summary | Good first serial-only check |
| [01_HowToCreateProject](esp-idf/01_HowToCreateProject/) | Minimal managed-BSP project template | Declares the online BSP dependency |
| [02_HelloWorld](esp-idf/02_HelloWorld/) | ESP-IDF hello world | Chip info, countdown, restart |
| [03_nvs_counter](esp-idf/03_nvs_counter/) | NVS persistent boot counter | Demonstrates flash-backed state |
| [04_freertos_tasks](esp-idf/04_freertos_tasks/) | FreeRTOS task and queue demo | Producer/consumer logging |
| [05_gpio_io](esp-idf/05_gpio_io/) | GPIO input/output loopback | Choose safe exposed pins in menuconfig |
| [06_gpio_interrupt](esp-idf/06_gpio_interrupt/) | GPIO interrupt handling | Choose safe exposed input pin in menuconfig |
| [08_i2c_tools](esp-idf/08_i2c_tools/) | I2C scan tool | Defaults to the onboard I2C bus |
| [09_sdmmc](esp-idf/09_sdmmc/) | SDMMC/FAT example | Mounts through the BSP SD API |
| [10_wifistation](esp-idf/10_wifistation/) | Wi-Fi station example | Configure credentials in menuconfig |
| [12_I2SCodec](esp-idf/12_I2SCodec/) | ES8311 playback or microphone echo | Uses BSP audio codec handles |
| [13_Displaycolorbar](esp-idf/13_Displaycolorbar/) | Display color-bar rendering | Draws RGB565 bars through the BSP panel |
| [14_lvgl_demo_v9](esp-idf/14_lvgl_demo_v9/) | LVGL v9 widgets demo | Uses BSP display, touch, LVGL, brightness |

## Legacy Hardware Examples

The original ESP-IDF examples are kept for hardware-specific bring-up and compatibility checks. They may include local component copies or older structure, but remain useful when validating individual peripherals.

| Directory | Purpose | Notes |
| --- | --- | --- |
| [00_BSP_QuickStart](esp-idf/00_BSP_QuickStart/) | Managed BSP component quick start | Interactive LVGL smoke test |
| [01_AXP2101](esp-idf/01_AXP2101/) | PMU check | Legacy local-component example |
| [02_PCF85063](esp-idf/02_PCF85063/) | RTC check | Legacy local-component example |
| [03_QMI8658](esp-idf/03_QMI8658/) | IMU acceleration and gyro readings | Uses SensorLib |
| [04_SD_MMC](esp-idf/04_SD_MMC/) | SD card mount and file I/O | Requires a formatted microSD card |
| [05_LVGL_WITH_RAM](esp-idf/05_LVGL_WITH_RAM/) | AMOLED touch UI with LVGL | Uses PSRAM-backed draw buffers |
| [06_I2SCodec](esp-idf/06_I2SCodec/) | ES8311 music/echo demo | Requires speaker or headphones |

## Arduino

Arduino examples are kept in the versioned directories provided by the board package:

| Directory | Purpose |
| --- | --- |
| [Arduino-v3.3.5](Arduino-v3.3.5/) | Original Arduino example set |
| [Arduino-v3.3.5-v2](Arduino-v3.3.5-v2/) | Updated Arduino example set |

Use the Arduino-ESP32 core version named by the directory unless an individual sketch README says otherwise.

## Adding Examples

Keep new examples focused and easy to build:

- Put ESP-IDF examples under `esp-idf/<number>_<name>/`.
- Include `README.md`, `CMakeLists.txt`, `main/`, and `sdkconfig.defaults`.
- Prefer managed components in `main/idf_component.yml` over checked-in component copies.
- Document required hardware, menuconfig options, expected serial output, and safety notes.
- Do not commit generated `build/`, `managed_components/`, dependency lock files, or local `sdkconfig` files unless intentionally curated.

See [../docs/EXAMPLES_GUIDE.md](../docs/EXAMPLES_GUIDE.md) for the suggested learning path.
