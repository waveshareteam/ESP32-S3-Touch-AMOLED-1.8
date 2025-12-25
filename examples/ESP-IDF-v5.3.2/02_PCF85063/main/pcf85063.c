#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"

// I2C configuration
#define I2C_MASTER_SCL_IO    14        // Set the SCL pin
#define I2C_MASTER_SDA_IO    15        // Set the SDA pin
#define I2C_MASTER_NUM       I2C_NUM_0 // I2C port number
#define I2C_MASTER_FREQ_HZ   100000    // I2C frequency
#define I2C_MASTER_TX_BUF_DISABLE 0    // Disable tx buffer
#define I2C_MASTER_RX_BUF_DISABLE 0    // Disable rx buffer
#define I2C_TIMEOUT_MS       1000

#define PCF85063_ADDR        0x51      // PCF85063 I2C address

static const char *TAG = "PCF85063";

// Helper function to initiate I2C
esp_err_t i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C configuration failed");
        return err;
    }
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

// Read RTC register function
esp_err_t rtc_read_reg(uint8_t reg_addr, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PCF85063_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PCF85063_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Write RTC register function
esp_err_t rtc_write_reg(uint8_t reg_addr, uint8_t *data, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PCF85063_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
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
