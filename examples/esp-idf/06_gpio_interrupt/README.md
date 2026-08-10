# 06_gpio_interrupt

[简体中文](README_ZH.md)

GPIO interrupt demo adapted from the ESP32-P4 platform example.

The default configuration is a safe dry-run. Use `idf.py menuconfig` to choose an exposed, unused input GPIO before testing interrupts.

## Hardware

- ESP32-S3-Touch-AMOLED-1.8 board
- USB data cable
- Optional jumper wire, button, or external signal source

## Configuration

Run `idf.py menuconfig` and set the input GPIO and interrupt edge. Choose a pin that is exposed and not reserved by onboard peripherals.

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for the board.

## Expected Result

The serial monitor reports interrupt counts when the configured input changes according to the selected edge.

## Safety Notes

Keep external signals within the board I/O voltage limits. Do not attach an external signal to a reserved board pin.
