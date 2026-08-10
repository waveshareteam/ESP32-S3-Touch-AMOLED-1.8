# 02_hello_world

[简体中文](README_ZH.md)

Classic ESP-IDF hello-world example adapted for ESP32-S3-Touch-AMOLED-1.8.

It prints chip information, waits ten seconds, and restarts.

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

The serial monitor prints the ESP-IDF target, CPU core count, wireless features, silicon revision, flash size, and minimum free heap size. The board then counts down and restarts.

## Notes

Use this example to confirm a clean ESP-IDF toolchain and serial monitor flow before adding board-specific BSP code.
