#include "rtc_service.h"

#include <stddef.h>
#include <stdio.h>
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
/* Below this the system clock is still the 1970 epoch and is not a time source. */
#define MINIMUM_VALID_EPOCH 1577836800 /* 2020-01-01 */
/* Rewrite the RTC only when it disagrees with the system clock by more than the
 * RTC's own one-second granularity plus I2C latency. */
#define RTC_SYNC_THRESHOLD_SECONDS 5

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

/* Inverse of copy_time(): the device register counts years from 2000 while the
 * component writes year - YEAR_OFFSET, so shift the century base back. */
static uint16_t to_component_year(uint16_t year)
{
    return (uint16_t)(YEAR_OFFSET + (year - PCF85063A_DEVICE_YEAR_BASE));
}

static uint8_t weekday_from_epoch(int64_t seconds)
{
    /* 1970-01-01 was a Thursday, so day 0 maps to dotw 4 with Sunday == 0. */
    const int64_t days = seconds / 86400LL;
    return (uint8_t)(((days % 7) + 11) % 7);
}

/* The board has neither a network time source nor a guaranteed RTC backup, so a
 * factory-fresh board reports the oscillator-stop flag and an invalid calendar.
 * In that case seed the PCF85063A from the firmware build timestamp: the Clock
 * application and the status-bar clock then run instead of showing 1970, and
 * the RTC keeps real elapsed time from that point on. Writing the seconds
 * register also clears the oscillator-stop flag. */
static esp_err_t seed_rtc_from_build_time(void)
{
    static const char *const MONTHS[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };

    char month_name[4] = {};
    int day = 0;
    int year = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    if (sscanf(__DATE__, "%3s %d %d", month_name, &day, &year) != 3 ||
            sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second) != 3) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t month = 0;
    for (uint8_t i = 0; i < 12U; ++i) {
        if (strcmp(month_name, MONTHS[i]) == 0) {
            month = (uint8_t)(i + 1U);
            break;
        }
    }

    rtc_service_time_t build_time = {
        .year = (uint16_t)year,
        .month = month,
        .day = (uint8_t)day,
        .weekday = 0,
        .hour = (uint8_t)hour,
        .minute = (uint8_t)minute,
        .second = (uint8_t)second,
    };
    if (!is_valid_time(&build_time)) {
        return ESP_ERR_INVALID_ARG;
    }
    build_time.weekday = weekday_from_epoch(to_unix_seconds(&build_time));

    const pcf85063a_datetime_t device_time = {
        .year = to_component_year(build_time.year),
        .month = build_time.month,
        .day = build_time.day,
        .dotw = build_time.weekday,
        .hour = build_time.hour,
        .min = build_time.minute,
        .sec = build_time.second,
    };
    esp_err_t result = pcf85063a_set_time_date(&s_rtc, device_time);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Seeding the PCF85063A failed: %s", esp_err_to_name(result));
        return result;
    }

    s_status.time = build_time;
    s_status.time_valid = true;
    result = set_system_time(&build_time);
    if (result == ESP_OK) {
        ESP_LOGW(TAG,
                 "RTC had no valid time; seeded from the firmware build timestamp: "
                 "%04u-%02u-%02u %02u:%02u:%02u UTC",
                 build_time.year, build_time.month, build_time.day,
                 build_time.hour, build_time.minute, build_time.second);
    }
    return result;
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

    esp_err_t result = rtc_service_read_and_sync_system_time();
    if (result != ESP_OK) {
        /* A first-boot board, or one whose RTC backup was lost, has no usable
         * calendar yet. Seed it instead of leaving the system clock at 1970. */
        const esp_err_t seed_result = seed_rtc_from_build_time();
        if (seed_result == ESP_OK) {
            return ESP_OK;
        }
        ESP_LOGW(TAG, "RTC unusable and seeding failed: %s", esp_err_to_name(seed_result));
        return result;
    }
    return result;
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
        /* Bit 7 of the seconds register is the oscillator-stop flag: the RTC has
         * lost its time, typically a board that has never been set. */
        ESP_LOGW(TAG, "PCF85063A reports a stopped oscillator (seconds reg 0x%02x)",
                 raw_seconds);
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
        ESP_LOGW(TAG, "PCF85063A calendar is out of range: %04u-%02u-%02u %02u:%02u:%02u",
                 s_status.time.year, s_status.time.month, s_status.time.day,
                 s_status.time.hour, s_status.time.minute, s_status.time.second);
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

esp_err_t rtc_service_sync_from_system_time(void)
{
    if (!s_status.initialized) {
        s_status.last_error = ESP_ERR_INVALID_STATE;
        return s_status.last_error;
    }

    /* The system clock carries local time (see the esp_xiaozhi server_time
     * handling), so the RTC is kept in the same frame of reference. */
    const time_t now = time(NULL);
    struct tm local = {};
    if (now < (time_t)MINIMUM_VALID_EPOCH || localtime_r(&now, &local) == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    const rtc_service_time_t system_time = {
        .year = (uint16_t)(local.tm_year + 1900),
        .month = (uint8_t)(local.tm_mon + 1),
        .day = (uint8_t)local.tm_mday,
        .weekday = (uint8_t)local.tm_wday,
        .hour = (uint8_t)local.tm_hour,
        .minute = (uint8_t)local.tm_min,
        .second = (uint8_t)local.tm_sec,
    };
    if (!is_valid_time(&system_time)) {
        return ESP_ERR_INVALID_ARG;
    }

    /* Read first: the RTC already advanced past the last synchronized sample, so
     * comparing against a cached value would rewrite the registers every call. */
    uint8_t raw_seconds = 0;
    esp_err_t result = pcf85063a_read_register(&s_rtc, PCF85063A_RTC_SECOND_ADDR,
                                               &raw_seconds, sizeof(raw_seconds));
    if (result == ESP_OK && (raw_seconds & 0x80U) == 0U) {
        pcf85063a_datetime_t rtc_time = {0};
        result = pcf85063a_get_time_date(&s_rtc, &rtc_time);
        if (result == ESP_OK) {
            rtc_service_time_t current = {};
            copy_time(&current, &rtc_time);
            if (is_valid_time(&current)) {
                const int64_t delta = to_unix_seconds(&system_time) - to_unix_seconds(&current);
                if (delta <= RTC_SYNC_THRESHOLD_SECONDS && delta >= -RTC_SYNC_THRESHOLD_SECONDS) {
                    return ESP_OK;
                }
            }
        }
    }

    const pcf85063a_datetime_t device_time = {
        .year = to_component_year(system_time.year),
        .month = system_time.month,
        .day = system_time.day,
        .dotw = system_time.weekday,
        .hour = system_time.hour,
        .min = system_time.minute,
        .sec = system_time.second,
    };
    result = pcf85063a_set_time_date(&s_rtc, device_time);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Writing the system time back to the RTC failed: %s",
                 esp_err_to_name(result));
        s_status.last_error = result;
        return result;
    }

    s_status.time = system_time;
    s_status.time_valid = true;
    s_status.last_error = ESP_OK;
    ESP_LOGI(TAG, "PCF85063A updated from the system clock: %04u-%02u-%02u %02u:%02u:%02u",
             system_time.year, system_time.month, system_time.day,
             system_time.hour, system_time.minute, system_time.second);
    return ESP_OK;
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
