#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "board_variant.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "qmi8658.h"

#define BOARD_I2C_PORT I2C_NUM_0
#define BOARD_I2C_SCL_IO 14
#define BOARD_I2C_SDA_IO 15
#define IMU_SAMPLE_PERIOD_MS 200
#define IMU_PROBE_TIMEOUT_MS 100

static const char *TAG = "qmi8658_imu";

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

static esp_err_t qmi8658_detect_address(i2c_master_bus_handle_t bus_handle, uint8_t *address)
{
    const uint8_t candidates[] = {
        QMI8658_ADDRESS_HIGH,
        QMI8658_ADDRESS_LOW,
    };

    if (bus_handle == NULL || address == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        const uint8_t candidate = candidates[i];
        const esp_err_t ret = i2c_master_probe(bus_handle, candidate, IMU_PROBE_TIMEOUT_MS);
        if (ret == ESP_OK) {
            *address = candidate;
            return ESP_OK;
        }
    }

    return ESP_ERR_NOT_FOUND;
}

static esp_err_t qmi8658_configure(qmi8658_dev_t *imu)
{
    esp_err_t ret = qmi8658_set_accel_range(imu, QMI8658_ACCEL_RANGE_4G);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = qmi8658_set_accel_odr(imu, QMI8658_ACCEL_ODR_250HZ);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = qmi8658_set_gyro_range(imu, QMI8658_GYRO_RANGE_256DPS);
    if (ret != ESP_OK) {
        return ret;
    }

    ret = qmi8658_set_gyro_odr(imu, QMI8658_GYRO_ODR_250HZ);
    if (ret != ESP_OK) {
        return ret;
    }

    qmi8658_set_accel_unit_mps2(imu, true);
    qmi8658_set_gyro_unit_dps(imu, true);
    qmi8658_set_display_precision(imu, 3);

    return qmi8658_enable_sensors(imu, QMI8658_ENABLE_ACCEL | QMI8658_ENABLE_GYRO);
}

void app_main(void)
{
    const board_variant_t variant = board_variant_detect();
    ESP_LOGI(TAG, "Running on %s", board_variant_to_name(variant));
    ESP_LOGI(TAG, "QMI8658 component version %s", QMI8658_LIBRARY_VERSION);

    i2c_master_bus_handle_t bus_handle = NULL;
    esp_err_t ret = board_i2c_init(&bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed: %s", esp_err_to_name(ret));
        return;
    }

    uint8_t imu_address = 0;
    ret = qmi8658_detect_address(bus_handle, &imu_address);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "QMI8658 not found at 0x%02x or 0x%02x",
                 QMI8658_ADDRESS_HIGH, QMI8658_ADDRESS_LOW);
        return;
    }
    ESP_LOGI(TAG, "Detected QMI8658 at 0x%02x", imu_address);

    qmi8658_dev_t imu = {0};
    ret = qmi8658_init(&imu, bus_handle, imu_address);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "QMI8658 init failed: %s", esp_err_to_name(ret));
        return;
    }

    uint8_t who_am_i = 0;
    ret = qmi8658_get_who_am_i(&imu, &who_am_i);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "WHO_AM_I=0x%02x", who_am_i);
    }

    ret = qmi8658_configure(&imu);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "QMI8658 configuration failed: %s", esp_err_to_name(ret));
        return;
    }

    uint32_t waiting_samples = 0;
    while (true) {
        bool ready = false;
        ret = qmi8658_is_data_ready(&imu, &ready);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read data-ready status: %s", esp_err_to_name(ret));
            vTaskDelay(pdMS_TO_TICKS(IMU_SAMPLE_PERIOD_MS));
            continue;
        }

        if (!ready) {
            if ((waiting_samples++ % 20) == 0) {
                ESP_LOGW(TAG, "Waiting for IMU data-ready status");
            }
            vTaskDelay(pdMS_TO_TICKS(IMU_SAMPLE_PERIOD_MS));
            continue;
        }

        waiting_samples = 0;
        qmi8658_data_t data = {0};
        ret = qmi8658_read_sensor_data(&imu, &data);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG,
                     "accel[m/s^2] x=% .3f y=% .3f z=% .3f | "
                     "gyro[dps] x=% .3f y=% .3f z=% .3f | "
                     "temp=%.2f degC ts=%" PRIu32,
                     data.accelX,
                     data.accelY,
                     data.accelZ,
                     data.gyroX,
                     data.gyroY,
                     data.gyroZ,
                     data.temperature,
                     data.timestamp);
        } else {
            ESP_LOGE(TAG, "Failed to read sensor data: %s", esp_err_to_name(ret));
        }

        vTaskDelay(pdMS_TO_TICKS(IMU_SAMPLE_PERIOD_MS));
    }
}


