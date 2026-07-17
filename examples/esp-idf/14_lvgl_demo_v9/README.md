# 14_lvgl_demo_v9

LVGL v9 widgets demo adapted from the ESP32-P4 platform example.

This port uses the managed S3 BSP to initialize the AMOLED panel, touch input, LVGL port, and brightness control, then starts `lv_demo_widgets()`.

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

The AMOLED display shows the LVGL widgets demo and responds to touch input. The serial monitor reports display and LVGL initialization status.

## Notes

- This example needs PSRAM and a larger application partition; keep the provided `sdkconfig.defaults` and `partitions.csv`.
- Run `13_display_colorbar` first if the display does not light up.
- On ESP-IDF 5.5, the example suppresses the expected I2C NACK log while the BSP probes the alternate touch-controller address, then restores the previous log level.
