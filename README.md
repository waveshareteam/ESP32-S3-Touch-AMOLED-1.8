<div align="center">
  <h1>ESP32-S3-Touch-AMOLED-1.8</h1>
  <p><strong>ESP32-S3 1.8-inch 368 x 448 QSPI AMOLED touch development board</strong></p>
  <p>
    <a href="https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/actions/workflows/examples.yml"><img alt="Build Examples" src="https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/actions/workflows/examples.yml/badge.svg"></a>
    <a href="https://components.espressif.com/components/waveshare/esp32_s3_touch_amoled_1_8"><img alt="Component Registry" src="https://components.espressif.com/components/waveshare/esp32_s3_touch_amoled_1_8/badge.svg"></a>
    <a href="LICENSE"><img alt="License" src="https://img.shields.io/github/license/waveshareteam/ESP32-S3-Touch-AMOLED-1.8"></a>
  </p>
  <p>
    <a href="README_ZH.md">中文</a> ·
    <a href="https://www.waveshare.com/esp32-s3-touch-amoled-1.8.htm">🌐 Product Page</a> ·
    <a href="https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-1.8">📚 Documentation</a> ·
    <a href="https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/releases">📦 Firmware Releases</a> ·
    <a href="examples/esp-idf/">🧩 ESP-IDF Examples</a> ·
    <a href="examples/README.md#arduino">🔧 Arduino Examples</a>
  </p>
  <img src="./assets/ESP32-S3-Touch-AMOLED-1.8-hero.webp" alt="Waveshare ESP32-S3-Touch-AMOLED-1.8 development board" width="800">
</div>

---

## ✨ Overview

This repository provides first-party ESP-IDF projects, two Arduino example
sets, source-built firmware packages, factory recovery images, and development
documentation for the Waveshare ESP32-S3-Touch-AMOLED-1.8.

The board combines an ESP32-S3 with a high-resolution AMOLED display,
capacitive touch, power management, RTC, motion sensing, microSD storage, and
audio interfaces. The repository supports both display and touch revisions:
the original SH8601/FT3168 board and the newer CO5300/CST820 board.

## 🖥️ Hardware Overview

| Feature | Device / interface |
| --- | --- |
| MCU | ESP32-S3 |
| Display | 1.8-inch 368 x 448 QSPI AMOLED |
| Original display / touch | SH8601 with FT3168 capacitive touch |
| V2 display / touch | CO5300 with CST820 capacitive touch |
| Power management | AXP2101 PMU |
| Real-time clock | PCF85063A RTC |
| Motion sensor | QMI8658 six-axis IMU |
| Audio | ES8311 codec, onboard microphone input, and speaker amplifier |
| Storage | microSD over SDMMC |
| Board support | Managed component: `waveshare/esp32_s3_touch_amoled_1_8` |

> [!NOTE]
> The V2 board is fitted with a CST820 touch controller. Some bundled Arduino
> sources retain <code>Arduino_CST816x</code> family or API identifiers for
> compatible driver code; those identifiers do not describe the fitted V2
> touch chip.

> [!IMPORTANT]
> A board-level schematic is not included in this repository yet. CI validates
> source compatibility and firmware packaging, but it does not replace
> hardware validation of pins, PSRAM, USB, display, touch, audio, or sensors.

## 📦 Firmware Artifacts

Each successful CI build is packaged as a flashable firmware archive and
uploaded to the
[Build Examples workflow](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/actions/workflows/examples.yml).

Versioned firmware bundles are available from
[GitHub Releases](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/releases)
when a release is published. Release contents may vary by version; use the CI
artifacts for per-example builds from recent commits.

The easiest way to download and extract artifacts from a completed run is:

```bash
python3 releases/download_artifacts.py --run-id <run-id> --clean
```

After extraction, open the matching folder under
`releases/downloads/run-<run-id>/` and flash the board with:

```bash
./flash.sh /dev/ttyACM0
```

On Windows:

```bat
flash.bat COMx
```

Each package contains `manifest.json`, `flash_args.txt`, platform-specific
flash helpers, and the required binaries under `bin/`. Install esptool with
`python -m pip install esptool` when needed.

Factory and recovery images under [Firmware](Firmware/) are checked-in product
assets, not CI build outputs. See
[Firmware Artifacts](docs/FIRMWARE.md) and [Release Tools](releases/README.md)
for the source-built and factory firmware boundaries.

## 🧪 Examples

### ESP-IDF

