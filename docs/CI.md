# Continuous Integration

This repository uses GitHub Actions to build selected ESP-IDF examples when they are added or changed.

## ESP-IDF Example Builds

The `ESP-IDF examples` workflow runs on:

- Pull requests that change files under `examples/ESP-IDF-v5.5.1/`.
- Pull requests that change the workflow or discovery script.
- Pushes to `main` that touch the same paths.
- Manual runs from the GitHub Actions page.

The workflow discovers buildable examples by looking for directories under `examples/ESP-IDF-v5.5.1/` that contain both:

- `CMakeLists.txt`
- `main/`

For pull requests and pushes, changed examples are built. The managed BSP quick-start example is included as a default smoke test because it validates the online `waveshare/esp32_s3_touch_amoled_1_8` component path.

Manual runs accept one input:

| Input | Value |
| --- | --- |
| `example` | Defaults to `00_BSP_QuickStart`; accepts `all`, a directory name such as `04_SD_MMC`, or a full path such as `examples/ESP-IDF-v5.5.1/04_SD_MMC` |

The workflow currently builds with:

- ESP-IDF Docker image `espressif/idf:v5.5.4`
- Target: `esp32s3`

Arduino examples are intentionally not built by this workflow yet.
