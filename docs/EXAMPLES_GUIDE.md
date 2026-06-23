# ESP-IDF Examples Guide

Start with the managed BSP example, then move outward to individual peripherals.

## Suggested Path

1. [00_BSP_QuickStart](../examples/ESP-IDF-v5.5.1/00_BSP_QuickStart/) validates the online BSP component, display, touch input, LVGL task, brightness slider, heap status, and SD card probe path.
2. [04_SD_MMC](../examples/ESP-IDF-v5.5.1/04_SD_MMC/) focuses on SD card mounting and file operations.
3. [05_LVGL_WITH_RAM](../examples/ESP-IDF-v5.5.1/05_LVGL_WITH_RAM/) runs a heavier LVGL demo using PSRAM-backed draw buffers.
4. [06_I2SCodec](../examples/ESP-IDF-v5.5.1/06_I2SCodec/) validates ES8311 playback or echo mode.
5. [03_QMI8658](../examples/ESP-IDF-v5.5.1/03_QMI8658/) reads accelerometer, gyroscope, temperature, and timestamp data.
6. [01_AXP2101](../examples/ESP-IDF-v5.5.1/01_AXP2101/) and [02_PCF85063](../examples/ESP-IDF-v5.5.1/02_PCF85063/) remain useful for low-level PMU and RTC checks.

## Component Policy

New ESP-IDF examples should use managed dependencies where possible. The preferred board dependency is:

```yaml
waveshare/esp32_s3_touch_amoled_1_8: "^1.0.0"
```

Checked-in component copies should only be used when an example intentionally demonstrates a local patch or a compatibility fallback.

## CI Coverage

The GitHub Actions workflow builds examples that are changed in a pull request and always includes the managed BSP quick-start example as a smoke test. See [CI.md](CI.md) for details.
