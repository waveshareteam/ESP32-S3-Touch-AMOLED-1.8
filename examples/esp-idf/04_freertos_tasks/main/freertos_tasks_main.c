#include <inttypes.h>
#include <stdio.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

typedef struct {
    uint32_t sequence;
    int64_t uptime_ms;
} demo_message_t;

static const char *TAG = "freertos_tasks";
static QueueHandle_t s_demo_queue;

static void producer_task(void *arg)
{
    uint32_t sequence = 0;
    while (true) {
        demo_message_t msg = {
            .sequence = sequence++,
            .uptime_ms = esp_timer_get_time() / 1000,
        };
        if (xQueueSend(s_demo_queue, &msg, pdMS_TO_TICKS(100)) == pdPASS) {
            ESP_LOGI(TAG, "producer sent message %" PRIu32, msg.sequence);
        } else {
            ESP_LOGW(TAG, "producer queue full");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void consumer_task(void *arg)
{
    demo_message_t msg;
    while (true) {
        if (xQueueReceive(s_demo_queue, &msg, portMAX_DELAY) == pdPASS) {
            ESP_LOGI(TAG, "consumer received message %" PRIu32 " at %" PRId64 " ms",
                     msg.sequence, msg.uptime_ms);
        }
    }
}

void app_main(void)
{
    printf("\nFreeRTOS Tasks Demo\n");
    printf("This demo creates a producer task, a consumer task, and a queue.\n");

    s_demo_queue = xQueueCreate(5, sizeof(demo_message_t));
    if (s_demo_queue == NULL) {
        ESP_LOGE(TAG, "failed to create queue");
        return;
    }

    xTaskCreate(producer_task, "producer", 3072, NULL, 5, NULL);
    xTaskCreate(consumer_task, "consumer", 3072, NULL, 5, NULL);
}
