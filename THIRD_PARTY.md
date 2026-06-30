# Third-Party Software

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

## Firmware Binaries

Files under `Firmware/` are prebuilt factory firmware images for this board. See [Firmware/README.txt](Firmware/README.txt) for usage notes.

## License Summary

Unless noted otherwise in a file or subdirectory, repository source and documentation are provided under the Apache License 2.0. Third-party libraries, generated assets, and firmware binaries may have additional or different license terms. Review the notices in the relevant directory before redistribution.
