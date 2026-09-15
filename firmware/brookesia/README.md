# ESP32-S3-Touch-AMOLED-1.8 Brookesia firmware

[简体中文](README_ZH.md)

This directory contains the default **ESP-Brookesia Phone** firmware source for
ESP32-S3-Touch-AMOLED-1.8. It targets the board's 368 × 448 AMOLED display,
16 MB flash, and 8 MB PSRAM. It is not a standalone LVGL peripheral dashboard:
the shared application starts the Brookesia Phone launcher and installs the
board-appropriate applications.

## Profiles

The display and touch implementations are selected only by the two wrapper
projects. Their shared `components/` and `main/` source intentionally use the
common BSP API.

| Profile | Project root | Published BSP | Delivered combined image |
| --- | --- | --- | --- |
| Original | [`original/`](original/) | `waveshare/esp32_s3_touch_amoled_1_8` 1.1.4 | `firmware/ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin` |
| V2 | [`v2/`](v2/) | `waveshare/esp32_s3_touch_amoled_1_8` 2.0.3 | `firmware/ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin` |

Build either wrapper directly; the top-level directory is deliberately not an
ESP-IDF project because one Component Manager resolution cannot select both
BSP versions.

## Included Brookesia applications

- Calculator and Draw (display/touch interaction)
- Gravitysphere (QMI8658 IMU); the ball travels the whole screen inside a
  rounded arena
- Clock (PCF85063A plus SNTP); the RTC is seeded from the firmware build
  timestamp when it has no valid time, and SNTP corrects it once Wi-Fi connects
- SpecAnalyzer and Recorder (ES8311 single-microphone capture); the analyzer
  draws only the spectrum canvas, with no title banner over it
- Music from the built-in SPIFFS library, with an SD card as fallback; Gallery
  and Video remain microSD media
- Settings (Wi-Fi, brightness, audio, storage, power, and board diagnostics),
  opened as an immersive full-screen page with the status bar hidden
- Xiaozhi native Wi-Fi voice and text application, reading its prompt clips from
  SPIFFS

Crosshair, a display/touch alignment target, was removed from the product.

The SquareLine demo component is retained in the source tree as an optional
Brookesia-resized demo, but is not installed by the default launcher. Camera,
buttons, GNSS, hosted Wi-Fi, and P4-specific media paths are intentionally not
included because they do not match confirmed hardware on this board.

## Peripheral and component boundary

The published BSP supplies the AMOLED/touch path, I2C, microSD, display
brightness API, and ES8311 board wiring. The firmware covers the confirmed
display/touch, Wi-Fi, microSD, ES8311 speaker and microphone, AXP2101 status,
TCA9554 presence, QMI8658 IMU, and PCF85063A RTC surfaces.

Online components are used where available:

- `waveshare/esp32_s3_touch_amoled_1_8` 1.1.4 or 2.0.3, selected solely by the
  wrapper manifest
- `waveshare/qmi8658` 2.0.1 for the IMU; the application probes 0x6A and 0x6B
  before initialization
- `waveshare/pcf85063a` 2.0.0 for RTC access
- BSP-resolved `espressif/esp_codec_dev` and `espressif/esp_new_jpeg`, plus
  the audio/media dependencies declared by the local components

Two local adapters remain intentionally. `bsp_extra` presents one
board-specific ES8311 full-duplex/session interface to the audio applications.
`system_status` has no selected online AXP2101 driver; it only reads AXP2101
identity, status, and battery-percent registers, and only probes the TCA9554
at 0x20 without changing its registers. See the bilingual
[component guide](components/README.md) for the detailed ownership boundary.

## SD-card media layout

Media applications mount a FAT/FAT32 card at `/sdcard` and use these optional
directories:

| Directory | Content |
| --- | --- |
| `/sdcard/music` | MP3 or WAV music |
| `/sdcard/photos` | Baseline JPG/JPEG images |
| `/sdcard/video` | MJPEG/PCM AVI video |
| `/sdcard/Waveshare/Recordings` | Recorder WAV output |
| `/sdcard/Waveshare/AIChats` | Xiaozhi text history |

Use modest-resolution, low-frame-rate MJPEG/PCM AVI files. The video player
does not support H.264 or compressed AVI audio.

## Build and combine

Use ESP-IDF **5.5.5**. Run the commands from the repository root. Build each
profile independently:

```text
idf.py -C firmware/brookesia/original set-target esp32s3
idf.py -C firmware/brookesia/original build

idf.py -C firmware/brookesia/v2 set-target esp32s3
idf.py -C firmware/brookesia/v2 build
```

Generate each whole-flash combined image from that profile's final generated
`flash_args`; do not substitute guessed offsets:

```text
cd firmware/brookesia/original/build
python -m esptool --chip esp32s3 merge_bin \
  -o ../../../ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin @flash_args

cd ../../v2/build
python -m esptool --chip esp32s3 merge_bin \
  -o ../../../ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin @flash_args
```

