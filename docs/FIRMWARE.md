# Firmware Artifacts

This repository has two different firmware artifact types.

## Factory Binaries

`Firmware/` contains prebuilt factory or recovery images supplied with the product. These binaries are checked in intentionally so users can recover or restore a board without requiring a source build. Their corresponding source and build instructions are not included in this repository yet and may be added in a later update.

Factory binaries are not source projects:

- They are not rebuilt by GitHub Actions.
- They are not repackaged as CI artifacts.
- They should not be treated as generated output from this repository.

Keep factory-binary documentation focused on the board revision, intended use, and flashing or recovery notes.

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