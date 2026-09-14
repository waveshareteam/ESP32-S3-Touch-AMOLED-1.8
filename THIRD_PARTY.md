# Third-Party Software

简体中文请参见 [THIRD_PARTY_ZH.md](THIRD_PARTY_ZH.md)。

This repository includes source examples, firmware assets, managed component references, and bundled third-party libraries.

## Managed ESP-IDF Components

Most ESP-IDF examples use components resolved by the ESP-IDF Component Manager, including:

- `waveshare/esp32_s3_touch_amoled_1_8`
- `waveshare/pcf85063a`
- `waveshare/qmi8658`

The exact versions are declared in each example's `main/idf_component.yml` and resolved during build.

## Bundled Libraries

The Arduino example trees include bundled libraries such as LVGL, Adafruit BusIO, SensorLib, GFX Library for Arduino, and board-specific helper libraries. These libraries keep their upstream licenses in their own directories.

The `examples/esp-idf/90_axp2101_pmu` diagnostic includes a local XPowersLib port for low-level PMU bring-up. XPowersLib files retain their upstream MIT license notices.

## Default Firmware

The ESP-Brookesia Phone source and generated Original/V2 combined images under `firmware/` use the dependencies documented in [firmware/brookesia/README.md](firmware/brookesia/README.md). Review the resolved managed-component licenses before redistributing a binary.

## License Summary

Unless noted otherwise in a file or subdirectory, repository source and documentation are provided under the Apache License 2.0. Third-party libraries, generated assets, and firmware binaries may have additional or different license terms. Review the notices in the relevant directory before redistribution.