The two named images were compiled and packaged locally with ESP-IDF 5.5.5.

## Boot-critical tuning

These settings are load-bearing; changing them reintroduces the failures
described in [docs/FIRMWARE.md](../../docs/FIRMWARE.md#boot-critical-tuning):

- `LVGL_PORT_INIT_CONFIG()` in [`main/main.cpp`](main/main.cpp) raises the LVGL
  worker task stack to 20 KB. The esp_lvgl_port default of 7168 bytes is sized
  for the plain LVGL demos; the Brookesia launcher overruns it and panics with
  `A stack overflow in task taskLVGL`.
- `CONFIG_BSP_DISPLAY_LVGL_BUF_HEIGHT=10` in
  [`sdkconfig.defaults`](sdkconfig.defaults). The BSP puts the LVGL draw buffer
  in PSRAM, so every flush needs an equally sized temporary DMA-capable
  *internal* buffer. The BSP default of 100 lines asks for 73,600 bytes per
  flush and is dropped once Wi-Fi and the launcher hold internal RAM.
- `brookesia::system_status::init_wifi_stack()` is called early in `app_main()`,
  before the launcher fragments the internal heap. `esp_wifi_init()` allocates
  its static RX pool as one contiguous internal block and silently degrades to
  two RX buffers if that fails.
- `CONFIG_BSP_SPIFFS_MAX_FILES=5` in
  [`sdkconfig.defaults`](sdkconfig.defaults). The Xiaozhi font, the music library
  and the prompt clips now share one SPIFFS partition, and the music player keeps
  one file open for a whole track.
- `CONFIG_XIAOZHI_SYNC_SYSTEM_TIME_FROM_SERVER=n` in
  [`sdkconfig.defaults`](sdkconfig.defaults). The RTC, the build-timestamp seed
  and SNTP own the system clock; the Xiaozhi component would otherwise rewrite it
  from `server_time` plus the server's own timezone offset, which is a different
  convention and double-shifts the clock once SNTP applies `CST-8`.
- [SPIFFS staging](v2/main/CMakeLists.txt) runs on **every** build rather than
  behind a timestamp stamp. A stamp-gated copy silently keeps a stale staging
  directory, which packages an out-of-date SPIFFS image with no build error: the
  bundled media is then simply absent on the device. If a firmware image ever
  looks like it is missing its media, check `Partition size: ... used:` on the
  console first.

## Hardware verification

`ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin` was written to a
V2 board as a complete `0x0` image and verified over the USB-Serial-JTAG
console: the launcher installs its eleven applications and reaches
`Brookesia firmware is ready`, the LVGL worker no longer overflows, the panel
updates without SPI DMA allocation failures, the Wi-Fi driver initializes all
eight static RX buffers, and the AXP2101 status monitor starts. No panic or
reset occurred during the observation window.

The same run confirms the media and time changes: the mounted SPIFFS partition
reports `used: 5679377` of `total: 5775761`, i.e. the music library and the
Xiaozhi prompt clips are present in the flashed image, and `rtc_service` logs
`System time synchronized from RTC: ...` with a value that advances across
resets, so the clock, the status bar and Settings ▸ Power all have a valid time.
SNTP could not be exercised because no Wi-Fi network was configured; it starts
on `IP_EVENT_STA_GOT_IP` and applies `CST-8`.

`ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin` was written to a V1 board as
a complete `0x0` image and exercised by hand: the launcher starts, the
applications open, and **audio playback is audible** through the on-board
speaker. Remaining per-revision work is limited to the accessories this run did
not cover (SD-card media, Wi-Fi association, voice operation).

### Audio playback is not gated by firmware

Getting no sound from a board is worth a moment's care, because the whole
playback chain is shared and easy to blame on software. On the V2 board used for
the earlier runs the chain was verified end to end and still produced silence:
`bsp_extra_player_play_file()` opened the MP3 from `/spiffs/music`, the I2S was
reconfigured to 44100 Hz stereo, the ES8311 reported itself unmuted (`REG31=00`)
with a sane DAC volume (`REG32=0xB2`), the amplifier-enable bit for
`BSP_POWER_AMP_IO` read back as asserted, and a raw 1 kHz tone written straight
to `bsp_extra_i2s_write()` completed with exact byte counts. The same image on a
V1 board plays normally, so that V2 unit's **speaker was faulty**. Treat the
sequence above as the software-side checklist: if it all passes, the fault is in
the speaker, the amplifier, or the board wiring - not in this firmware.

[`tools/capture_serial.py`](tools/capture_serial.py) is the console helper used
for those runs: it reopens `/dev/ttyACM*` when the ESP32-S3 USB-Serial-JTAG
peripheral re-enumerates on reset and preserves raw bytes, which is what makes a
crash-reboot loop and a corrupted FreeRTOS task name observable at all.

```text
python3 firmware/brookesia/tools/capture_serial.py --seconds 240 --out /tmp/boot.txt
```
