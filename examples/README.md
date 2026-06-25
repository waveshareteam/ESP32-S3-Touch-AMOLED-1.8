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
| [00_bsp_quickstart](esp-idf/00_bsp_quickstart/) | Interactive BSP/LVGL quick start | Display, touch, brightness, SD probe |
| [01_project_template](esp-idf/01_project_template/) | Minimal managed-BSP project template | Declares the online BSP dependency |
| [02_hello_world](esp-idf/02_hello_world/) | ESP-IDF hello world | Chip info, countdown, restart |
| [03_nvs_counter](esp-idf/03_nvs_counter/) | NVS persistent boot counter | Demonstrates flash-backed state |
| [04_freertos_tasks](esp-idf/04_freertos_tasks/) | FreeRTOS task and queue demo | Producer/consumer logging |
| [05_gpio_io](esp-idf/05_gpio_io/) | GPIO input/output loopback | Choose safe exposed pins in menuconfig |
| [06_gpio_interrupt](esp-idf/06_gpio_interrupt/) | GPIO interrupt handling | Choose safe exposed input pin in menuconfig |
| [08_i2c_tools](esp-idf/08_i2c_tools/) | I2C scan tool | Defaults to the onboard I2C bus |
| [09_sdmmc](esp-idf/09_sdmmc/) | SDMMC/FAT example | Mounts through the BSP SD API |
| [10_wifi_station](esp-idf/10_wifi_station/) | Wi-Fi station example | Configure credentials in menuconfig |
| [12_i2s_codec](esp-idf/12_i2s_codec/) | ES8311 playback or microphone echo | Uses BSP audio codec handles |
| [13_display_colorbar](esp-idf/13_display_colorbar/) | Display color-bar rendering | Draws RGB565 bars through the BSP panel |
| [14_lvgl_demo_v9](esp-idf/14_lvgl_demo_v9/) | LVGL v9 widgets demo | Uses BSP display, touch, LVGL, brightness |

## Hardware Diagnostics

These examples are kept for board-specific bring-up beyond the main BSP path. They are grouped in the `90_` range so the main path stays focused on the managed online BSP; RTC and IMU checks use managed Waveshare sensor components.

| Directory | Purpose | Notes |
| --- | --- | --- |
| [90_axp2101_pmu](esp-idf/90_axp2101_pmu/) | PMU check | Local-component diagnostic |
| [91_pcf85063_rtc](esp-idf/91_pcf85063_rtc/) | RTC check | Uses `waveshare/pcf85063a` |
| [92_qmi8658_imu](esp-idf/92_qmi8658_imu/) | IMU acceleration and gyro readings | Uses `waveshare/qmi8658` |

The previous local-component SD, LVGL-with-RAM, and direct I2S codec demos were removed to avoid duplicate workflows. Use `09_sdmmc`, `00_bsp_quickstart`, `12_i2s_codec`, and `14_lvgl_demo_v9` for those paths.

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
