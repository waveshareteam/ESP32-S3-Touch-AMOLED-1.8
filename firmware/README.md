# Factory firmware

Match the hardware revision: V1 uses SH8601 / FT3168; V2 uses CO5300 / CST820. Do not flash an image for the other revision.

| File | Version date | Hardware | Purpose | Flash offset |
| --- | --- | --- | --- | --- |
| [v2/ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20260915.bin](v2/ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20260915.bin) | 2026-09-15 | V2 | ESP-Brookesia factory recovery | `0x0` |
| [v1/ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20260915.bin](v1/ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20260915.bin) | 2026-09-15 | V1 | ESP-Brookesia factory recovery | `0x0` |

The original images are retained. Each canonical copy has the same SHA-256 as its corresponding original. The firmware center registers the two revisions separately through [manifest.json](manifest.json).

| Original file | Hardware | SHA-256 |
| --- | --- | --- |
| [ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin](ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin) | V2 | `4621d61def9ec75903e447668d53e0f3d6d50317193dbfe4c25308502ba66033` |
| [ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin](ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin) | V1 | `c811100a0cc8ca4cf94eb1c5d1b9fc5e320954b70888f945a21098e4020f47d7` |

## Flashing

ESP32-S3, 16MB Flash; use esptool v5. Replace `COMx` with the serial port and select the command matching your hardware.

```powershell
python -m esptool --chip esp32s3 --port COMx write-flash 0x0 firmware/v1/ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20260915.bin
python -m esptool --chip esp32s3 --port COMx write-flash 0x0 firmware/v2/ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20260915.bin
```

The maintainer confirmed physical verification on both revisions. See [firmware notes](../docs/FIRMWARE.md) for detailed coverage and known limitations.
