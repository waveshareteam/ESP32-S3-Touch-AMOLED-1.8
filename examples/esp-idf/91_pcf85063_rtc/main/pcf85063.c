#include <stdio.h>
#include <string.h>
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "board_variant.h"

// I2C configuration
#define I2C_MASTER_SCL_IO    14        // Set the SCL pin
#define I2C_MASTER_SDA_IO    15        // Set the SDA pin
#define I2C_MASTER_NUM       I2C_NUM_0 // I2C port number
#define I2C_MASTER_FREQ_HZ   100000    // I2C frequency
#define I2C_TIMEOUT_MS       1000

#define PCF85063_ADDR        0x51      // PCF85063 I2C address

static const char *TAG = "PCF85063";
static i2c_master_bus_handle_t i2c_bus = NULL;
static i2c_master_dev_handle_t rtc_dev = NULL;

// Helper function to initiate I2C
esp_err_t i2c_master_init(void) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

    esp_err_t err = i2c_new_master_bus(&bus_config, &i2c_bus);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed: %s", esp_err_to_name(err));
        return err;
    }

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = PCF85063_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    err = i2c_master_bus_add_device(i2c_bus, &dev_config, &rtc_dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "RTC device add failed: %s", esp_err_to_name(err));
    }
    return err;
}

// Read RTC register function
esp_err_t rtc_read_reg(uint8_t reg_addr, uint8_t *data, size_t len) {
    return i2c_master_transmit_receive(rtc_dev, &reg_addr, 1, data, len, I2C_TIMEOUT_MS);
}

// Write RTC register function
esp_err_t rtc_write_reg(uint8_t reg_addr, uint8_t *data, size_t len) {
    uint8_t write_buf[16] = {0};
    if (len + 1 > sizeof(write_buf)) {
        return ESP_ERR_INVALID_SIZE;
    }

    write_buf[0] = reg_addr;
    memcpy(&write_buf[1], data, len);
    return i2c_master_transmit(rtc_dev, write_buf, len + 1, I2C_TIMEOUT_MS);
}

// BCD to decimal conversion
uint8_t bcd_to_dec(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0F);
}

// Decimal to BCD conversion
uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

// Function to get current time from the RTC
esp_err_t rtc_get_time(void) {
    uint8_t data[7];
    esp_err_t ret = rtc_read_reg(0x04, data, 7); // PCF85063 time registers start from 0x04
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read time registers");
        return ret;
    }

    uint8_t seconds = bcd_to_dec(data[0] & 0x7F); // Masking out the stop bit
    uint8_t minutes = bcd_to_dec(data[1]);
    uint8_t hours = bcd_to_dec(data[2]);
    uint8_t day = bcd_to_dec(data[3]);
    uint8_t weekday = bcd_to_dec(data[4]);
    uint8_t month = bcd_to_dec(data[5] & 0x1F); // Masking out the century bit
    uint8_t year = bcd_to_dec(data[6]);

    ESP_LOGI(TAG, "Current Time: %02d:%02d:%02d %02d/%02d/%04d", hours, minutes, seconds, day, month, 2000 + year);
    return ESP_OK;
}

// Function to set time on the RTC
esp_err_t rtc_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t day, uint8_t month, uint8_t year) {
    uint8_t data[7] = {
        dec_to_bcd(seconds),
        dec_to_bcd(minutes),
        dec_to_bcd(hours),
        dec_to_bcd(day),
        dec_to_bcd(0), // Weekday, you can set this if needed
        dec_to_bcd(month),
        dec_to_bcd(year)
    };
    return rtc_write_reg(0x04, data, 7);
}

void app_main(void) {
    board_variant_t variant = board_variant_detect();
    ESP_LOGI(TAG, "Running on %s", board_variant_to_name(variant));

    // Initialize I2C
    esp_err_t err = i2c_master_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed");
        return;
    }

    // Set RTC time (Example: 12:30:45, 27th September 2024)
    err = rtc_set_time(12, 30, 45, 27, 9, 24); // Set the time
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Time set successfully");
    } else {
        ESP_LOGE(TAG, "Failed to set time");
    }

    while (1) {
        // Get and log the current time
        err = rtc_get_time();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to get time");
        }

        // Delay for 1 second
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
