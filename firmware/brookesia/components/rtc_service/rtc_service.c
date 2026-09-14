#include "rtc_service.h"

#include <stddef.h>
#include <string.h>
#include <sys/time.h>

#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "pcf85063a.h"

static const char *TAG = "rtc_service";

static pcf85063a_dev_t s_rtc;
static rtc_service_status_t s_status = {
    .last_error = ESP_ERR_INVALID_STATE,
};

#define PCF85063A_DEVICE_YEAR_BASE 2000U

static bool is_leap_year(uint16_t year)
{
    return (year % 4U == 0U && year % 100U != 0U) || (year % 400U == 0U);
}

static uint8_t days_in_month(uint16_t year, uint8_t month)
{
    static const uint8_t days[] = {31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31};

    if (month == 2U) {
        return is_leap_year(year) ? 29U : 28U;
    }
    return days[month - 1U];
}

static bool is_valid_time(const rtc_service_time_t *time)
{
    return time->year >= 2020U && time->year <= 2069U &&
           time->month >= 1U && time->month <= 12U &&
           time->day >= 1U && time->day <= days_in_month(time->year, time->month) &&
           time->hour <= 23U && time->minute <= 59U && time->second <= 59U;
}

static int64_t to_unix_seconds(const rtc_service_time_t *time)
{
    int64_t days = 0;
    for (uint16_t year = 1970U; year < time->year; ++year) {
        days += is_leap_year(year) ? 366 : 365;
    }
    for (uint8_t month = 1U; month < time->month; ++month) {
        days += days_in_month(time->year, month);
    }
    days += time->day - 1U;
    return days * 86400LL + time->hour * 3600LL + time->minute * 60LL + time->second;
}

static esp_err_t set_system_time(const rtc_service_time_t *time)
{
    const int64_t seconds = to_unix_seconds(time);
    const time_t epoch = (time_t)seconds;
    if ((int64_t)epoch != seconds) {
        return ESP_ERR_INVALID_ARG;
    }

    const struct timeval tv = {
        .tv_sec = epoch,
        .tv_usec = 0,
    };
    return settimeofday(&tv, NULL) == 0 ? ESP_OK : ESP_FAIL;
}

static void copy_time(rtc_service_time_t *destination, const pcf85063a_datetime_t *source)
{
    /* Registry waveshare/pcf85063a 2.0.0 exposes YEAR_OFFSET=1970 even though
     * the PCF85063 year byte and this board's existing SensorLib use 2000 as
     * the century base. Recover the original 0..99 register value from the
     * component result, then apply the device contract locally. */
    const int32_t register_year = (int32_t)source->year - (int32_t)YEAR_OFFSET;
    destination->year = register_year >= 0 && register_year <= 99
                            ? (uint16_t)(PCF85063A_DEVICE_YEAR_BASE + register_year)
                            : source->year;
    destination->month = source->month;
    destination->day = source->day;
    destination->weekday = source->dotw;
    destination->hour = source->hour;
    destination->minute = source->min;
    destination->second = source->sec;
}

static esp_err_t remove_failed_init_device(esp_err_t init_error)
{
    if (s_rtc.dev_handle == NULL) {
        memset(&s_rtc, 0, sizeof(s_rtc));
        return init_error;
    }

    const esp_err_t cleanup_error = i2c_master_bus_rm_device(s_rtc.dev_handle);
    if (cleanup_error != ESP_OK) {
        ESP_LOGE(TAG, "PCF85063A initialization failed: %s; device cleanup also failed: %s",
                 esp_err_to_name(init_error), esp_err_to_name(cleanup_error));
        return cleanup_error;
    }

    memset(&s_rtc, 0, sizeof(s_rtc));
    return init_error;
}

esp_err_t rtc_service_init(void)
{
    if (!s_status.initialized) {
        if (s_rtc.dev_handle != NULL) {
            const esp_err_t cleanup_error = i2c_master_bus_rm_device(s_rtc.dev_handle);
            if (cleanup_error != ESP_OK) {
                s_status.last_error = cleanup_error;
                ESP_LOGE(TAG, "PCF85063A stale device cleanup failed: %s",
                         esp_err_to_name(cleanup_error));
                return s_status.last_error;
            }
            memset(&s_rtc, 0, sizeof(s_rtc));
        }

        i2c_master_bus_handle_t i2c_bus = bsp_i2c_get_handle();
        if (i2c_bus == NULL) {
            s_status.last_error = ESP_ERR_INVALID_STATE;
            return s_status.last_error;
        }

        s_status.last_error = pcf85063a_init(&s_rtc, i2c_bus, PCF85063A_ADDRESS);
        if (s_status.last_error != ESP_OK) {
            const esp_err_t init_error = s_status.last_error;
            s_status.last_error = remove_failed_init_device(init_error);
            if (s_status.last_error == init_error) {
                ESP_LOGW(TAG, "PCF85063A initialization failed: %s", esp_err_to_name(init_error));
            }
            return s_status.last_error;
        }
        s_status.initialized = true;
    }

    return rtc_service_read_and_sync_system_time();
}

esp_err_t rtc_service_read_and_sync_system_time(void)
{
    if (!s_status.initialized) {
        s_status.last_error = ESP_ERR_INVALID_STATE;
        return s_status.last_error;
    }

    uint8_t raw_seconds = 0;
    s_status.last_error = pcf85063a_read_register(&s_rtc, PCF85063A_RTC_SECOND_ADDR,
                                                   &raw_seconds, sizeof(raw_seconds));
    if (s_status.last_error != ESP_OK) {
        s_status.time_valid = false;
        return s_status.last_error;
    }
    if ((raw_seconds & 0x80U) != 0U) {
        s_status.time_valid = false;
        s_status.last_error = ESP_ERR_INVALID_STATE;
        return s_status.last_error;
    }

    pcf85063a_datetime_t rtc_time = {0};
    s_status.last_error = pcf85063a_get_time_date(&s_rtc, &rtc_time);
    if (s_status.last_error != ESP_OK) {
        s_status.time_valid = false;
        return s_status.last_error;
    }

    copy_time(&s_status.time, &rtc_time);
    s_status.time_valid = is_valid_time(&s_status.time);
    if (!s_status.time_valid) {
        s_status.last_error = ESP_ERR_INVALID_STATE;
        return s_status.last_error;
    }

    s_status.last_error = set_system_time(&s_status.time);
    if (s_status.last_error == ESP_OK) {
        ESP_LOGI(TAG, "System time synchronized from RTC: %04u-%02u-%02u %02u:%02u:%02u",
                 s_status.time.year, s_status.time.month, s_status.time.day,
                 s_status.time.hour, s_status.time.minute, s_status.time.second);
    }
    return s_status.last_error;
}

esp_err_t rtc_service_get_status(rtc_service_status_t *status)
{
    if (status == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    *status = s_status;
    return ESP_OK;
}

esp_err_t rtc_service_get_last_error(void)
{
    return s_status.last_error;
}
