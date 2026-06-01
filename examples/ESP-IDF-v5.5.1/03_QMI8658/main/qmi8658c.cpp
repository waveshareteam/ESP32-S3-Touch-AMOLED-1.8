#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "SensorLib.h"
#include "SensorQMI8658.hpp"  // Ensure this path is correct
#include "board_variant.h"

// I2C configuration
#define I2C_MASTER_SCL 14
#define I2C_MASTER_SDA 15
#define I2C_MASTER_NUM I2C_NUM_0
#define QMI8658_ADDRESS 0x6B // Replace with your QMI8658 address

SensorQMI8658 qmi;
IMUdata acc;
IMUdata gyr;

static const char *TAG = "QMI8658"; // Define a tag for logging
static i2c_master_bus_handle_t i2c_bus_handle = NULL;

esp_err_t i2c_master_init() {
    i2c_master_bus_config_t conf = {};
    conf.i2c_port = I2C_MASTER_NUM;
    conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA;
    conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL;
    conf.clk_source = I2C_CLK_SRC_DEFAULT;
    conf.glitch_ignore_cnt = 7;
    conf.flags.enable_internal_pullup = true;
    return i2c_new_master_bus(&conf, &i2c_bus_handle);
}

void read_sensor_data(void* arg); // Function declaration

void setup_sensor() {
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);
    }

    // Initialize QMI8658 sensor on the shared ESP-IDF 5.x I2C master bus.
    if (!qmi.begin(i2c_bus_handle, QMI8658_ADDRESS)) {
        ESP_LOGE(TAG, "Failed to find QMI8658 - check your wiring!");
        vTaskDelete(NULL); // Handle error gracefully
    }

    // Get chip ID
    ESP_LOGI(TAG, "Device ID: %x", qmi.getChipID());

    // Configure accelerometer
    qmi.configAccelerometer(
        SensorQMI8658::ACC_RANGE_4G,
        SensorQMI8658::ACC_ODR_1000Hz,
        SensorQMI8658::LPF_MODE_0,
        true
    );

    // Configure gyroscope
    qmi.configGyroscope(
        SensorQMI8658::GYR_RANGE_64DPS,
        SensorQMI8658::GYR_ODR_896_8Hz,
        SensorQMI8658::LPF_MODE_3,
        true
    );

    // Enable gyroscope and accelerometer
    qmi.enableGyroscope();
    qmi.enableAccelerometer();

    ESP_LOGI(TAG, "Ready to read data...");
}


extern "C" void app_main() {
    board_variant_t variant = board_variant_detect();
    ESP_LOGI(TAG, "Running on %s", board_variant_to_name(variant));

    setup_sensor();
    xTaskCreate(read_sensor_data, "sensor_read_task", 4096, NULL, 10, NULL);
}

void read_sensor_data(void* arg) {
    while (1) {
        if (qmi.getDataReady()) {
            if (qmi.getAccelerometer(acc.x, acc.y, acc.z)) {
                ESP_LOGI(TAG, "ACCEL: %f, %f, %f", acc.x, acc.y, acc.z);
            } else {
                ESP_LOGE(TAG, "Failed to read accelerometer data");
            }

            if (qmi.getGyroscope(gyr.x, gyr.y, gyr.z)) {
                ESP_LOGI(TAG, "GYRO: %f, %f, %f", gyr.x, gyr.y, gyr.z);
            } else {
                ESP_LOGE(TAG, "Failed to read gyroscope data");
            }

            ESP_LOGI(TAG, "Timestamp: %u, Temperature: %.2f *C", (unsigned int)qmi.getTimestamp(), qmi.getTemperature_C()); // Casting to unsigned int
        } else {
            ESP_LOGW(TAG, "Data not ready yet");
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
