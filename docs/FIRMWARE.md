# Firmware Artifacts

中文请参见 [FIRMWARE_ZH.md](FIRMWARE_ZH.md)。

This repository has two different firmware artifact types.

## Default Firmware

[`firmware/brookesia/`](../firmware/brookesia/) contains the source for the board's default ESP-Brookesia Phone firmware. Separate Original and V2 project wrappers select the display and touch implementation supplied by the matching published BSP line while sharing the Phone application and board-specific applications.

The checked-in combined images are generated from each final ESP-IDF build's own `flash_args` and include the bootloader, partition table, OTA data, speech models, application, and SPIFFS image:

- `firmware/ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260824.bin`
- `firmware/ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260824.bin`

See the [default-firmware build instructions](../firmware/brookesia/README.md) for the exact profile, build, and merge commands. These source projects and whole-flash images are maintained separately from the example CI packaging described below.

## Source-Built CI Artifacts

ESP-IDF examples under `examples/esp-idf/` and first-party Arduino sketches under `examples/arduino/examples/` and `examples/arduino-v2/examples/` are built by GitHub Actions. After a successful build, the workflow packages the build output into a flashable archive with `releases/package_firmware.py`.

Each CI firmware archive contains:

- `manifest.json` with schema version, framework, target, `project_path`, git SHA, `timestamp_utc`, baud rate, flash command, and binary offsets
- `flash.sh`
- `flash.bat`
- `flash_args.txt` with the esptool command arguments
- `bin/` with the bootloader, partition table, app, merged image, or other binaries referenced by the manifest

Download these archives from the workflow run artifacts. CI zip names include the framework, example, framework version, target, and short commit identifier. They are validation outputs from CI, not source files, and should stay out of the repository.

## Local Release Checks

For local release packaging, build the target first and run `releases/package_firmware.py` from the repository root. The default output directory is `releases/dist/`; CI uses `release-artifacts/`.

Generated or downloaded firmware packages are ignored in:

- `release-artifacts/`
- `releases/dist/`
- `releases/downloads/`
