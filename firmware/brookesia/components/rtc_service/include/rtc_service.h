#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} rtc_service_time_t;

typedef struct {
    bool initialized;
    bool time_valid;
    rtc_service_time_t time;
    esp_err_t last_error;
} rtc_service_status_t;

/**
 * @brief Initialize the PCF85063A on the board BSP I2C bus and synchronize
 *        system time if the RTC contains a valid calendar value.
 */
esp_err_t rtc_service_init(void);

/**
 * @brief Read the RTC and synchronize system time when the calendar value is valid.
 */
esp_err_t rtc_service_read_and_sync_system_time(void);

/**
 * @brief Return the last RTC state observed by this service.
 */
esp_err_t rtc_service_get_status(rtc_service_status_t *status);

/**
 * @brief Return the latest initialization, read, validation, or clock-set error.
 */
esp_err_t rtc_service_get_last_error(void);

#ifdef __cplusplus
}
#endif
