#include <stdbool.h>
#include <stdint.h>

#include "board_variant.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pcf85063a.h"

#define BOARD_I2C_PORT I2C_NUM_0
#define BOARD_I2C_SCL_IO 14
#define BOARD_I2C_SDA_IO 15
#define RTC_READ_PERIOD_MS 1000
#define RTC_PROBE_TIMEOUT_MS 100
#define RTC_SET_SAMPLE_TIME_WHEN_UNSET 1

static const char *TAG = "pcf85063_rtc";

static const pcf85063a_datetime_t s_sample_time = {
    .year = 2026,
    .month = 1,
    .day = 1,
    .dotw = 4,
    .hour = 0,
    .min = 0,
    .sec = 0,
};

static esp_err_t board_i2c_init(i2c_master_bus_handle_t *bus_handle)
{
    if (bus_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const i2c_master_bus_config_t bus_config = {
        .i2c_port = BOARD_I2C_PORT,
        .sda_io_num = BOARD_I2C_SDA_IO,
        .scl_io_num = BOARD_I2C_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

    return i2c_new_master_bus(&bus_config, bus_handle);
}

static bool rtc_datetime_is_reasonable(const pcf85063a_datetime_t *time)
{
    return time != NULL &&
           time->year >= 2024 && time->year <= 2069 &&
           time->month >= 1 && time->month <= 12 &&
           time->day >= 1 && time->day <= 31 &&
           time->dotw <= 6 &&
           time->hour <= 23 &&
           time->min <= 59 &&
           time->sec <= 59;
}

static void rtc_log_time(const char *label, const pcf85063a_datetime_t *time)
{
    ESP_LOGI(TAG, "%s: %04u-%02u-%02u %02u:%02u:%02u dotw=%u",
             label,
             (unsigned)time->year,
             (unsigned)time->month,
             (unsigned)time->day,
             (unsigned)time->hour,
             (unsigned)time->min,
             (unsigned)time->sec,
             (unsigned)time->dotw);
}

static esp_err_t rtc_seed_if_unset(pcf85063a_dev_t *rtc)
{
    pcf85063a_datetime_t current_time = {0};
    esp_err_t ret = pcf85063a_get_time_date(rtc, &current_time);
    if (ret != ESP_OK) {
        return ret;
    }

    if (rtc_datetime_is_reasonable(&current_time)) {
        rtc_log_time("Current RTC time", &current_time);
        return ESP_OK;
    }

    ESP_LOGW(TAG, "RTC time looks unset or invalid");

#if RTC_SET_SAMPLE_TIME_WHEN_UNSET
    rtc_log_time("Writing sample RTC time", &s_sample_time);
    ret = pcf85063a_set_time_date(rtc, s_sample_time);
    if (ret != ESP_OK) {
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
    return pcf85063a_get_time_date(rtc, &current_time);
#else
    return ESP_OK;
#endif
}

void app_main(void)
{
    const board_variant_t variant = board_variant_detect();
    ESP_LOGI(TAG, "Running on %s", board_variant_to_name(variant));
    ESP_LOGI(TAG, "PCF85063A component version %s", PCF85063A_LIBRARY_VERSION);

    i2c_master_bus_handle_t bus_handle = NULL;
    esp_err_t ret = board_i2c_init(&bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = i2c_master_probe(bus_handle, PCF85063A_ADDRESS, RTC_PROBE_TIMEOUT_MS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCF85063A not found at 0x%02x: %s",
                 PCF85063A_ADDRESS, esp_err_to_name(ret));
        return;
    }

    pcf85063a_dev_t rtc = {0};
    ret = pcf85063a_init(&rtc, bus_handle, PCF85063A_ADDRESS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCF85063A init failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = rtc_seed_if_unset(&rtc);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "RTC startup check failed: %s", esp_err_to_name(ret));
        return;
    }

    while (true) {
        pcf85063a_datetime_t current_time = {0};
        ret = pcf85063a_get_time_date(&rtc, &current_time);
        if (ret == ESP_OK) {
            rtc_log_time("RTC", &current_time);
        } else {
            ESP_LOGE(TAG, "Failed to read RTC time: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(RTC_READ_PERIOD_MS));
    }
}
