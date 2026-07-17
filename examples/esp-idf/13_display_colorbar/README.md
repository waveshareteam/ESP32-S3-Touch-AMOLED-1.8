# 13_display_colorbar

Minimal CO5300 panel test for ESP32-S3-Touch-AMOLED-1.8 and V2 boards.

## Hardware

- ESP32-S3-Touch-AMOLED-1.8 board
- USB data cable

## Build and Flash Directly

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash
```

Replace `PORT` with the serial port for the board.

The serial monitor is optional:

```bash
idf.py -p PORT monitor
```

## Expected Result

The AMOLED displays eight vertical RGB565 color bars immediately after reset.

## Notes

- This board connects the CO5300 through QSPI, not MIPI-DSI. Therefore, `esp_lcd_dpi_panel_set_pattern()` is not available.
- The example directly initializes the managed `esp_lcd_co5300` driver and draws with `esp_lcd_panel_draw_bitmap()`; it does not initialize LVGL or a touch driver.
- A single I2C address probe detects the V2 board and applies its 16-pixel panel X gap.
