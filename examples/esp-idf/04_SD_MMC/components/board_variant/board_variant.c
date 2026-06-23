#include "board_variant.h"

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "board_variant";

#define BOARD_I2C_PORT          I2C_NUM_0
#define BOARD_I2C_SDA           GPIO_NUM_15
#define BOARD_I2C_SCL           GPIO_NUM_14
#define BOARD_I2C_SPEED_HZ      400000

#define IO_EXPANDER_ADDR        0x20
#define TOUCH_ADDR_CST816       0x15
#define TOUCH_ADDR_FT3168       0x38

#define IO_EXPANDER_REG_OUTPUT  0x01
#define IO_EXPANDER_REG_CONFIG  0x03

#define IO_EXPANDER_LCD_RST     BIT(0)
#define IO_EXPANDER_DSI_PWR_EN  BIT(1)
#define IO_EXPANDER_TOUCH_RST   BIT(2)
#define IO_EXPANDER_SD_CS       BIT(7)
#define IO_EXPANDER_OUTPUT_MASK (IO_EXPANDER_LCD_RST | IO_EXPANDER_DSI_PWR_EN | IO_EXPANDER_TOUCH_RST | IO_EXPANDER_SD_CS)

static board_variant_t s_cached_variant = BOARD_VARIANT_UNKNOWN;
static bool s_detected = false;

static esp_err_t io_expander_write(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t value)
{
    uint8_t data[] = {reg, value};
    return i2c_master_transmit(dev, data, sizeof(data), 100);
}

static esp_err_t release_touch_reset(i2c_master_bus_handle_t bus)
{
    i2c_master_dev_handle_t io_expander = NULL;
    const i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = IO_EXPANDER_ADDR,
        .scl_speed_hz = BOARD_I2C_SPEED_HZ,
    };

    esp_err_t ret = i2c_master_bus_add_device(bus, &dev_cfg, &io_expander);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "IO expander 0x%02x not found: %s", IO_EXPANDER_ADDR, esp_err_to_name(ret));
        return ret;
    }

    uint8_t config = (uint8_t)~IO_EXPANDER_OUTPUT_MASK;
    ret = io_expander_write(io_expander, IO_EXPANDER_REG_CONFIG, config);
    if (ret == ESP_OK) {
        ret = io_expander_write(io_expander, IO_EXPANDER_REG_OUTPUT, IO_EXPANDER_SD_CS);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
    if (ret == ESP_OK) {
        ret = io_expander_write(io_expander, IO_EXPANDER_REG_OUTPUT, IO_EXPANDER_OUTPUT_MASK);
    }
    vTaskDelay(pdMS_TO_TICKS(150));

    esp_err_t rm_ret = i2c_master_bus_rm_device(io_expander);
    if (ret == ESP_OK) {
        ret = rm_ret;
    }
    return ret;
}

board_variant_t board_variant_detect(void)
{
    if (s_detected) {
        return s_cached_variant;
    }

    i2c_master_bus_handle_t bus = NULL;
    const i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = BOARD_I2C_PORT,
        .scl_io_num = BOARD_I2C_SCL,
        .sda_io_num = BOARD_I2C_SDA,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

    esp_err_t ret = i2c_new_master_bus(&bus_cfg, &bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create probe I2C bus: %s", esp_err_to_name(ret));
        s_detected = true;
        return s_cached_variant;
    }

    ret = release_touch_reset(bus);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Touch reset release returned %s, probing anyway", esp_err_to_name(ret));
    }

    if (i2c_master_probe(bus, TOUCH_ADDR_CST816, 100) == ESP_OK) {
        s_cached_variant = BOARD_VARIANT_CO5300_CST816;
    } else if (i2c_master_probe(bus, TOUCH_ADDR_FT3168, 100) == ESP_OK) {
        s_cached_variant = BOARD_VARIANT_SH8601_FT3168;
    } else {
        s_cached_variant = BOARD_VARIANT_UNKNOWN;
    }

    ret = i2c_del_master_bus(bus);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to delete probe I2C bus: %s", esp_err_to_name(ret));
    }

    s_detected = true;
    ESP_LOGI(TAG, "Detected board variant: %s", board_variant_to_name(s_cached_variant));
    return s_cached_variant;
}

const char *board_variant_to_name(board_variant_t variant)
{
    switch (variant) {
    case BOARD_VARIANT_SH8601_FT3168:
        return "original SH8601 + FT3168";
    case BOARD_VARIANT_CO5300_CST816:
        return "modified CO5300 + CST816";
    default:
        return "unknown";
    }
}
