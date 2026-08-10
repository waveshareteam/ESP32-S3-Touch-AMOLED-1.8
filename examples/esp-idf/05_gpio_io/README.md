# 05_gpio_io

[简体中文](README_ZH.md)

GPIO input/output loopback demo adapted from the ESP32-P4 platform example.

The default configuration does not drive any pin. Use `idf.py menuconfig` and choose two exposed, unused GPIOs, then connect output to input with a jumper.

## Hardware

- ESP32-S3-Touch-AMOLED-1.8 board
- USB data cable
- One jumper wire for loopback testing

## Configuration

Run `idf.py menuconfig` and set:

- Output GPIO
- Input GPIO
- Poll interval, if needed

Choose GPIOs that are exposed and not reserved by flash, PSRAM, display, touch, SD card, audio, USB, PMU, RTC, or IMU functions.

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for the board.

## Expected Result

When output and input pins are connected, the serial monitor reports matching GPIO levels as the output toggles.

## Safety Notes

- Do not connect two outputs together.
- Do not drive pins connected to onboard peripherals unless you know the board schematic and voltage requirements.
