# 00_board_check

Board bring-up check adapted from the ESP32-P4 platform example for ESP32-S3-Touch-AMOLED-1.8.

It prints chip, flash, PSRAM, BSP capability, and key board pin information. It uses the managed `waveshare/esp32_s3_touch_amoled_1_8` component and does not require display, touch, SD card, Wi-Fi, or audio accessories.

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

The serial monitor prints target, flash, PSRAM, BSP capabilities, display size, I2C pins, SD pins, audio support, and periodic heap status. If this example fails, fix the basic ESP-IDF setup before moving to the display, SD, audio, or Wi-Fi examples.
