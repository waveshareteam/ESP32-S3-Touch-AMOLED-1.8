# Continuous Integration

This repository uses the `Build Examples` GitHub Actions workflow to discover examples dynamically, build them, and upload flashable source-built firmware artifacts.

## Discovery

The workflow uses `scripts/discover_examples.py` for both framework surfaces:

- ESP-IDF projects are discovered from `examples/esp-idf/*/CMakeLists.txt` with a `main/` directory.
- Arduino sketches are discovered from first-party sketch directories under `examples/Arduino-v3.3.5*/examples/`.
- Arduino sketches inside bundled libraries are intentionally excluded from product CI.

`workflow_dispatch` accepts `all`, an example directory name, or a repo-relative example path. Maintainers can run the full matrix or a single example.

Pull request and push runs use the default `all` target and build the full discovered matrix for each triggered surface. Manual runs can narrow the matrix by passing an example name or repo-relative path to `target`.

## Matrix

Current CI matrix:

- ESP-IDF `v5.5.4` and `v6.0.2`, target `esp32s3`.
- Arduino-ESP32 core `3.3.10`, FQBN `esp32:esp32:esp32s3`, using bundled libraries from the matching `examples/Arduino-v3.3.5*/libraries` directory.

The selected framework versions were resolved from upstream stable releases on 2026-07-07. Do not replace them with beta, release-candidate, preview, or nightly tags unless the repository intentionally opts into that coverage.

## Firmware Artifacts

Each successful ESP-IDF and Arduino matrix build uploads a flashable firmware artifact. CI packages build outputs through `releases/package_firmware.py`.

Each generated archive contains:

- `manifest.json` with framework, framework version, target, project path, git SHA, flash arguments, and binary offsets.
- `flash.sh` and `flash.bat` helper scripts.
- `flasher_args.json` for ESP-IDF builds.
- `bin/` with the firmware binaries referenced by the manifest.

Download the artifact zip from the workflow run, extract it, then run `flash.sh` or `flash.bat` with the board serial port.

Generated archives are workflow artifacts only. Do not commit generated files from `release-artifacts/`, `releases/dist/`, or `releases/downloads/`.

Checked-in files under `Firmware/` are factory or recovery binaries. They are documented assets, not source-build outputs, and they do not trigger source-build packaging.

## Local Script Checks

Discovery can be checked without building firmware:

```bash
python scripts/discover_examples.py --surface esp-idf --selector 00_board_check
python scripts/discover_examples.py --surface esp-idf --selector all
python scripts/discover_examples.py --surface arduino --selector all
```

The packaging helper expects an existing ESP-IDF or Arduino build output and is normally exercised inside CI after the framework build finishes.

If an example requires hardware, credentials, or an upstream component that is not yet compatible with a selected framework version, document the exclusion here before excluding it from CI.