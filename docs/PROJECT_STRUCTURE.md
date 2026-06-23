# Project Structure

The repository keeps hardware assets, examples, and CI support separate so users can find the right starting point quickly.

| Path | Role |
| --- | --- |
| `README.md` | Repository overview and recommended entry points |
| `docs/` | Setup, example, and CI documentation |
| `examples/README.md` | Example index |
| `examples/ESP-IDF-v5.5.1/` | Standalone ESP-IDF projects |
| `examples/Arduino-v3.3.5*/` | Arduino sketches and bundled libraries |
| `Firmware/` | Factory firmware binaries and notes |
| `.github/workflows/` | GitHub Actions workflows |
| `.github/scripts/` | CI helper scripts |

New ESP-IDF work should prefer managed components and avoid adding generated build output, local dependency caches, or machine-specific files.
