# Examples

This directory contains ESP-IDF projects and Arduino sketches for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

## ESP-IDF

Each directory under [ESP-IDF-v5.5.1](ESP-IDF-v5.5.1/) is a standalone ESP-IDF project. Run ESP-IDF commands from inside the selected example directory unless the example README says otherwise.

```bash
cd examples/ESP-IDF-v5.5.1/00_BSP_QuickStart
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

| Directory | Purpose | Notes |
| --- | --- | --- |
| [00_BSP_QuickStart](ESP-IDF-v5.5.1/00_BSP_QuickStart/) | Managed BSP component quick start | Recommended first ESP-IDF example |
| [01_AXP2101](ESP-IDF-v5.5.1/01_AXP2101/) | PMU check | Legacy local-component example |
| [02_PCF85063](ESP-IDF-v5.5.1/02_PCF85063/) | RTC check | Legacy local-component example |
| [03_QMI8658](ESP-IDF-v5.5.1/03_QMI8658/) | IMU acceleration and gyro readings | Uses SensorLib |
| [04_SD_MMC](ESP-IDF-v5.5.1/04_SD_MMC/) | SD card mount and file I/O | Requires a formatted microSD card |
| [05_LVGL_WITH_RAM](ESP-IDF-v5.5.1/05_LVGL_WITH_RAM/) | AMOLED touch UI with LVGL | Uses PSRAM-backed draw buffers |
| [06_I2SCodec](ESP-IDF-v5.5.1/06_I2SCodec/) | ES8311 music/echo demo | Requires speaker or headphones |

The new BSP quick-start example uses the online `waveshare/esp32_s3_touch_amoled_1_8` component directly. Older examples are kept for hardware-specific bring-up and compatibility checks.

## Arduino

Arduino examples are kept in the versioned directories provided by the board package:

| Directory | Purpose |
| --- | --- |
| [Arduino-v3.3.5](Arduino-v3.3.5/) | Original Arduino example set |
| [Arduino-v3.3.5-v2](Arduino-v3.3.5-v2/) | Updated Arduino example set |

Use the Arduino-ESP32 core version named by the directory unless an individual sketch README says otherwise.

## Adding Examples

Keep new examples focused and easy to build:

- Put ESP-IDF examples under `ESP-IDF-v5.5.1/<number>_<name>/`.
- Include `README.md`, `CMakeLists.txt`, `main/`, and `sdkconfig.defaults`.
- Prefer managed components in `main/idf_component.yml` over checked-in component copies.
- Document required hardware, menuconfig options, expected serial output, and safety notes.
- Do not commit generated `build/`, `managed_components/`, dependency lock files, or local `sdkconfig` files unless intentionally curated.

See [../docs/EXAMPLES_GUIDE.md](../docs/EXAMPLES_GUIDE.md) for the suggested learning path.
