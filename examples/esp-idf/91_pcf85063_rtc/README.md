# 91_pcf85063_rtc

PCF85063 RTC diagnostic for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

This is a legacy local-component example. It remains available for direct RTC register checks, while the main ESP-IDF path uses the managed online BSP component.

## What It Checks

- Detects the board display/touch variant through the local `board_variant` helper.
- Opens the onboard I2C bus on SDA 15 and SCL 14.
- Writes a sample time to PCF85063 address `0x51`.
- Reads and logs the RTC time once per second.

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the board serial port.

## Notes

- The example writes a fixed sample time at startup. Adjust `rtc_set_time()` in `main/pcf85063.c` before using it for real timekeeping tests.
- For new application structure, start from `01_project_template` and use the managed BSP dependency.