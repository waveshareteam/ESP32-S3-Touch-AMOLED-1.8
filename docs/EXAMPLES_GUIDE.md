# ESP-IDF Examples Guide

Start with serial-only checks, then move outward to board services, peripherals, and display UI.

## Suggested Path

1. [00_board_check](../examples/ESP-IDF-v5.5.1/00_board_check/) confirms the target, flash, PSRAM, BSP component, display geometry, I2C pins, SD pins, and audio capabilities.
2. [01_HowToCreateProject](../examples/ESP-IDF-v5.5.1/01_HowToCreateProject/) is the smallest practical managed-BSP project template for new applications.
3. [02_HelloWorld](../examples/ESP-IDF-v5.5.1/02_HelloWorld/), [03_nvs_counter](../examples/ESP-IDF-v5.5.1/03_nvs_counter/), and [04_freertos_tasks](../examples/ESP-IDF-v5.5.1/04_freertos_tasks/) cover ESP-IDF basics before board-specific peripherals.
4. [05_gpio_io](../examples/ESP-IDF-v5.5.1/05_gpio_io/) and [06_gpio_interrupt](../examples/ESP-IDF-v5.5.1/06_gpio_interrupt/) demonstrate configurable GPIO loopback and interrupt tests without reserving board-critical pins by default.
5. [08_i2c_tools](../examples/ESP-IDF-v5.5.1/08_i2c_tools/) scans the onboard I2C bus and is useful before debugging PMU, RTC, touch, or audio-control devices.
6. [09_sdmmc](../examples/ESP-IDF-v5.5.1/09_sdmmc/) mounts the microSD card through the managed BSP and performs FAT file operations.
7. [10_wifistation](../examples/ESP-IDF-v5.5.1/10_wifistation/) connects the ESP32-S3 to an access point with menuconfig-provided credentials.
8. [12_I2SCodec](../examples/ESP-IDF-v5.5.1/12_I2SCodec/) validates ES8311 speaker playback or microphone echo mode through BSP audio APIs.
9. [13_Displaycolorbar](../examples/ESP-IDF-v5.5.1/13_Displaycolorbar/) draws RGB565 test bars directly through the BSP display panel handle.
10. [14_lvgl_demo_v9](../examples/ESP-IDF-v5.5.1/14_lvgl_demo_v9/) starts the LVGL v9 widgets demo with BSP display, touch, LVGL port, and brightness control.

## Legacy Compatibility Examples

The older examples remain useful when validating board-specific components that are not fully represented by the managed-BSP learning path:

| Directory | Use when you need to check |
| --- | --- |
| [00_BSP_QuickStart](../examples/ESP-IDF-v5.5.1/00_BSP_QuickStart/) | Interactive LVGL touch dashboard, brightness control, and SD probe |
| [01_AXP2101](../examples/ESP-IDF-v5.5.1/01_AXP2101/) | PMU register bring-up |
| [02_PCF85063](../examples/ESP-IDF-v5.5.1/02_PCF85063/) | RTC timekeeping |
| [03_QMI8658](../examples/ESP-IDF-v5.5.1/03_QMI8658/) | IMU acceleration, gyro, temperature, and timestamps |
| [04_SD_MMC](../examples/ESP-IDF-v5.5.1/04_SD_MMC/) | Legacy SDMMC wiring and local-component behavior |
| [05_LVGL_WITH_RAM](../examples/ESP-IDF-v5.5.1/05_LVGL_WITH_RAM/) | Heavier LVGL display/touch path with PSRAM-backed buffers |
| [06_I2SCodec](../examples/ESP-IDF-v5.5.1/06_I2SCodec/) | Direct ES8311 music or echo flow |

## Component Policy

New ESP-IDF examples should use managed dependencies where possible. The preferred board dependency is:

```yaml
dependencies:
  idf: ">=5.5,<6.0"
  waveshare/esp32_s3_touch_amoled_1_8: "^1.0.0"
```

Checked-in component copies should only be used when an example intentionally demonstrates a local patch or a compatibility fallback.

## CI Coverage

The GitHub Actions workflow builds examples that are changed in a pull request and always includes `00_board_check` and `00_BSP_QuickStart` as smoke tests. See [CI.md](CI.md) for details.
