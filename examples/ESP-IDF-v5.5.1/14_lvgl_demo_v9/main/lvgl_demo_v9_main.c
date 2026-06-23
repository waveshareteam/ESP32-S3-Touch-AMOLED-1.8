#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_demos.h"
#include "lvgl.h"

static const char *TAG = "lvgl_demo_v9";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting LVGL v9 demo through managed BSP");

    lv_display_t *display = bsp_display_start();
    if (display == NULL) {
        ESP_LOGE(TAG, "display start failed");
        return;
    }
    ESP_ERROR_CHECK(bsp_display_brightness_set(85));

    if (bsp_display_lock(0)) {
        lv_demo_widgets();
        bsp_display_unlock();
    } else {
        ESP_LOGE(TAG, "failed to lock LVGL");
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
