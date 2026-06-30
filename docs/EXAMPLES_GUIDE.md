# ESP-IDF Examples Guide

Start with serial-only checks, then move outward to board services, peripherals, and display UI.

## Suggested Path

1. [00_board_check](../examples/esp-idf/00_board_check/) confirms the target, flash, PSRAM, BSP component, display geometry, I2C pins, SD pins, and audio capabilities.
2. [00_bsp_quickstart](../examples/esp-idf/00_bsp_quickstart/) starts the managed BSP display, touch, brightness control, LVGL dashboard, and SD probe.
3. [01_project_template](../examples/esp-idf/01_project_template/) is the smallest practical managed-BSP project template for new applications.
4. [02_hello_world](../examples/esp-idf/02_hello_world/), [03_nvs_counter](../examples/esp-idf/03_nvs_counter/), and [04_freertos_tasks](../examples/esp-idf/04_freertos_tasks/) cover ESP-IDF basics before board-specific peripherals.
5. [05_gpio_io](../examples/esp-idf/05_gpio_io/) and [06_gpio_interrupt](../examples/esp-idf/06_gpio_interrupt/) demonstrate configurable GPIO loopback and interrupt tests without reserving board-critical pins by default.
6. [08_i2c_tools](../examples/esp-idf/08_i2c_tools/) scans the onboard I2C bus and is useful before debugging PMU, RTC, touch, or audio-control devices.
7. [09_sdmmc](../examples/esp-idf/09_sdmmc/) mounts the microSD card through the managed BSP and performs FAT file operations.
8. [10_wifi_station](../examples/esp-idf/10_wifi_station/) connects the ESP32-S3 to an access point with menuconfig-provided credentials.
9. [12_i2s_codec](../examples/esp-idf/12_i2s_codec/) validates ES8311 speaker playback through BSP audio APIs.
10. [13_display_colorbar](../examples/esp-idf/13_display_colorbar/) draws RGB565 test bars directly through the BSP display panel handle.
11. [14_lvgl_demo_v9](../examples/esp-idf/14_lvgl_demo_v9/) starts the LVGL v9 widgets demo with BSP display, touch, LVGL port, and brightness control.

## Hardware Diagnostics

Board-specific diagnostics that are not fully represented by the managed-BSP learning path are kept in the `90_` range:

| Directory | Use when you need to check |
| --- | --- |
| [90_axp2101_pmu](../examples/esp-idf/90_axp2101_pmu/) | AXP2101 PMU register bring-up with the local diagnostic component |
| [91_pcf85063_rtc](../examples/esp-idf/91_pcf85063_rtc/) | PCF85063A RTC timekeeping through `waveshare/pcf85063a` |
| [92_qmi8658_imu](../examples/esp-idf/92_qmi8658_imu/) | QMI8658 acceleration, gyro, temperature, and timestamps through `waveshare/qmi8658` |

The former local `04_SD_MMC`, `05_LVGL_WITH_RAM`, and direct `06_I2SCodec` demos were removed because the managed BSP examples cover those workflows with less duplicated driver code.

## Component Policy

New ESP-IDF examples should use managed dependencies where possible. The preferred board dependency is:

```yaml
dependencies:
  idf: ">=5.5,<6.0"
  waveshare/esp32_s3_touch_amoled_1_8: "^2.0.1"
```

The ESP Component Registry lists `2.0.1` as the latest `waveshare/esp32_s3_touch_amoled_1_8` release for `esp32s3` as of 2026-06-24. Checked-in component copies should only be used when an example intentionally demonstrates a local patch or a compatibility fallback. The RTC and IMU diagnostics use `waveshare/pcf85063a` `^2.0.0` and `waveshare/qmi8658` `^2.0.0`.

## CI Coverage

The GitHub Actions workflow builds ESP-IDF examples that are changed in a pull request. Workflow or discovery-script changes build the full ESP-IDF example set. See [CI.md](CI.md) for details.

## Customer-Facing Checklist

Before publishing or changing an example, make sure its README includes the required hardware, build commands, configuration options, expected result, and any safety notes. Keep public documentation free of local machine paths, private network paths, usernames, generated build directories, and temporary troubleshooting notes.
