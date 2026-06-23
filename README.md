# ESP32-S3-Touch-AMOLED-1.8

Engineering examples and firmware assets for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

The board combines an ESP32-S3 module, a 1.8-inch 368 x 448 AMOLED touch display, SD card support, an ES8311 audio codec, onboard microphone input, an RTC, a PMU, and an IMU. The ESP-IDF examples prefer the managed `waveshare/esp32_s3_touch_amoled_1_8` BSP component so new projects can start from the same online component used by the current board support package.

## Repository Layout

| Path | Contents |
| --- | --- |
| [examples/](examples/README.md) | Arduino sketches and ESP-IDF projects |
| [examples/ESP-IDF-v5.5.1/](examples/ESP-IDF-v5.5.1/) | ESP-IDF examples for ESP32-S3 |
| [examples/Arduino-v3.3.5/](examples/Arduino-v3.3.5/) | Arduino examples for the original board package |
| [examples/Arduino-v3.3.5-v2/](examples/Arduino-v3.3.5-v2/) | Arduino examples for the newer board package |
| [Firmware/](Firmware/README.txt) | Prebuilt factory firmware images |
| [docs/](docs/GETTING_STARTED.md) | Setup, example, and CI documentation |

## Recommended ESP-IDF Flow

Use ESP-IDF v5.5.x and the ESP32-S3 target:

```bash
cd examples/ESP-IDF-v5.5.1/00_board_check
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

`00_board_check` verifies chip, flash, PSRAM, BSP capabilities, display size, I2C pins, SD pins, and audio support through the managed BSP component. For a richer first UI test, use `00_BSP_QuickStart`, which starts LVGL, shows an interactive touch dashboard, controls AMOLED brightness, and probes the onboard SD card through the BSP API.

## ESP-IDF Example Highlights

The ESP-IDF example set now includes a BSP-component learning path adapted from the ESP32-P4 platform examples and adjusted for ESP32-S3-Touch-AMOLED-1.8 hardware.

| Directory | Purpose | Notes |
| --- | --- | --- |
| [00_board_check](examples/ESP-IDF-v5.5.1/00_board_check/) | Board and BSP capability check | Default CI smoke test |
| [01_HowToCreateProject](examples/ESP-IDF-v5.5.1/01_HowToCreateProject/) | Minimal managed-BSP project template | Practical new-project starting point |
| [02_HelloWorld](examples/ESP-IDF-v5.5.1/02_HelloWorld/) | ESP-IDF hello world | Chip info and restart flow |
| [03_nvs_counter](examples/ESP-IDF-v5.5.1/03_nvs_counter/) | Persistent NVS counter | Demonstrates flash-backed state |
| [04_freertos_tasks](examples/ESP-IDF-v5.5.1/04_freertos_tasks/) | FreeRTOS tasks and queue | Small producer/consumer demo |
| [05_gpio_io](examples/ESP-IDF-v5.5.1/05_gpio_io/) | GPIO input/output loopback | User-selected safe GPIOs |
| [06_gpio_interrupt](examples/ESP-IDF-v5.5.1/06_gpio_interrupt/) | GPIO interrupt handling | User-selected safe input GPIO |
| [08_i2c_tools](examples/ESP-IDF-v5.5.1/08_i2c_tools/) | I2C bus scanner | Defaults to the board I2C bus |
| [09_sdmmc](examples/ESP-IDF-v5.5.1/09_sdmmc/) | SDMMC/FAT file operations | Uses BSP SD card mounting |
| [10_wifistation](examples/ESP-IDF-v5.5.1/10_wifistation/) | Wi-Fi station connection | Configure credentials in menuconfig |
| [12_I2SCodec](examples/ESP-IDF-v5.5.1/12_I2SCodec/) | ES8311 audio playback or echo | Uses BSP audio APIs |
| [13_Displaycolorbar](examples/ESP-IDF-v5.5.1/13_Displaycolorbar/) | AMOLED color-bar rendering | Uses BSP display panel handle |
| [14_lvgl_demo_v9](examples/ESP-IDF-v5.5.1/14_lvgl_demo_v9/) | LVGL v9 widgets demo | Display, touch, LVGL, brightness |

Legacy low-level examples remain available for PMU, RTC, IMU, SD, LVGL, and audio compatibility checks:

| Directory | Purpose | Notes |
| --- | --- | --- |
| [00_BSP_QuickStart](examples/ESP-IDF-v5.5.1/00_BSP_QuickStart/) | Online BSP component quick start | Interactive LVGL smoke test |
| [01_AXP2101](examples/ESP-IDF-v5.5.1/01_AXP2101/) | PMU bring-up | Legacy local-component style |
| [02_PCF85063](examples/ESP-IDF-v5.5.1/02_PCF85063/) | RTC bring-up | Legacy local-component style |
| [03_QMI8658](examples/ESP-IDF-v5.5.1/03_QMI8658/) | IMU readings | Uses SensorLib |
| [04_SD_MMC](examples/ESP-IDF-v5.5.1/04_SD_MMC/) | SD card mount and file I/O | Board SDMMC wiring |
| [05_LVGL_WITH_RAM](examples/ESP-IDF-v5.5.1/05_LVGL_WITH_RAM/) | LVGL display/touch UI | BSP display path plus legacy fallback |
| [06_I2SCodec](examples/ESP-IDF-v5.5.1/06_I2SCodec/) | ES8311 audio playback/echo | Direct I2S codec example |

See [examples/README.md](examples/README.md) and [docs/EXAMPLES_GUIDE.md](docs/EXAMPLES_GUIDE.md) for the recommended learning path.

## Continuous Integration

GitHub Actions builds selected ESP-IDF examples for `esp32s3` using ESP-IDF v5.5.4. Pull requests build changed examples and always include the managed BSP smoke tests `00_board_check` and `00_BSP_QuickStart`; manual runs can build a named example or the full ESP-IDF example set.

See [docs/CI.md](docs/CI.md) for workflow details.

## Support

- Product wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.8
- Component Registry: https://components.espressif.com/components/waveshare/esp32_s3_touch_amoled_1_8
- Issues: https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/issues

For hardware support, include the board revision, the example path, ESP-IDF version, serial log, and any external devices connected to the board.

## License

Unless noted otherwise in a subdirectory, source files in this repository are provided under the Apache License 2.0. Third-party libraries keep their own licenses and notices.
