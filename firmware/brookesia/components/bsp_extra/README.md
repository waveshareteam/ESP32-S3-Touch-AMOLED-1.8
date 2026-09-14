# `bsp_extra` board integration

[简体中文](README_ZH.md)

`bsp_extra` is the firmware-local audio integration layer above the published
ESP32-S3-Touch-AMOLED-1.8 BSP. It uses the board's ES8311 speaker and
single-microphone helpers for MusicPlayer, VideoPlayer, Recorder, SpecAnalyzer,
and Xiaozhi, using the BSP-resolved `esp_codec_dev` component.

This local layer adds media-player callbacks and shared audio-session
coordination; display, touch, I2C, GPIO, and SD-card support remain in the
managed BSP.

## Audio pins supplied by the BSP

Applications use the resolved BSP macros rather than duplicating board GPIO
literals. The target-board audio route is:

| Signal | GPIO |
| --- | ---: |
| I2S MCLK | 42 |
| I2S BCLK | 9 |
| I2S LRCK | 45 |
| ESP32-S3 data out to ES8311 | 8 |
| ES8311 microphone data in to ESP32-S3 | 10 |
| Amplifier enable | 46 |

## Shared-session rule

The ES8311, I2S clocks, and amplifier enable are one shared hardware path.
An audio application acquires the local session before opening streams, stops
all workers and files before teardown, then releases the session only after
the codec is closed. This prevents one application from reconfiguring the
codec while another application still owns media or capture work.

The supported clients are MusicPlayer, VideoPlayer, Recorder, SpecAnalyzer,
and Xiaozhi. A build verifies API integration only; confirm switching, speaker,
and microphone behavior on the target board before claiming hardware results.
