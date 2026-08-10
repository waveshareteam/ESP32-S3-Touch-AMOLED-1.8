# 03_nvs_counter

[简体中文](README_ZH.md)

NVS persistent boot counter adapted from the ESP32-P4 platform example.

Reset the board repeatedly and watch the counter increase in the serial monitor.

## Hardware

- ESP32-S3-Touch-AMOLED-1.8 board
- USB data cable

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for the board.

## Expected Result

The serial monitor prints `Saved boot count: N`. Press reset and the number should increase after each boot.

## Notes

This example uses ESP-IDF NVS APIs directly. It is useful before storing application settings such as Wi-Fi credentials, calibration values, or user preferences.
