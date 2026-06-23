#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "bsp/esp-bsp.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *TAG = "i2c_tools";
static const int I2C_PROBE_TIMEOUT_MS = 50;

static void scan_i2c_bus(i2c_master_bus_handle_t bus)
{
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for (int base = 0; base < 128; base += 16) {
        printf("%02x: ", base);
        for (int offset = 0; offset < 16; offset++) {
            const uint8_t address = base + offset;
            esp_err_t ret = i2c_master_probe(bus, address, I2C_PROBE_TIMEOUT_MS);
            if (ret == ESP_OK) {
                printf("%02x ", address);
            } else if (ret == ESP_ERR_TIMEOUT) {
                printf("UU ");
            } else {
                printf("-- ");
            }
        }
        printf("\n");
    }
}

void app_main(void)
{
    printf("\nI2C Tools Demo\n");
    printf("Default board I2C pins: SDA=%d SCL=%d\n", BSP_I2C_SDA, BSP_I2C_SCL);
    printf("Configured scan pins: SDA=%d SCL=%d, frequency=%d Hz\n",
           CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL, CONFIG_EXAMPLE_I2C_MASTER_FREQ_HZ);

    if (CONFIG_EXAMPLE_I2C_MASTER_SDA == BSP_I2C_SDA && CONFIG_EXAMPLE_I2C_MASTER_SCL == BSP_I2C_SCL) {
        ESP_ERROR_CHECK(bsp_i2c_init());
        scan_i2c_bus(bsp_i2c_get_handle());
    } else {
        i2c_master_bus_handle_t bus = NULL;
        i2c_master_bus_config_t bus_config = {
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .i2c_port = I2C_NUM_1,
            .scl_io_num = CONFIG_EXAMPLE_I2C_MASTER_SCL,
            .sda_io_num = CONFIG_EXAMPLE_I2C_MASTER_SDA,
            .glitch_ignore_cnt = 7,
            .trans_queue_depth = 0,
            .flags.enable_internal_pullup = true,
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus));
        scan_i2c_bus(bus);
        ESP_ERROR_CHECK(i2c_del_master_bus(bus));
    }

    while (true) {
        ESP_LOGI(TAG, "scan complete; reset or reflash to scan again");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
