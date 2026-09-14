cmake_minimum_required(VERSION 3.16)

message(FATAL_ERROR
    "ESP32-S3-Touch-AMOLED-1.8 has two explicit firmware profiles. "
    "Build firmware/brookesia/original for SH8601/FT3168 boards or "
    "firmware/brookesia/v2 for CO5300/CST820 boards.")