| Example | Focus |
| --- | --- |
| [00_board_check](examples/esp-idf/00_board_check/) | Serial board, memory, BSP, and capability check |
| [00_bsp_quickstart](examples/esp-idf/00_bsp_quickstart/) | Interactive display, touch, brightness, and SD quick start |
| [01_project_template](examples/esp-idf/01_project_template/) | Minimal managed-BSP project template |
| [02_hello_world](examples/esp-idf/02_hello_world/) | ESP-IDF chip information and restart flow |
| [03_nvs_counter](examples/esp-idf/03_nvs_counter/) | Persistent NVS boot counter |
| [04_freertos_tasks](examples/esp-idf/04_freertos_tasks/) | FreeRTOS tasks, queue, and producer/consumer flow |
| [05_gpio_io](examples/esp-idf/05_gpio_io/) | Configurable GPIO loopback |
| [06_gpio_interrupt](examples/esp-idf/06_gpio_interrupt/) | GPIO interrupt handling |
| [08_i2c_tools](examples/esp-idf/08_i2c_tools/) | Onboard I2C bus scanner |
| [09_sdmmc](examples/esp-idf/09_sdmmc/) | SDMMC and FAT file operations |
| [10_wifi_station](examples/esp-idf/10_wifi_station/) | Wi-Fi station connection |
| [12_i2s_codec](examples/esp-idf/12_i2s_codec/) | ES8311 speaker playback |
| [13_display_colorbar](examples/esp-idf/13_display_colorbar/) | AMOLED RGB565 color-bar rendering |
| [14_lvgl_demo_v9](examples/esp-idf/14_lvgl_demo_v9/) | LVGL 9 display and touch demo |
| [90_axp2101_pmu](examples/esp-idf/90_axp2101_pmu/) | Low-level AXP2101 diagnostic |
| [91_pcf85063_rtc](examples/esp-idf/91_pcf85063_rtc/) | PCF85063A RTC diagnostic |
| [92_qmi8658_imu](examples/esp-idf/92_qmi8658_imu/) | QMI8658 acceleration and gyro readings |

Start with `00_board_check` for a serial-first check, then use
`00_bsp_quickstart` for the first interactive display and touch test.

### Arduino

Arduino examples are split into two first-party sets with matching bundled
libraries:

| Set | Display / touch | First-party sketches |
| --- | --- | ---: |
| [Original](examples/arduino/examples/) | SH8601 / FT3168 | 16 |
| [V2](examples/arduino-v2/examples/) | CO5300 / CST820 | 10 |

The sets cover display bring-up, drawing, RTC, LVGL, IMU, SD, and ES8311 audio.
The original set additionally includes Wi-Fi analysis, clock, AXP2101
telemetry, animation, and a SquareLine-style LVGL project.

Bundled libraries live under
[`examples/arduino/libraries`](examples/arduino/libraries/) and
[`examples/arduino-v2/libraries`](examples/arduino-v2/libraries/). Their own
upstream library examples are intentionally excluded from the product CI
matrix.

See [Examples](examples/README.md) and the
[Examples Guide](docs/EXAMPLES_GUIDE.md) for the recommended learning path.

## 🛠️ Supported Toolchains

| Surface | Version | Firmware builds |
| --- | --- | ---: |
| ESP-IDF | `v5.5.5` | 17 |
| ESP-IDF | `v6.0.2` | 17 |
| Arduino-ESP32 original | `3.3.11` | 16 |
| Arduino-ESP32 V2 | `3.3.11` | 10 |

These stable versions were reverified from official releases on 2026-08-10.

The
[Build Examples workflow](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/actions/workflows/examples.yml)
runs two discovery jobs and up to 60 firmware build jobs. Pull request and
branch push runs build affected first-party examples; workflow, discovery, and
release-packaging changes rebuild both framework surfaces. Tag pushes and
manual `all` runs build the full matrix.

Every successful build uploads a source-built firmware artifact. See
[Continuous Integration](docs/CI.md) for change detection, dispatch inputs,
board options, artifact behavior, and hardware validation boundaries.

## 🗂️ Repository Layout

| Path | Purpose |
| --- | --- |
| [`examples/esp-idf/`](examples/esp-idf/) | First-party ESP-IDF projects |
| [`examples/arduino/`](examples/arduino/) | Original Arduino sketches and bundled libraries |
| [`examples/arduino-v2/`](examples/arduino-v2/) | V2 Arduino sketches and bundled libraries |
| [`Firmware/`](Firmware/) | Factory flashing and recovery binaries |
| [`releases/`](releases/) | Firmware packaging and artifact download tools |
| [`config/`](config/) | Shared ESP-IDF configuration notes and overlays |
| [`docs/`](docs/) | Setup, examples, CI, structure, and firmware documentation |

## 📚 Documentation

- [Product Page](https://www.waveshare.com/esp32-s3-touch-amoled-1.8.htm)
- [Official Product Documentation](https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-1.8)
- [Getting Started](docs/GETTING_STARTED.md)
- [Examples Guide](docs/EXAMPLES_GUIDE.md)
- [Repository Structure](docs/PROJECT_STRUCTURE.md)
- [Continuous Integration](docs/CI.md)
- [Firmware Artifacts](docs/FIRMWARE.md)
- [Release Tools](releases/README.md)
- [中文说明](README_ZH.md)

## 🤝 Support and Contributions

Contributions and reproducible issue reports are welcome. Include the board
revision, example path, framework version, reproduction steps, expected
behavior, actual behavior, and relevant serial or build logs.

- [Contributing Guide](CONTRIBUTING.md)
- [Support](SUPPORT.md)
- [Security Policy](SECURITY.md)
- [Code of Conduct](CODE_OF_CONDUCT.md)
- [Third-party Notices](THIRD_PARTY.md)
- [Open an Issue](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.8/issues/new/choose)

## 📄 License

Unless noted otherwise in a subdirectory, this repository is licensed under
the Apache License 2.0. Third-party libraries keep their own licenses and
notices. See [LICENSE](LICENSE) and [Third-party Notices](THIRD_PARTY.md).
