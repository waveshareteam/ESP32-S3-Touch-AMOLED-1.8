# 08_i2c_tools

I2C scan tool adapted from the ESP32-P4 platform example.

By default it scans the board I2C bus used by PMU, touch, RTC/audio control, and other onboard devices. Use `idf.py menuconfig` to choose an external I2C bus if needed.

## Hardware

- ESP32-S3-Touch-AMOLED-1.8 board
- USB data cable
- Optional external I2C devices

## Configuration

The default settings scan the onboard I2C bus. For external devices, run `idf.py menuconfig` and choose the SDA, SCL, pull-up, and scan range settings for the external bus.

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for the board.

## Expected Result

The serial monitor prints detected I2C addresses. Use this before debugging PMU, RTC, touch, IMU, or codec-control failures.

## Notes

I2C address presence does not prove the full device driver is configured correctly, but it confirms that the bus and pull-ups are basically working.
