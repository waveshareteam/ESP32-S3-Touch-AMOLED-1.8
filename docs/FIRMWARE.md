# Firmware Artifacts

中文请参见 [FIRMWARE_ZH.md](FIRMWARE_ZH.md)。

This repository has two different firmware artifact types.

## Default Firmware

[`firmware/brookesia/`](../firmware/brookesia/) contains the source for the board's default ESP-Brookesia Phone firmware. Separate Original and V2 project wrappers select the display and touch implementation supplied by the matching published BSP line while sharing the Phone application and board-specific applications.

The checked-in combined images are generated from each final ESP-IDF build's own `flash_args` and include the bootloader, partition table, OTA data, speech models, application, and SPIFFS image:

- `firmware/ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin`
- `firmware/ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin`

See the [default-firmware build instructions](../firmware/brookesia/README.md) for the exact profile, build, and merge commands. These source projects and whole-flash images are maintained separately from the example CI packaging described below.

### Boot-critical tuning

The default firmware is sensitive to a set of settings that each address a failure observed on real hardware. Relaxing any of them brings the failure back:

- **LVGL worker task stack.** `bsp_display_start()` uses the esp_lvgl_port default of 7168 bytes, which is sized for the plain LVGL peripheral demos. The Brookesia launcher resolves a much deeper style and event chain while it builds and redraws the app grid, overrunning that stack. The panic reports `A stack overflow in task` with a corrupted task name, because the overflow writes through the bottom of the stack into the neighbouring task control block before FreeRTOS notices. `main/main.cpp` therefore starts the panel through `bsp_display_start_with_config()` with a local `LVGL_PORT_INIT_CONFIG()` that requests 20 KB, matching the other Waveshare Brookesia firmwares.
- **LVGL draw buffer height.** `BSP_DISPLAY_LVGL_BUF_HEIGHT` controls the LVGL draw buffer as `H_RES x height` pixels, and the BSP places that buffer in PSRAM. A PSRAM buffer cannot be handed to the panel directly, so `esp_lcd_panel_io_spi` has the SPI master copy each flush through a temporary DMA-capable **internal** buffer of the same size. At the BSP default of 100 lines that is a 73,600-byte internal allocation per flush, which fails once Wi-Fi and the launcher hold internal RAM; the frames are dropped and the screen only partially appears. `sdkconfig.defaults` pins the height to 10 lines (7,360 bytes per flush), which always fits.
- **Wi-Fi stack bring-up order.** `esp_wifi_init()` allocates its static RX pool as a single contiguous block of DMA-capable internal RAM. If the launcher has already fragmented that heap, the driver silently falls back to two RX buffers and `esp_wifi_init()` fails with `ESP_ERR_NO_MEM`, which costs both the status monitor and the Xiaozhi voice application. `app_main()` calls `brookesia::system_status::init_wifi_stack()` before the application bring-up; the helper is idempotent, so the later calls from the status monitor and the Xiaozhi app still succeed.
- **SPIFFS file count and staging.** The Xiaozhi font, the music library and the prompt clips share the `storage` partition (about 98% of the usable SPIFFS space), and the music player keeps one file open for a whole track, so `CONFIG_BSP_SPIFFS_MAX_FILES` is 5. The staging copy in `v2/main/CMakeLists.txt` and `original/main/CMakeLists.txt` deliberately runs on every build: a timestamp-stamped copy can silently keep a stale staging directory and package an out-of-date SPIFFS image with no build error, which shows up only as missing media on the device.
- **Time keeping.** The PCF85063A is seeded from the firmware build timestamp when it reports no valid time, and `rtc_service_sync_from_system_time()` writes the system clock back whenever it disagrees by more than a few seconds, so a corrected time survives a reboot. SNTP (`ntp.aliyun.com`, timezone `CST-8`) starts on `IP_EVENT_STA_GOT_IP`; `CONFIG_XIAOZHI_SYNC_SYSTEM_TIME_FROM_SERVER` is disabled because its `server_time` handler uses a different convention and would double-shift the clock.

### Default-firmware HIL status

`firmware/ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin` was written to a V2 board as a complete `0x0` image and observed over the USB-Serial-JTAG console: the launcher installs its eleven applications and reaches `Brookesia firmware is ready` with no panic or reset, the panel updates without SPI DMA allocation failures, the Wi-Fi driver reports all eight static RX buffers, the AXP2101 status monitor starts, and the mounted SPIFFS partition reports `used: 5679377` of `total: 5775761`, i.e. the bundled media is present. SNTP could not be exercised without a configured network.

`firmware/ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin` was written to a V1 board as a complete `0x0` image and exercised by hand: the launcher starts, the applications open, and audio playback is audible through the on-board speaker. SD-card media, Wi-Fi association, and voice operation remain unverified per revision.

Audio silence on a board is worth attributing carefully. On one V2 unit the full software chain verified clean - MP3 opened from SPIFFS, I2S reconfigured to 44100 Hz stereo, ES8311 unmuted with a sane DAC volume, the `BSP_POWER_AMP_IO` enable asserted, and a raw tone written through `bsp_extra_i2s_write()` with exact byte counts - and there was still no sound; the same image plays on a V1 board, so that unit's speaker was faulty. Check the software chain first, then the speaker, amplifier, and wiring.

## Source-Built CI Artifacts

ESP-IDF examples under `examples/esp-idf/` and first-party Arduino sketches under `examples/arduino/examples/` and `examples/arduino-v2/examples/` are built by GitHub Actions. After a successful build, the workflow packages the build output into a flashable archive with `releases/package_firmware.py`.

Each CI firmware archive contains:

- `manifest.json` with schema version, framework, target, `project_path`, git SHA, `timestamp_utc`, baud rate, flash command, and binary offsets
- `flash.sh`
- `flash.bat`
- `flash_args.txt` with the esptool command arguments
- `bin/` with the bootloader, partition table, app, merged image, or other binaries referenced by the manifest

Download these archives from the workflow run artifacts. CI zip names include the framework, example, framework version, target, and short commit identifier. They are validation outputs from CI, not source files, and should stay out of the repository.

## Local Release Checks

For local release packaging, build the target first and run `releases/package_firmware.py` from the repository root. The default output directory is `releases/dist/`; CI uses `release-artifacts/`.

Generated or downloaded firmware packages are ignored in:

- `release-artifacts/`
- `releases/dist/`
- `releases/downloads/`
