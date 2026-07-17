# 12_i2s_codec

ES8311 audio demo adapted from the ESP32-P4 platform example for this ESP32-S3 board.

This version uses the BSP board definitions and plays a sine wave through the ES8311 speaker output. It intentionally avoids microphone echo mode because this board uses a single ES8311 codec path.

The demo follows the ESP32-P4 I2S codec example structure: configure I2S first, initialize the codec control path next, then start playback. The ES8311 setup is kept in the example so initialization failures are reported as errors instead of triggering BSP helper assertions.

## Hardware

- ESP32-S3-Touch-AMOLED-1.8 board
- USB data cable
- Speaker or headphones connected to the board audio output

## Configuration

`idf.py menuconfig` can adjust:

- Sample rate
- Original hardware speaker volume (default 70)
- V2 speaker volume (default 90 after CST816 detection)
- Test tone frequency

The defaults compensate for the lower level observed on V2 hardware. Reduce both settings before testing sensitive headphones or a different speaker.

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for the board.

## Expected Result

The serial monitor reports the detected hardware generation, selected volume, and ES8311 initialization while a sine wave plays from the output.

## Troubleshooting

- If codec initialization fails, the example logs the error and stays idle instead of aborting into a reboot loop.
- A boot loop with `i2c_ctrl_if` in `bsp_audio_codec_speaker_init` indicates an older build is still using the BSP speaker helper. Rebuild and flash the current example.
- Use `08_i2c_tools` if the codec control interface is not detected.
- Confirm the connected output accessory and volume before assuming an I2S failure.
- CI verifies both IDF configurations compile but cannot validate perceived loudness; compare V1 and V2 on hardware after changing codec gain or volume defaults.
