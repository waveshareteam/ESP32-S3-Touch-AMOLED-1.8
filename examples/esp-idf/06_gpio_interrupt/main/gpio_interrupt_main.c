#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"

typedef struct {
    int gpio_num;
    int level;
    TickType_t tick;
} gpio_event_t;

static const char *TAG = "gpio_interrupt";
static QueueHandle_t s_gpio_evt_queue;
static TickType_t s_last_event_tick;

static bool gpio_is_configured(int gpio)
{
    return gpio >= 0 && gpio < GPIO_NUM_MAX;
}

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    const int gpio_num = (int)(intptr_t)arg;
    const TickType_t now_tick = xTaskGetTickCountFromISR();
    const TickType_t debounce_ticks = pdMS_TO_TICKS(CONFIG_EXAMPLE_DEBOUNCE_MS);

    if ((now_tick - s_last_event_tick) < debounce_ticks) {
        return;
    }
    s_last_event_tick = now_tick;

    gpio_event_t event = {
        .gpio_num = gpio_num,
        .level = gpio_get_level(gpio_num),
        .tick = now_tick,
    };
    xQueueSendFromISR(s_gpio_evt_queue, &event, NULL);
}

static void gpio_event_task(void *arg)
{
    gpio_event_t event;
    while (true) {
        if (xQueueReceive(s_gpio_evt_queue, &event, portMAX_DELAY) == pdPASS) {
            ESP_LOGI(TAG, "gpio=%d level=%d time=%" PRIu32 " ms",
                     event.gpio_num, event.level, event.tick * portTICK_PERIOD_MS);
        }
    }
}

void app_main(void)
{
    const gpio_num_t input_gpio = CONFIG_EXAMPLE_GPIO_INPUT;

    printf("\nGPIO Interrupt Demo\n");
    printf("Input GPIO: %d\n", input_gpio);
    printf("Debounce: %d ms\n", CONFIG_EXAMPLE_DEBOUNCE_MS);

    if (!gpio_is_configured(input_gpio)) {
        ESP_LOGW(TAG, "Interrupt GPIO is not configured. Running safely without installing an ISR.");
        while (true) {
            ESP_LOGI(TAG, "dry-run: configure EXAMPLE_GPIO_INPUT to test external GPIO interrupts");
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
    }

    s_gpio_evt_queue = xQueueCreate(10, sizeof(gpio_event_t));
    if (s_gpio_evt_queue == NULL) {
        ESP_LOGE(TAG, "failed to create GPIO event queue");
        return;
    }

    gpio_config_t input_config = {
        .pin_bit_mask = 1ULL << input_gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = CONFIG_EXAMPLE_GPIO_INPUT_PULLUP ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&input_config));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(input_gpio, gpio_isr_handler, (void *)(intptr_t)input_gpio));

    xTaskCreate(gpio_event_task, "gpio_event_task", 3072, NULL, 10, NULL);
}
