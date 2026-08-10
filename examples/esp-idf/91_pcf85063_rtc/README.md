# 91_pcf85063_rtc

[简体中文](README_ZH.md)

PCF85063A RTC diagnostic for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

This example uses the managed `waveshare/pcf85063a` component instead of a checked-in RTC driver copy. It is intended for low-level RTC bring-up after the board I2C bus has been verified.

## What It Checks

- Detects the board display/touch variant through the local `board_variant` helper.
- Opens the onboard I2C bus on SDA 15 and SCL 14.
- Probes the PCF85063A at address `0x51`.
- Initializes the RTC through the managed Waveshare component.
- Reads and logs the RTC date/time once per second.
- Writes the sample time `2026-01-01 00:00:00` only when the current RTC contents look invalid.

## Managed Component

The dependency is declared in [main/idf_component.yml](main/idf_component.yml):

```yaml
dependencies:
  idf: ">=5.5,<6.1"
  waveshare/pcf85063a:
    version: "^2.0.0"
    public: true
```

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the board serial port.

## Notes

- Use `08_i2c_tools` first if the RTC is not detected.
- Set `RTC_SET_SAMPLE_TIME_WHEN_UNSET` to `0` in [main/pcf85063_rtc.c](main/pcf85063_rtc.c) when you want a read-only RTC check.
- For new application structure, start from `01_project_template` and add only the sensor components you need.
