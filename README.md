# ESP32-S3-Touch-AMOLED-1.8

Engineering examples and firmware assets for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

The board combines an ESP32-S3 module, a 1.8-inch 368 x 448 AMOLED touch display, SD card support, an ES8311 audio codec, onboard microphone input, an RTC, a PMU, and an IMU. The ESP-IDF examples prefer the managed `waveshare/esp32_s3_touch_amoled_1_8` BSP component so new projects can start from the same online component used by the current board support package.

## Repository Layout

| Path | Contents |
| --- | --- |
| [examples/](examples/README.md) | Arduino sketches and ESP-IDF projects |
| [examples/esp-idf/](examples/esp-idf/) | ESP-IDF examples for ESP32-S3 |
| [examples/Arduino-v3.3.5/](examples/Arduino-v3.3.5/) | Arduino examples for the original board package |
| [examples/Arduino-v3.3.5-v2/](examples/Arduino-v3.3.5-v2/) | Arduino examples for the newer board package |
| [Firmware/](Firmware/README.txt) | Prebuilt factory firmware images |
| [docs/](docs/GETTING_STARTED.md) | Setup, example, and CI documentation |

## Recommended ESP-IDF Flow

Use ESP-IDF v5.5.x and the ESP32-S3 target:

```bash
cd examples/esp-idf/00_board_check
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

`00_board_check` verifies chip, flash, PSRAM, BSP capabilities, display size, I2C pins, SD pins, and audio support through the managed BSP component. For a richer first UI test, use `00_bsp_quickstart`, which starts LVGL, shows an interactive touch dashboard, controls AMOLED brightness, and probes the onboard SD card through the BSP API.

## ESP-IDF Example Highlights

The ESP-IDF example set follows the ESP32-P4 platform learning order where it applies to this board, with ESP32-S3-specific changes and lower_snake_case directory names.

| Directory | Purpose | Notes |
| --- | --- | --- |
| [00_board_check](examples/esp-idf/00_board_check/) | Board and BSP capability check | Default CI smoke test |
| [00_bsp_quickstart](examples/esp-idf/00_bsp_quickstart/) | Interactive BSP/LVGL quick start | Default CI UI smoke test |
| [01_project_template](examples/esp-idf/01_project_template/) | Minimal managed-BSP project template | Practical new-project starting point |
| [02_hello_world](examples/esp-idf/02_hello_world/) | ESP-IDF hello world | Chip info and restart flow |
| [03_nvs_counter](examples/esp-idf/03_nvs_counter/) | Persistent NVS counter | Demonstrates flash-backed state |
| [04_freertos_tasks](examples/esp-idf/04_freertos_tasks/) | FreeRTOS tasks and queue | Small producer/consumer demo |
| [05_gpio_io](examples/esp-idf/05_gpio_io/) | GPIO input/output loopback | User-selected safe GPIOs |
| [06_gpio_interrupt](examples/esp-idf/06_gpio_interrupt/) | GPIO interrupt handling | User-selected safe input GPIO |
| [08_i2c_tools](examples/esp-idf/08_i2c_tools/) | I2C bus scanner | Defaults to the board I2C bus |
| [09_sdmmc](examples/esp-idf/09_sdmmc/) | SDMMC/FAT file operations | Uses BSP SD card mounting |
| [10_wifi_station](examples/esp-idf/10_wifi_station/) | Wi-Fi station connection | Configure credentials in menuconfig |
| [12_i2s_codec](examples/esp-idf/12_i2s_codec/) | ES8311 audio playback or echo | Uses BSP audio APIs |
| [13_display_colorbar](examples/esp-idf/13_display_colorbar/) | AMOLED color-bar rendering | Uses BSP display panel handle |
| [14_lvgl_demo_v9](examples/esp-idf/14_lvgl_demo_v9/) | LVGL v9 widgets demo | Display, touch, LVGL, brightness |

Hardware diagnostics that still require local low-level components are kept in the `90_` range so they do not compete with the managed BSP learning path:

| Directory | Purpose | Notes |
| --- | --- | --- |
| [90_axp2101_pmu](examples/esp-idf/90_axp2101_pmu/) | AXP2101 PMU bring-up | Legacy local-component diagnostic |
| [91_pcf85063_rtc](examples/esp-idf/91_pcf85063_rtc/) | PCF85063 RTC bring-up | Legacy local-component diagnostic |
| [92_qmi8658_imu](examples/esp-idf/92_qmi8658_imu/) | QMI8658 IMU readings | Uses SensorLib |

The older `04_SD_MMC`, `05_LVGL_WITH_RAM`, and direct `06_I2SCodec` examples were removed because their supported workflows are covered by `09_sdmmc`, `00_bsp_quickstart`, `12_i2s_codec`, and `14_lvgl_demo_v9` through the online BSP component.

See [examples/README.md](examples/README.md) and [docs/EXAMPLES_GUIDE.md](docs/EXAMPLES_GUIDE.md) for the recommended learning path.

## Continuous Integration

GitHub Actions builds selected ESP-IDF examples for `esp32s3` using ESP-IDF v5.5.4. Pull requests build changed examples and always include the managed BSP smoke tests `00_board_check` and `00_bsp_quickstart`; manual runs can build a named example or the full ESP-IDF example set.

See [docs/CI.md](docs/CI.md) for workflow details.

## Support

- Product wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.8
- Component Registry: https://components.espressif.com/components/waveshare/esp32_s3_touch_amoled_1_8
- Issues: https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/issues

For hardware support, include the board revision, the example path, ESP-IDF version, serial log, and any external devices connected to the board.

## License

Unless noted otherwise in a subdirectory, source files in this repository are provided under the Apache License 2.0. Third-party libraries keep their own licenses and notices.