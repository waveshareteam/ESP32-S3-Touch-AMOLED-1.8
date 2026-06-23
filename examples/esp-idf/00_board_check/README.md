# 00_board_check

Board bring-up check adapted from the ESP32-P4 platform example for ESP32-S3-Touch-AMOLED-1.8.

It prints chip, flash, PSRAM, BSP capability, and key board pin information. It uses the managed `waveshare/esp32_s3_touch_amoled_1_8` component.

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```
