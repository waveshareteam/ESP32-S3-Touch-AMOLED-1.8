# 出厂固件

硬件版本必须与固件匹配：V1 使用 SH8601 / FT3168，V2 使用 CO5300 / CST820。两个版本不可混烧。

| 文件 | 版本日期 | 适用硬件 | 用途 | 烧录偏移 |
| --- | --- | --- | --- | --- |
| [ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20261118.bin](ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20261118.bin) | 2026-11-18 | V2 | ESP-Brookesia 出厂恢复 | `0x0` |
| [ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20260915.bin](ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20260915.bin) | 2026-09-15 | V1 | ESP-Brookesia 出厂恢复 | `0x0` |

原始镜像保留，规范命名副本与对应原文件的 SHA-256 完全一致。固件中心通过 [manifest.json](manifest.json) 分别登记两个硬件版本。

| 原文件 | 适用硬件 | SHA-256 |
| --- | --- | --- |
| [ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin](ESP32-S3-Touch-AMOLED-1.8-V2-FactoryOnly-260915.bin) | V2 | `4621d61def9ec75903e447668d53e0f3d6d50317193dbfe4c25308502ba66033` |
| [ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin](ESP32-S3-Touch-AMOLED-1.8-FactoryOnly-260915.bin) | V1 | `c811100a0cc8ca4cf94eb1c5d1b9fc5e320954b70888f945a21098e4020f47d7` |

V2 的 2026-11-18 是本次规范整理的版本日期，原始镜像标记为 260915；未重新编译或修改 BIN 内容。

## 烧录

ESP32-S3，16MB Flash；使用 esptool v5。将 `COMx` 替换为实际串口，按硬件版本选择一条命令。

```powershell
python -m esptool --chip esp32s3 --port COMx write-flash 0x0 firmware/ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20260915.bin
python -m esptool --chip esp32s3 --port COMx write-flash 0x0 firmware/ESP32-S3-Touch-AMOLED-1.8_FactoryOnly_20261118.bin
```

两个版本均由维护者确认已在对应实物上验证。详细的已验证范围与已知限制见 [固件说明](../docs/FIRMWARE_ZH.md)。
