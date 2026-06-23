# Project Structure

The repository keeps hardware assets, examples, and CI support separate so users can find the right starting point quickly.

| Path | Role |
| --- | --- |
| `README.md` | Repository overview and recommended entry points |
| `docs/` | Setup, example, and CI documentation |
| `examples/README.md` | Example index |
| `examples/ESP-IDF-v5.5.1/` | Standalone ESP-IDF projects |
| `examples/ESP-IDF-v5.5.1/00_board_check` | Managed-BSP serial smoke test |
| `examples/ESP-IDF-v5.5.1/01_HowToCreateProject` | Minimal managed-BSP project template |
| `examples/ESP-IDF-v5.5.1/0*_*, examples/ESP-IDF-v5.5.1/1*_*` | Board, OS, peripheral, display, and LVGL examples |
| `examples/Arduino-v3.3.5*/` | Arduino sketches and bundled libraries |
| `Firmware/` | Factory firmware binaries and notes |
| `.github/workflows/` | GitHub Actions workflows |
| `.github/scripts/` | CI helper scripts |

New ESP-IDF work should prefer managed components and avoid adding generated build output, local dependency caches, or machine-specific files.
