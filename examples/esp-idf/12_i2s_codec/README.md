# 12_i2s_codec

ES8311 audio demo adapted from the ESP32-P4 platform example for this ESP32-S3 board.

This version uses the online BSP audio APIs and plays a sine wave through the ES8311 speaker output. It intentionally avoids microphone echo mode because this board uses a single ES8311 codec path.

## Configuration

`idf.py menuconfig` can adjust:

- Sample rate
- Speaker volume
- Test tone frequency

If codec initialization fails, the example logs the error and stays idle instead of aborting into a reboot loop.
