# 92_qmi8658_imu

QMI8658 IMU diagnostic for the Waveshare ESP32-S3-Touch-AMOLED-1.8 board.

This is a legacy local-component example. It is kept for direct accelerometer and gyroscope bring-up with the bundled SensorLib source.

## What It Checks

- Detects the board display/touch variant through the local `board_variant` helper.
- Opens the onboard I2C bus on SDA 15 and SCL 14.
- Initializes QMI8658 at address `0x6B`.
- Logs accelerometer, gyroscope, temperature, and timestamp data.

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the board serial port.

## Notes

- This diagnostic carries local SensorLib and board-variant helper code. It is not intended as a new-project template.
- Use `08_i2c_tools` first if QMI8658 is not detected, then use this example for sensor-specific checks.