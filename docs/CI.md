# Continuous Integration

中文请参见 [CI_ZH.md](CI_ZH.md)。

This repository uses the `Build Examples` GitHub Actions workflow to discover examples dynamically, build them, and upload flashable source-built firmware artifacts.

## Discovery

The workflow uses `scripts/discover_examples.py` for both framework surfaces:

- ESP-IDF projects are discovered from `examples/esp-idf/*/CMakeLists.txt` with a `main/` directory.
- Arduino sketches are discovered from first-party sketch directories under `examples/arduino/examples/` and `examples/arduino-v2/examples/`.
- Arduino sketches inside bundled libraries are intentionally excluded from product CI.

`workflow_dispatch` accepts `all`, an example directory name, or a repo-relative example path. Maintainers can run the full matrix or a single example.

The repository-policy job is always visible on pull requests. Its routing contract fails closed: unavailable or incomplete diff data is an error, not a fallback build or pass. A separate `change-scope` job consumes and publishes the documentation-only, firmware, immutable-artifact-review, and unknown-path classifier outputs. Documentation-only changes select no example builds. Every `Firmware/` file kind is reported separately and remains outside the example matrix; binary or archive paths also receive an explicit release-review warning. A usable pull request or branch diff builds only affected first-party examples. A bundled-library change rebuilds all first-party sketches in the matching Arduino root. Workflow, discovery, policy, routing, or release-packaging changes run the relevant full framework surface; shared ESP-IDF configuration changes rebuild the ESP-IDF surface. Tag pushes and manual `all` runs build the full matrix.

Manual runs can build the full matrix or narrow it by passing an example name, a parent example directory such as `08_LVGL_Animation`, or a repo-relative path to `target`.

## Matrix

Current CI matrix:

- ESP-IDF `v5.5.5` and `v6.0.2`, target `esp32s3`.
- Arduino-ESP32 core `3.3.11`, FQBN `esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB`, using bundled libraries from the matching `examples/arduino/libraries` or `examples/arduino-v2/libraries` directory.

The selected framework versions were reverified from official stable releases on 2026-08-13: ESP-IDF `v5.5.5` and `v6.0.2`, plus Arduino-ESP32 `3.3.11`. The ESP-IDF coverage retains the v5.5-to-v6.0 migration context. Do not replace them with beta, release-candidate, preview, or nightly tags unless the repository intentionally opts into that coverage.

The full matrix contains 60 firmware build jobs: 17 ESP-IDF examples for each of two ESP-IDF versions, plus 16 original and 10 V2 Arduino sketches.

## Firmware Artifacts

Each successful ESP-IDF and Arduino matrix build uploads a flashable firmware artifact. CI packages build outputs through `releases/package_firmware.py`.

Each generated archive contains:

- `manifest.json` with schema version, framework, framework version, target, `project_path`, git SHA, `timestamp_utc`, baud rate, flash command, and binary offsets.
- `flash.sh` and `flash.bat` helper scripts.
- `flash_args.txt` with the esptool command arguments.
- `bin/` with the firmware binaries referenced by the manifest.

Download the artifact zip from the workflow run, extract it, then run `flash.sh` or `flash.bat` with the board serial port. CI zip names include the framework, example, framework version, target, and short commit identifier; the outer GitHub artifact names remain stable for filtering and scripted downloads.

Generated archives are workflow artifacts only. Do not commit generated files from `release-artifacts/`, `releases/dist/`, or `releases/downloads/`.

Checked-in files under `Firmware/` are factory or recovery binaries. They are documented assets, not source-build outputs, and they do not trigger source-build packaging. The lightweight policy job checks each tracked factory binary against the repository-relative path, byte size, and SHA-256 identity in `firmware_integrity.json`; on pull requests and branch pushes it also compares the binaries with the trusted base commit. It fails on missing, modified, unsafe, or unlisted tracked `.bin` files without rebuilding or repackaging them.

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
python -B -m unittest discover -s tests -v
python -B scripts/check_repository_policy.py --config repository_policy.json
python -B scripts/check_firmware_integrity.py --manifest firmware_integrity.json
```

The last three commands are the exact non-build checks for the repository policy, immutable firmware identities, and their tests. Product builds are a separate local or GitHub Actions responsibility; these checks do not compile examples or validate hardware.

The packaging helper expects an existing ESP-IDF or Arduino build output and is normally exercised inside CI after the framework build finishes.

If an example requires hardware, credentials, or an upstream component that is not yet compatible with a selected framework version, document the exclusion here before excluding it from CI.
