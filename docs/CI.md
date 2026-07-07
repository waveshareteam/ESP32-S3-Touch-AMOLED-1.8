# Continuous Integration

This repository uses GitHub Actions to build selected ESP-IDF examples and upload flashable source-built firmware artifacts.

## ESP-IDF Example Builds

The `ESP-IDF examples` workflow runs on:

- Pull requests that change files under `examples/esp-idf/`.
- Pull requests that change shared config files under `config/`.
- Pull requests that change the workflow, discovery script, or firmware packaging script.
- Pushes to `main` that touch the same paths.
- Manual runs from the GitHub Actions page.

The workflow discovers buildable ESP-IDF examples by looking for directories under `examples/esp-idf/` that contain both:

- `CMakeLists.txt`
- `main/`

For pull requests and pushes, only changed ESP-IDF examples are built. If the workflow, discovery script, packaging script, or shared config changes, the workflow builds all ESP-IDF examples because those files affect the entire CI surface.

Manual runs accept one input:

| Input | Value |
| --- | --- |
| `project` | Defaults to `all`; accepts `all`, a directory name such as `14_lvgl_demo_v9`, or a full path such as `examples/esp-idf/14_lvgl_demo_v9` |

The workflow currently builds with:

- ESP-IDF Docker image `espressif/idf:v5.5.4`
- ESP-IDF Docker image `espressif/idf:v6.0.2`
- Target: `esp32s3`

The selected versions were resolved from Espressif stable releases on 2026-07-07. Do not replace them with beta, release-candidate, preview, or nightly tags unless the repository intentionally opts into that coverage.

## Firmware Artifacts

After each successful ESP-IDF build, CI packages the build output by reading the example build directory's `flasher_args.json`. Each uploaded archive contains:

- `manifest.json` with the project path, ESP-IDF version, target, flash arguments, and binary list.
- `flasher_args.json` from the ESP-IDF build output.
- `flash.sh` and `flash.bat` helper scripts.
- The bootloader, partition table, app, and other binaries referenced by `flasher_args.json` under `bin/`.

Generated archives are workflow artifacts only. Do not commit generated files from `release-artifacts/`, `releases/dist/`, or `releases/downloads/`.

Checked-in files under `Firmware/` are factory or recovery binaries. They are documented assets, not source-build outputs, and they do not trigger this ESP-IDF build workflow.

## Arduino Scope

Arduino sketches remain in the versioned board-package directories under `examples/Arduino-v3.3.5*/` and use bundled libraries. They are intentionally not built by the ESP-IDF workflow, and examples inside bundled libraries are excluded from product CI by default.

Add a dedicated Arduino workflow only after the first-party Arduino root, FQBN, board options, and sketch inclusion policy are normalized for this repository.

## Local Script Checks

The discovery helper can be run without building firmware:

```bash
python .github/scripts/discover_esp_idf_examples.py --example 00_board_check
python .github/scripts/discover_esp_idf_examples.py --example all
```

Both commands print the matrix JSON that the workflow passes to the ESP-IDF CI action. The packaging helper expects an existing ESP-IDF build output and is normally exercised inside CI after `esp-idf-ci-action` finishes.
