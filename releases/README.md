# Release Scripts

中文请参见 [README_ZH.md](README_ZH.md)。

This directory contains helper scripts for packaging CI build outputs into flashable firmware archives and downloading completed workflow artifacts.

## ESP-IDF

Build an example first, then package the generated build directory:

```bash
idf.py -C examples/esp-idf/00_board_check -B build/00_board_check-v6.0.2 set-target esp32s3 build
python3 releases/package_firmware.py \
  --framework esp-idf \
  --project examples/esp-idf/00_board_check \
  --build-dir build/00_board_check-v6.0.2 \
  --name ESP32-S3-Touch-AMOLED-1.8-00_board_check-esp-idf-v6.0.2-esp32s3 \
  --framework-version v6.0.2 \
  --target esp32s3
```

The script reads ESP-IDF `flasher_args.json`, copies the required binary files, writes flash helper scripts, and creates a zip under `releases/dist/`.

## Arduino

Export binaries into a stable output directory, then package them:

```bash
arduino-cli compile \
  --fqbn esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB \
  --libraries examples/arduino/libraries \
  --export-binaries \
  --output-dir build/arduino-01_HelloWorld-3.3.11 \
  examples/arduino/examples/01_HelloWorld

python3 releases/package_firmware.py \
  --framework arduino \
  --project examples/arduino/examples/01_HelloWorld \
  --build-dir build/arduino-01_HelloWorld-3.3.11 \
  --name ESP32-S3-Touch-AMOLED-1.8-arduino-01_HelloWorld-arduino-3.3.11-esp32s3 \
  --framework-version 3.3.11 \
  --target esp32s3
```

For V2 sketches, use `examples/arduino-v2/libraries` and a project under `examples/arduino-v2/examples/`.

Each archive includes `manifest.json`, `flash.sh`, `flash.bat`, `flash_args.txt`, a package `README.md`, and the firmware binaries under `bin/`. The manifest records `project_path`, `timestamp_utc`, framework and target metadata, git SHA, baud rate, flash command, and binary offsets.

CI passes the workflow commit SHA to the packager, so generated zip names end with the short commit identifier. The outer GitHub artifact name stays stable for filtering and downloads.

## Download CI Artifacts

After a CI run completes, download and extract firmware artifacts with:

```bash
python3 releases/download_artifacts.py --run-id <run-id> --clean
```

If `--run-id` is omitted, the script finds the latest successful `examples.yml` run for the current branch:

```bash
python3 releases/download_artifacts.py --clean
```

The extracted firmware is written to `releases/downloads/run-<run-id>/`. Each artifact gets its own folder, for example `firmware-esp-idf-00_board_check-v6.0.2/` or `firmware-arduino-arduino-v2-01_HelloWorld-3.3.11/`, with `flash.sh`, `flash.bat`, `manifest.json`, `flash_args.txt`, and `bin/` ready for flashing.

Use `--artifact <name>` to download one firmware package, or `--pattern "firmware-arduino-*"` to filter by glob pattern. The script uses `GH_TOKEN`, `GITHUB_TOKEN`, or `gh auth token` for GitHub artifact access. When GitHub CLI is installed, artifact downloads use `gh run download` so `gh auth login` can be reused directly.

## Generated Outputs

Generated archives, downloaded workflow artifacts, and build directories are ignored by git. Do not commit generated zip files, extracted firmware folders, or local build outputs.

Factory binaries under `Firmware/` are separate recovery assets. They are documented but not repackaged as CI build outputs. Their corresponding source and build instructions are not included in this repository yet and may be added in a later update.
