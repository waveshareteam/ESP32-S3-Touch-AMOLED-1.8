# 12_i2s_codec

ES8311 audio demo adapted from the ESP32-P4 platform example for this ESP32-S3 board.

This version uses the online BSP audio APIs and plays a sine wave through the ES8311 speaker output. It intentionally avoids microphone echo mode because this board uses a single ES8311 codec path.

## Hardware

- ESP32-S3-Touch-AMOLED-1.8 board
- USB data cable
- Speaker or headphones connected to the board audio output

## Configuration

`idf.py menuconfig` can adjust:

- Sample rate
- Speaker volume
- Test tone frequency

Start with a low speaker volume when testing new hardware.

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for the board.

## Expected Result

The serial monitor reports ES8311 speaker initialization and repeated audio writes while a sine wave plays from the output.

## Troubleshooting

- If codec initialization fails, the example logs the error and stays idle instead of aborting into a reboot loop.
- Use `08_i2c_tools` if the codec control interface is not detected.
- Confirm the connected output accessory and volume before assuming an I2S failure.
