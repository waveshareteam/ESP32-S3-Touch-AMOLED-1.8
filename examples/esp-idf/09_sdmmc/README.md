# 09_sdmmc

SDMMC/FAT filesystem example adapted from the ESP32-P4 platform example.

This port uses `bsp_sdcard_mount()` from the managed S3 BSP, writes files to the mounted card, renames one file, reads both files back, and unmounts the card.
