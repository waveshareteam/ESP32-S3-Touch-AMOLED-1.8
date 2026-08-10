# Project Structure

中文请参见 [PROJECT_STRUCTURE_ZH.md](PROJECT_STRUCTURE_ZH.md)。

The repository keeps hardware assets, examples, and CI support separate so users can find the right starting point quickly.

| Path | Role |
| --- | --- |
| `README.md` | Repository overview and recommended entry points |
| `README_ZH.md` | Maintained Simplified Chinese repository overview |
| `README_CN.md` | Compatibility entry linking to `README_ZH.md` |
| `docs/` | Setup, example, firmware, and CI documentation |
| `examples/README.md` | Example index |
| `examples/esp-idf/` | Standalone ESP-IDF projects |
| `examples/esp-idf/00_board_check` | Managed-BSP serial smoke test |
| `examples/esp-idf/00_bsp_quickstart` | Managed-BSP display, touch, LVGL, and SD smoke test |
| `examples/esp-idf/01_project_template` | Minimal managed-BSP project template |
| `examples/esp-idf/0*_*, examples/esp-idf/1*_*` | Board, OS, peripheral, display, and LVGL examples |
| `examples/esp-idf/90_*` | Board-specific hardware diagnostics and managed sensor checks |
| `examples/arduino/` | Original Arduino sketches and bundled libraries |
| `examples/arduino-v2/` | V2 Arduino sketches and bundled libraries |
| `config/` | Shared ESP-IDF configuration overlays and CI-facing defaults |
| `Firmware/` | Factory firmware binaries and notes; excluded from source-build CI |
| `releases/` | Firmware packaging scripts and release notes |
| `CONTRIBUTING.md` | Contribution, example, and documentation guidelines |
| `CODE_OF_CONDUCT.md` | Community participation expectations |
| `SUPPORT.md` | Support channels and issue checklist |
| `SECURITY.md` | Vulnerability reporting policy |
| `THIRD_PARTY.md` | Managed component and bundled library notices |
| `.editorconfig` | Common editor whitespace and encoding settings |
| `.gitattributes` | Git text and binary file handling |
| `.github/workflows/` | GitHub Actions workflows, including `Build Examples` |
| `scripts/` | CI discovery helpers shared by GitHub Actions |
| `.github/ISSUE_TEMPLATE/` | Public issue forms |
| `.github/PULL_REQUEST_TEMPLATE.md` | Pull request checklist |

New ESP-IDF work should prefer managed components and avoid adding generated build output, local dependency caches, or machine-specific files.
