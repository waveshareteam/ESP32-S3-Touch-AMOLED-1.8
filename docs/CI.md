# Continuous Integration

This repository uses the `Build Examples` GitHub Actions workflow to discover examples dynamically, build them, and upload flashable source-built firmware artifacts.

## Discovery

The workflow uses `scripts/discover_examples.py` for both framework surfaces:

- ESP-IDF projects are discovered from `examples/esp-idf/*/CMakeLists.txt` with a `main/` directory.
- Arduino sketches are discovered from first-party sketch directories under `examples/arduino/examples/` and `examples/arduino-v2/examples/`.
- Arduino sketches inside bundled libraries are intentionally excluded from product CI.

`workflow_dispatch` accepts `all`, an example directory name, or a repo-relative example path. Maintainers can run the full matrix or a single example.

Pull request and branch push runs compare the event base and head revisions, then build only affected first-party examples. A bundled-library change rebuilds all first-party sketches in the matching Arduino root. Changes to the workflow, discovery script, or firmware packager rebuild both surfaces; shared ESP-IDF configuration changes rebuild the ESP-IDF surface. Tag pushes and events without a usable base revision fall back to the full matrix.

Manual runs can build the full matrix or narrow it by passing an example name, a parent example directory such as `08_LVGL_Animation`, or a repo-relative path to `target`.

## Matrix

Current CI matrix:

- ESP-IDF `v5.5.5` and `v6.0.2`, target `esp32s3`.
- Arduino-ESP32 core `3.3.11`, FQBN `esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB`, using bundled libraries from the matching `examples/arduino/libraries` or `examples/arduino-v2/libraries` directory.

The selected framework versions were resolved from upstream stable releases on 2026-07-15. Do not replace them with beta, release-candidate, preview, or nightly tags unless the repository intentionally opts into that coverage.

## Firmware Artifacts

Each successful ESP-IDF and Arduino matrix build uploads a flashable firmware artifact. CI packages build outputs through `releases/package_firmware.py`.

Each generated archive contains:

- `manifest.json` with schema version, framework, framework version, target, `project_path`, git SHA, `timestamp_utc`, baud rate, flash command, and binary offsets.
- `flash.sh` and `flash.bat` helper scripts.
- `flash_args.txt` with the esptool command arguments.
- `bin/` with the firmware binaries referenced by the manifest.

Download the artifact zip from the workflow run, extract it, then run `flash.sh` or `flash.bat` with the board serial port. CI zip names include the framework, example, framework version, target, and short commit identifier; the outer GitHub artifact names remain stable for filtering and scripted downloads.

Generated archives are workflow artifacts only. Do not commit generated files from `release-artifacts/`, `releases/dist/`, or `releases/downloads/`.

Checked-in files under `Firmware/` are factory or recovery binaries. They are documented assets, not source-build outputs, and they do not trigger source-build packaging.

## Hardware Validation Boundary

CI validates source compatibility, compilation, and firmware packaging. The Arduino FQBN fixes the ESP32-S3 target, 16 MB flash size, and application partition layout used by these examples. It does not prove runtime pin, PSRAM, USB, display, touch, audio, or sensor correctness.

A board-level schematic or equivalent hardware reference is not included in this repository yet. Pin-level validation remains pending until that reference is added; hardware-facing changes should be checked against the official board documentation and the managed BSP before release.

## Local Script Checks

Discovery can be checked without building firmware:

```bash
python scripts/discover_examples.py --surface esp-idf --selector 00_board_check
python scripts/discover_examples.py --surface esp-idf --selector all
python scripts/discover_examples.py --surface arduino --selector all
python scripts/discover_examples.py --surface arduino --selector 08_LVGL_Animation
```

The packaging helper expects an existing ESP-IDF or Arduino build output and is normally exercised inside CI after the framework build finishes.

If an example requires hardware, credentials, or an upstream component that is not yet compatible with a selected framework version, document the exclusion here before excluding it from CI.