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

For pull requests and pushes, changed examples are built. The default smoke-test set is also included so CI keeps checking the managed component path even when the pull request only changes a later demo:

- `00_board_check`
- `00_BSP_QuickStart`

Manual runs accept one input:

| Input | Value |
| --- | --- |
| `example` | Defaults to `00_board_check`; accepts `all`, a directory name such as `14_lvgl_demo_v9`, or a full path such as `examples/ESP-IDF-v5.5.1/14_lvgl_demo_v9` |

The workflow currently builds with:

- ESP-IDF Docker image `espressif/idf:v5.5.4`
- Target: `esp32s3`

Arduino examples are intentionally not built by this workflow yet.

## Local Script Checks

The discovery helper can be run without building firmware:

```bash
python .github/scripts/discover_esp_idf_examples.py --example 00_board_check
python .github/scripts/discover_esp_idf_examples.py --example all
```

Both commands print the matrix JSON that the workflow passes to the ESP-IDF CI action.
