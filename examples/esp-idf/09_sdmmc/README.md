# 09_sdmmc

SDMMC/FAT filesystem example adapted from the ESP32-P4 platform example.

This port uses `bsp_sdcard_mount()` from the managed S3 BSP, writes files to the mounted card, renames one file, reads both files back, and unmounts the card.

## Hardware

- ESP32-S3-Touch-AMOLED-1.8 board
- USB data cable
- FAT-formatted microSD card

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for the board.

## Expected Result

The serial monitor reports the SD card mount, file write, rename, read-back, and unmount steps. If no card is present or the card cannot be mounted, the example logs the mount error.

## Notes

- Start with `00_board_check` if the BSP SD capability is not reported.
- Use a known-good FAT-formatted microSD card when debugging card-detect or mount failures.
