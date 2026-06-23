#include <stdbool.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *TAG = "gpio_io";

static bool gpio_is_configured(int gpio)
{
    return gpio >= 0 && gpio < GPIO_NUM_MAX;
}

void app_main(void)
{
    const gpio_num_t output_gpio = CONFIG_EXAMPLE_GPIO_OUTPUT;
    const gpio_num_t input_gpio = CONFIG_EXAMPLE_GPIO_INPUT;

    printf("\nGPIO IO Demo\n");
    printf("Output GPIO: %d\n", output_gpio);
    printf("Input GPIO: %d\n", input_gpio);
    printf("Set both pins in menuconfig and connect output to input for a loopback test.\n");

    if (!gpio_is_configured(output_gpio) || !gpio_is_configured(input_gpio)) {
        ESP_LOGW(TAG, "GPIOs are not configured. Running safely without driving board pins.");
        while (true) {
            ESP_LOGI(TAG, "dry-run: configure EXAMPLE_GPIO_OUTPUT and EXAMPLE_GPIO_INPUT to test external GPIO");
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
    }

    gpio_config_t output_config = {
        .pin_bit_mask = 1ULL << output_gpio,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&output_config));

    gpio_config_t input_config = {
        .pin_bit_mask = 1ULL << input_gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = CONFIG_EXAMPLE_GPIO_INPUT_PULLUP ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&input_config));

    bool level = false;
    while (true) {
        level = !level;
        ESP_ERROR_CHECK(gpio_set_level(output_gpio, level));
        ESP_LOGI(TAG, "output=%d input=%d", level, gpio_get_level(input_gpio));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
