# ESP32-S3-Touch-AMOLED-1.8

Engineering examples and firmware assets for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

The board combines an ESP32-S3 module, a 1.8-inch 368 x 448 AMOLED touch display, SD card support, an ES8311 audio codec, onboard microphone input, an RTC, a PMU, and an IMU. The ESP-IDF examples now prefer the managed `waveshare/esp32_s3_touch_amoled_1_8` BSP component so new projects can start from the same online component used by the current board support package.

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
cd examples/ESP-IDF-v5.5.1/00_BSP_QuickStart
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

The quick-start example initializes the managed BSP component, starts LVGL, shows an interactive touch dashboard, controls AMOLED brightness with an LVGL slider, and probes the onboard SD card through the BSP API.

## Example Highlights

| Directory | Purpose | Notes |
| --- | --- | --- |
| [00_BSP_QuickStart](examples/ESP-IDF-v5.5.1/00_BSP_QuickStart/) | Online BSP component smoke test | Recommended starting point |
| [01_AXP2101](examples/ESP-IDF-v5.5.1/01_AXP2101/) | PMU bring-up | Legacy local-component style |
| [02_PCF85063](examples/ESP-IDF-v5.5.1/02_PCF85063/) | RTC bring-up | Legacy local-component style |
| [03_QMI8658](examples/ESP-IDF-v5.5.1/03_QMI8658/) | IMU readings | Uses SensorLib |
| [04_SD_MMC](examples/ESP-IDF-v5.5.1/04_SD_MMC/) | SD card mount and file I/O | Board SDMMC wiring |
| [05_LVGL_WITH_RAM](examples/ESP-IDF-v5.5.1/05_LVGL_WITH_RAM/) | LVGL display/touch UI | BSP display path plus legacy fallback |
| [06_I2SCodec](examples/ESP-IDF-v5.5.1/06_I2SCodec/) | ES8311 audio playback/echo | Direct I2S codec example |

See [examples/README.md](examples/README.md) and [docs/EXAMPLES_GUIDE.md](docs/EXAMPLES_GUIDE.md) for the recommended learning path.

## Continuous Integration

GitHub Actions builds selected ESP-IDF examples for `esp32s3` using ESP-IDF v5.5.4. The discovery script follows the pattern used by the ESP32-P4 reference platform while adapting it to this repository's existing `examples/ESP-IDF-v5.5.1` layout.

See [docs/CI.md](docs/CI.md) for workflow details.

## Support

- Product wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.8
- Component Registry: https://components.espressif.com/components/waveshare/esp32_s3_touch_amoled_1_8
- Issues: https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/issues

For hardware support, include the board revision, the example path, ESP-IDF version, serial log, and any external devices connected to the board.

## License

Unless noted otherwise in a subdirectory, source files in this repository are provided under the Apache License 2.0. Third-party libraries keep their own licenses and notices.
