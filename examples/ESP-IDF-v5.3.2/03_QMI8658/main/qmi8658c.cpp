#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "driver/i2c.h"
#include "SensorLib.h"
#include "SensorQMI8658.hpp"  // Ensure this path is correct

// I2C configuration
#define I2C_MASTER_SCL 14
#define I2C_MASTER_SDA 15
#define I2C_MASTER_NUM I2C_NUM_0
#define QMI8658_ADDRESS 0x6B // Replace with your QMI8658 address

SensorQMI8658 qmi;
IMUdata acc;
IMUdata gyr;

static const char *TAG = "QMI8658"; // Define a tag for logging

void i2c_master_init() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000; // I2C clock frequency
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void read_sensor_data(void* arg); // Function declaration

void setup_sensor() {
    i2c_master_init();

    // Initialize QMI8658 sensor with 4 parameters (port number, address, SDA, SCL)
    if (!qmi.begin(I2C_MASTER_NUM, QMI8658_ADDRESS, I2C_MASTER_SDA, I2C_MASTER_SCL)) {
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
