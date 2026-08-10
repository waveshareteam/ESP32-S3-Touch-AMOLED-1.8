# 10_wifi_station

[简体中文](README_ZH.md)

Wi-Fi station demo adapted from the ESP32-P4 platform example for ESP32-S3.

Configure SSID and password with `idf.py menuconfig`, then build and flash.

## Hardware

- ESP32-S3-Touch-AMOLED-1.8 board
- USB data cable
- 2.4 GHz Wi-Fi access point

## Configuration

Run `idf.py menuconfig` and set:

- Wi-Fi SSID
- Wi-Fi password
- Optional retry count

Do not commit Wi-Fi credentials in `sdkconfig` or documentation.

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for the board.

## Expected Result

The serial monitor reports Wi-Fi initialization, connection progress, and the acquired IP address.

## Notes

ESP32-S3 station mode uses 2.4 GHz Wi-Fi. Check AP band settings if the board cannot see or join the network.
