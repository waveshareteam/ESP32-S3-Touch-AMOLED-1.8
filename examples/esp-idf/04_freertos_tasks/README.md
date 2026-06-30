# 04_freertos_tasks

FreeRTOS task and queue demo adapted from the ESP32-P4 platform example.

The producer task sends uptime messages to a queue. The consumer task prints them.

## Hardware

- ESP32-S3-Touch-AMOLED-1.8 board
- USB data cable

## Build and Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for the board.

## Expected Result

The serial monitor shows messages moving from the producer task to the consumer task through a FreeRTOS queue.

## Notes

Use this example as a small pattern for separating periodic sampling, UI updates, and logging work into different tasks.
