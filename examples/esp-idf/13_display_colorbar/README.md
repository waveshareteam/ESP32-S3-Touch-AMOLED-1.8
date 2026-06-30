# 13_display_colorbar

Display color-bar demo adapted from the ESP32-P4 platform example.

ESP32-P4 uses a MIPI-DSI hardware pattern. This ESP32-S3 AMOLED port uses the BSP display panel handle and draws RGB565 bars with `esp_lcd_panel_draw_bitmap()`.

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

The AMOLED display shows static RGB565 color bars. The serial monitor reports display initialization status.

## Notes

Use this example before LVGL when you need to isolate panel bring-up from UI, touch, and LVGL tasks.
