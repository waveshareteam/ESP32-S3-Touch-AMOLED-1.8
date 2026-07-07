# Release Packaging

`package_firmware.py` converts ESP-IDF and Arduino build outputs into flashable zip archives for CI artifacts and maintainer release testing.

The generated archive contains:

- `manifest.json` with framework, version, target, project path, git SHA, flash arguments, and binary offsets.
- `flash.sh` for POSIX shells.
- `flash.bat` for Windows command prompt.
- `flasher_args.json` when the source build is ESP-IDF.
- `bin/` with the firmware binaries referenced by the manifest.

GitHub Actions writes generated archives to `release-artifacts/` and uploads them as workflow artifacts. Local release checks default to `releases/dist/` unless another output directory is provided.

Generated archives, downloaded workflow artifacts, and build directories are ignored by git. Do not commit generated zip files.

Factory binaries under `Firmware/` are separate recovery assets. They are documented but not repackaged as CI build outputs.