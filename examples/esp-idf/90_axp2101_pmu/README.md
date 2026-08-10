# 90_axp2101_pmu

[简体中文](README_ZH.md)

AXP2101 PMU diagnostic for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

This is a legacy local-component example. It is kept in the `90_` range because normal application projects should start from the managed online BSP examples instead of copying low-level PMU driver code.

## What It Checks

- Detects the board display/touch variant through the local `board_variant` helper.
- Opens the onboard I2C bus used by the PMU.
- Initializes AXP2101 through the bundled XPowersLib port.
- Logs PMU regulator state and handles PMU interrupt polling.

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the board serial port.

## Notes

- This diagnostic carries local component copies for PMU bring-up. It is not the recommended template for new projects.
- For new applications, use `01_project_template` and the managed `waveshare/esp32_s3_touch_amoled_1_8` BSP dependency.
- Do not change PMU output settings unless the connected load voltage requirements are known.
