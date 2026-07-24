# 01_project_template

Minimal ESP-IDF project skeleton adapted from the ESP32-P4 platform example.

Unlike an empty project, this template already declares the online `waveshare/esp32_s3_touch_amoled_1_8` dependency and exposes the BSP board definitions, so it is a practical starting point for new board projects.

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

The serial monitor prints the board name, I2C pins, display resolution, and a periodic `template running` message. Copy this project when starting a new ESP-IDF application that should use the managed BSP component.

## Notes

- Initialize only the board services your application needs, such as display, touch, SD card, audio, RTC, or IMU.
- Keep managed dependencies in `main/idf_component.yml`.
- Do not copy generated `managed_components/`, `dependencies.lock`, or local `sdkconfig` files into a reusable template.
