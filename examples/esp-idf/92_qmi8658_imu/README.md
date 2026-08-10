# 92_qmi8658_imu

[简体中文](README_ZH.md)

QMI8658 IMU diagnostic for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

This example uses the managed `waveshare/qmi8658` component instead of the older checked-in driver copy. It keeps the code small and focused on the board-level IMU bring-up path.

## What It Checks

- Detects the board display/touch variant through the local `board_variant` helper.
- Opens the onboard I2C bus on SDA 15 and SCL 14.
- Probes QMI8658 address `0x6B`, then `0x6A` as a fallback.
- Reads and logs the `WHO_AM_I` register.
- Performs the datasheet soft-reset sequence before configuring the sensor.
- Configures accelerometer range `4G` at `250 Hz`.
- Configures gyroscope range `256 dps` at `250 Hz`.
- Logs accelerometer, gyroscope, temperature, and timestamp data every 200 ms when data is ready.

## Managed Component

The dependency is declared in [main/idf_component.yml](main/idf_component.yml):

```yaml
dependencies:
  idf: ">=5.5,<6.1"
  waveshare/qmi8658:
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

- Use `08_i2c_tools` first if QMI8658 is not detected.
- The example reports acceleration in `m/s^2` and angular rate in `dps`.
- Recheck stationary readings on both board variants after changing the reset or range configuration; acceleration should be close to one gravity on the vertical axis and gyroscope values should remain near zero.
- For new application structure, start from `01_project_template` and add only the sensor components you need.
