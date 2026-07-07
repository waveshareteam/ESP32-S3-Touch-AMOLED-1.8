# Firmware Artifacts

This repository has two different firmware artifact types.

## Factory Binaries

`Firmware/` contains prebuilt factory or recovery images supplied with the product. These binaries are checked in intentionally so users can recover or restore a board without rebuilding private factory firmware.

Factory binaries are not source projects:

- They are not rebuilt by GitHub Actions.
- They are not uploaded again as CI artifacts.
- They should not be treated as generated output from this repository.

Keep factory-binary documentation focused on the board revision, intended use, and flashing or recovery notes.

## Source-Built CI Artifacts

ESP-IDF examples under `examples/esp-idf/` are built by GitHub Actions. After a successful build, the workflow packages the build output into a flashable archive by reading `flasher_args.json` from the ESP-IDF build directory.

Each CI firmware archive contains:

- `manifest.json`
- `flasher_args.json`
- `flash.sh`
- `flash.bat`
- `bin/` with the bootloader, partition table, app, and any other binaries referenced by `flasher_args.json`

Download these archives from the workflow run artifacts. They are validation outputs from CI, not source files, and should stay out of the repository.

## Ignored Generated Outputs

The repository ignores generated or downloaded firmware packages in:

- `release-artifacts/`
- `releases/dist/`
- `releases/downloads/`

Use those locations for temporary local packaging checks or downloaded CI artifacts.
